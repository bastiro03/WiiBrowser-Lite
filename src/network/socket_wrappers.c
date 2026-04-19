// Libc-compatible wrappers around libogc's net_* for IOS network sockets.
//
// Provides:
//   - fcntl()         -> O_NONBLOCK translation to IOS flag bits
//   - __wrap_connect()-> synchronous connect (polls until EISCONN)
//   - select()        -> SO_ERROR + MSG_PEEK loop (bypasses net_poll)
//   - poll()          -> same, iterating pollfd array
//
// Strategy: Make connect() synchronous in our wrapper, matching the
// pattern used by CheckConnection. Dolphin's IOS emulation requires
// explicit polling (repeated connect() calls) to drive the connection
// state machine from EINPROGRESS -> EALREADY -> EISCONN. SO_ERROR does
// NOT auto-update in Dolphin, so a select()+SO_ERROR loop alone never
// sees the connect complete.
//
// The __wrap_connect symbol is activated via -Wl,--wrap=connect in the
// link: calls to connect() are redirected here, while the original
// libnetport connect() is still available as __real_connect().
//
// Why we still override libnetport.a's select()/poll():
//   curl uses select()/poll() for read/write readiness after the
//   connect completes. libnetport's select() delegates to net_poll
//   which triggers SO_POLL with uninitialized event flags that Dolphin
//   doesn't recognize (0xc0000, 0x88a00). Our wrappers use MSG_PEEK to
//   check readability directly.
//
// Defining BOTH select() and poll() here prevents libnetport's
// select.o (which bundles both) from being pulled in, avoiding the
// "multiple definition" linker error.

#include <errno.h>
#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <unistd.h>
#include <network.h>
#include <ogc/lwp_watchdog.h>

// IOS's O_NONBLOCK value differs from newlib's. newlib defines
// O_NONBLOCK as 0x4000 (_FNONBLOCK); IOS expects 0x04.
#ifndef IOS_O_NONBLOCK
#define IOS_O_NONBLOCK 0x04
#endif

int fcntl(int fd, int cmd, ...) __attribute__((externally_visible, used));

int fcntl(int fd, int cmd, ...)
{
	va_list ap;
	int arg = 0;

	va_start(ap, cmd);
	arg = va_arg(ap, int);
	va_end(ap);

	if (cmd == F_SETFL)
	{
		unsigned int ios_flags = 0;
		if (arg & O_NONBLOCK)
			ios_flags |= IOS_O_NONBLOCK;
		return net_fcntl(fd, F_SETFL, ios_flags);
	}

	if (cmd == F_GETFL)
	{
		int ios_flags = net_fcntl(fd, F_GETFL, 0);
		if (ios_flags < 0)
			return ios_flags;
		int ret = 0;
		if (ios_flags & IOS_O_NONBLOCK)
			ret |= O_NONBLOCK;
		return ret;
	}

	return net_fcntl(fd, cmd, arg);
}

// IOS errno values (BSD numbering). Dolphin's IOS emulation returns
// these from net_connect as negative values.
#define IOS_EINPROGRESS 36
#define IOS_EALREADY    37
#define IOS_EAGAIN_VAL  35
#define IOS_EISCONN     56

// Translate IOS (BSD-style) errno values to newlib errno values.
// libogc's net_recv/net_send/net_connect return negative IOS errno on
// error. curl (and other POSIX code) checks errno against newlib values,
// e.g. EAGAIN=11 not 35. Without this translation, curl treats an
// EAGAIN-on-nonblocking-recv (IOS 35) as a fatal error because it
// doesn't match newlib EAGAIN (11).
static int ios_errno_to_newlib(int ios_err)
{
	switch (ios_err)
	{
	case 35: return EAGAIN;       // IOS_EAGAIN/EWOULDBLOCK -> newlib 11
	case 36: return EINPROGRESS;  // IOS 36 -> newlib 119
	case 37: return EALREADY;     // IOS 37 -> newlib 120
	case 49: return EADDRNOTAVAIL;// IOS 49 -> newlib 125
	case 56: return EISCONN;      // IOS 56 -> newlib 127
	case 57: return ENOTCONN;     // IOS 57 -> newlib 128
	case 60: return ETIMEDOUT;    // IOS 60 -> newlib 116
	case 61: return ECONNREFUSED; // IOS 61 -> newlib 111
	case 51: return ENETUNREACH;  // IOS 51 -> newlib 114
	case 65: return EHOSTUNREACH; // IOS 65 -> newlib 118
	case 54: return ECONNRESET;   // IOS 54 -> newlib 104
	case 40: return EMSGSIZE;     // IOS 40 -> newlib 122
	case 32: return EPIPE;        // IOS 32 -> newlib 32 (same)
	case 55: return ENOBUFS;      // IOS 55 -> newlib 105
	// Values that are the same across IOS and newlib:
	case 1:  return EPERM;
	case 2:  return ENOENT;
	case 4:  return EINTR;
	case 5:  return EIO;
	case 9:  return EBADF;
	case 12: return ENOMEM;
	case 13: return EACCES;
	case 14: return EFAULT;
	case 22: return EINVAL;
	default: return ios_err;      // Unknown - pass through
	}
}

// Track which fds have successfully connected. Dolphin's SO_GETSOCKOPT
// for SO_ERROR returns -22 (EINVAL), so we can't query connect status
// that way. Instead, track successful connects in __wrap_connect and
// check this set in socket_writable(). Wii apps typically have <10 open
// sockets, so a simple fixed array is sufficient.
#define MAX_TRACKED_FDS 16
static int connected_fds[MAX_TRACKED_FDS];
static int connected_fd_count = 0;

static void mark_fd_connected(int fd)
{
	// Check if already tracked
	for (int i = 0; i < connected_fd_count; i++)
		if (connected_fds[i] == fd)
			return;
	// Add if room
	if (connected_fd_count < MAX_TRACKED_FDS)
		connected_fds[connected_fd_count++] = fd;
}

static int is_fd_connected(int fd)
{
	for (int i = 0; i < connected_fd_count; i++)
		if (connected_fds[i] == fd)
			return 1;
	return 0;
}

static void unmark_fd_connected(int fd)
{
	for (int i = 0; i < connected_fd_count; i++)
	{
		if (connected_fds[i] == fd)
		{
			// Shift remaining fds down
			for (int j = i; j < connected_fd_count - 1; j++)
				connected_fds[j] = connected_fds[j + 1];
			connected_fd_count--;
			return;
		}
	}
}

// Wrap close() to untrack fds when they're closed. This prevents stale
// tracking entries and handles fd reuse correctly.
int close(int fd) __attribute__((externally_visible, used));

int close(int fd)
{
	unmark_fd_connected(fd);
	return net_close(fd);
}

// Synchronous connect: call net_connect repeatedly until the connection
// actually completes (EISCONN) or fails. This matches CheckConnection's
// pattern, which is required on Dolphin because SO_ERROR does not
// auto-update during async connect. Only repeated SO_CONNECT calls drive
// the state machine EINPROGRESS -> EALREADY -> EISCONN.
int __real_connect(int s, struct sockaddr *addr, socklen_t addrlen);

int __wrap_connect(int s, struct sockaddr *addr, socklen_t addrlen)
    __attribute__((externally_visible, used));

int __wrap_connect(int s, struct sockaddr *addr, socklen_t addrlen)
{
	extern int *__errno(void);
	int res = __real_connect(s, addr, addrlen);

	// Already connected or succeeded synchronously. Clear errno so curl
	// doesn't see stale error values alongside return=0.
	if (res == 0)
	{
		fprintf(stderr, "__wrap_connect(fd=%d): immediate success\n", s);
		fflush(stderr);
		*__errno() = 0;
		mark_fd_connected(s);
		return 0;
	}

	// net_connect returns negative errno on failure. Dolphin's IOS
	// emulation gives us IOS_EINPROGRESS on the first call; subsequent
	// calls yield IOS_EALREADY until the connect finishes with
	// IOS_EISCONN.
	int err = -res;
	if (err != IOS_EINPROGRESS && err != IOS_EALREADY &&
	    err != IOS_EAGAIN_VAL && err != EINPROGRESS && err != EALREADY &&
	    err != EAGAIN)
	{
		// Real error, not a pending-connect signal.
		fprintf(stderr, "__wrap_connect(fd=%d): error %d\n", s, err);
		fflush(stderr);
		return res;
	}

	// Poll up to 30 seconds for the connect to complete. curl's
	// CURLOPT_CONNECTTIMEOUT gates the overall retry budget; we just
	// need to keep advancing the state machine while curl waits.
	fprintf(stderr, "__wrap_connect(fd=%d): polling for completion...\n", s);
	fflush(stderr);
	u64 start = gettime();
	while (ticks_to_millisecs(gettime() - start) < 30000)
	{
		usleep(20 * 1000); // 20ms between polls (matches CheckConnection)
		res = __real_connect(s, addr, addrlen);
		if (res == 0)
		{
			fprintf(stderr, "__wrap_connect(fd=%d): success after %ums\n",
			        s, (unsigned)ticks_to_millisecs(gettime() - start));
			fflush(stderr);
			*__errno() = 0;
			mark_fd_connected(s);
			return 0;
		}
		err = -res;
		if (err == IOS_EISCONN || err == EISCONN)
		{
			fprintf(stderr, "__wrap_connect(fd=%d): EISCONN after %ums\n",
			        s, (unsigned)ticks_to_millisecs(gettime() - start));
			fflush(stderr);
			*__errno() = 0;
			mark_fd_connected(s);
			return 0;
		}
		if (err == IOS_EINPROGRESS || err == IOS_EALREADY ||
		    err == IOS_EAGAIN_VAL || err == EINPROGRESS || err == EALREADY ||
		    err == EAGAIN)
			continue;
		// Real error; surface it.
		fprintf(stderr, "__wrap_connect(fd=%d): poll error %d\n", s, err);
		fflush(stderr);
		return res;
	}
	// Timed out. Return -ETIMEDOUT so curl sees a connect failure.
	fprintf(stderr, "__wrap_connect(fd=%d): poll timeout\n", s);
	fflush(stderr);
	return -ETIMEDOUT;
}

// Wrap recv/send to translate IOS errno values to newlib errno values.
// libnetport's recv/send either don't set errno correctly or set it to
// raw IOS values. curl checks errno == EAGAIN (newlib 11) but IOS
// returns 35; without translation, curl treats non-blocking EAGAIN as
// a fatal error ("Recv failure: Success" in the log).
//
// These call net_recv/net_send directly to bypass libnetport's errno
// handling and do the translation ourselves.
ssize_t __wrap_recv(int s, void *mem, size_t len, int flags)
    __attribute__((externally_visible, used));
ssize_t __wrap_send(int s, const void *mem, size_t len, int flags)
    __attribute__((externally_visible, used));

ssize_t __wrap_recv(int s, void *mem, size_t len, int flags)
{
	int ret = net_recv(s, mem, len, flags);
	if (ret < 0)
	{
		extern int *__errno(void);
		*__errno() = ios_errno_to_newlib(-ret);
		return -1;
	}
	return ret;
}

ssize_t __wrap_send(int s, const void *mem, size_t len, int flags)
{
	int ret = net_send(s, mem, len, flags);
	if (ret < 0)
	{
		extern int *__errno(void);
		*__errno() = ios_errno_to_newlib(-ret);
		return -1;
	}
	return ret;
}

// Check if a socket is writable: connect has completed (successfully or
// with an error). On Dolphin, SO_GETSOCKOPT(SO_ERROR) returns -22 EINVAL,
// so we can't query connect status that way. Instead, check if the fd was
// marked connected by __wrap_connect. Since __wrap_connect is synchronous
// and only returns after the connection completes, any fd it marked is
// definitively writable.
// Returns: 1 if writable, -1 if error (socket unusable), 0 if still
// pending. On error, *err_out receives the pending error code (unused now).
static int socket_writable(int fd, int *err_out)
{
	// If __wrap_connect successfully connected this fd, it's writable.
	if (is_fd_connected(fd))
	{
		fprintf(stderr, "socket_writable(fd=%d): connected (tracked)\n", fd);
		fflush(stderr);
		if (err_out)
			*err_out = 0;
		return 1;
	}

	// Not in our connected set. Either connect hasn't been called yet,
	// or it's still in progress (shouldn't happen with synchronous
	// __wrap_connect, but possible if code calls select() before connect).
	fprintf(stderr, "socket_writable(fd=%d): not connected (not tracked)\n", fd);
	fflush(stderr);
	return 0;
}

// Check if a socket has data (or EOF/error) ready to read.
// Returns: 1 if readable, 0 if not yet readable.
static int socket_readable(int fd)
{
	char buf;
	int rc = net_recv(fd, &buf, 1, MSG_PEEK);
	if (rc >= 0)
		return 1;
	// Non-blocking with nothing available -> EAGAIN/EWOULDBLOCK.
	// Any other errno means the socket is broken; surface as readable
	// so the caller's recv() returns the real error.
	extern int *__errno(void);
	int e = *__errno();
	if (e == 11 /* EAGAIN */ || e == 35 /* EWOULDBLOCK */)
		return 0;
	return 1;
}

static long timeval_to_ms(const struct timeval *tv)
{
	if (!tv)
		return -1;
	return (long)tv->tv_sec * 1000L + (long)tv->tv_usec / 1000L;
}

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout) __attribute__((externally_visible, used));

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
           struct timeval *timeout)
{
	long timeout_ms = timeval_to_ms(timeout);
	u64 start = gettime();

	// Debug: log select() calls to trace curl's post-connect behavior
	int r_count = 0, w_count = 0;
	if (readfds)
		for (int i = 0; i < nfds && i < FD_SETSIZE; i++)
			if (FD_ISSET(i, readfds)) r_count++;
	if (writefds)
		for (int i = 0; i < nfds && i < FD_SETSIZE; i++)
			if (FD_ISSET(i, writefds)) w_count++;
	fprintf(stderr, "select(nfds=%d, r=%d, w=%d, timeout=%ldms)\n",
	        nfds, r_count, w_count, timeout_ms);
	fflush(stderr);

	fd_set r_in, w_in;
	FD_ZERO(&r_in);
	FD_ZERO(&w_in);
	if (readfds)
		r_in = *readfds;
	if (writefds)
		w_in = *writefds;

	if (readfds)
		FD_ZERO(readfds);
	if (writefds)
		FD_ZERO(writefds);
	if (exceptfds)
		FD_ZERO(exceptfds);

	for (;;)
	{
		int ready = 0;

		for (int fd = 0; fd < nfds && fd < FD_SETSIZE; fd++)
		{
			if (FD_ISSET(fd, &w_in))
			{
				int err = 0;
				int s = socket_writable(fd, &err);
				if (s == 1)
				{
					FD_SET(fd, writefds);
					ready++;
				}
				else if (s < 0)
				{
					if (exceptfds)
						FD_SET(fd, exceptfds);
					ready++;
				}
			}
			if (FD_ISSET(fd, &r_in))
			{
				if (socket_readable(fd))
				{
					FD_SET(fd, readfds);
					ready++;
				}
			}
		}

		if (ready > 0)
		{
			fprintf(stderr, "select: returning %d ready\n", ready);
			fflush(stderr);
			return ready;
		}

		if (timeout_ms == 0)
		{
			fprintf(stderr, "select: timeout=0, returning 0\n");
			fflush(stderr);
			return 0;
		}

		if (timeout_ms > 0)
		{
			u32 elapsed = (u32)ticks_to_millisecs(gettime() - start);
			if (elapsed >= (u32)timeout_ms)
			{
				fprintf(stderr, "select: timeout after %ums\n", elapsed);
				fflush(stderr);
				return 0;
			}
		}

		usleep(5000); // 5ms poll interval
	}
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms)
    __attribute__((externally_visible, used));

int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms)
{
	fprintf(stderr, "poll(nfds=%u, timeout=%dms)\n", (unsigned)nfds, timeout_ms);
	fflush(stderr);
	u64 start = gettime();

	for (;;)
	{
		int ready = 0;

		for (nfds_t i = 0; i < nfds; i++)
		{
			fds[i].revents = 0;
			if (fds[i].fd < 0)
				continue;

			if (fds[i].events & (POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI))
			{
				if (socket_readable(fds[i].fd))
					fds[i].revents |= (fds[i].events &
					                   (POLLIN | POLLRDNORM));
			}
			if (fds[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND))
			{
				int err = 0;
				int s = socket_writable(fds[i].fd, &err);
				if (s == 1)
					fds[i].revents |= (fds[i].events &
					                   (POLLOUT | POLLWRNORM));
				else if (s < 0)
					fds[i].revents |= POLLERR;
			}
			if (fds[i].revents)
				ready++;
		}

		if (ready > 0)
		{
			fprintf(stderr, "poll: returning %d ready\n", ready);
			fflush(stderr);
			return ready;
		}

		if (timeout_ms == 0)
		{
			fprintf(stderr, "poll: timeout=0, returning 0\n");
			fflush(stderr);
			return 0;
		}

		if (timeout_ms > 0)
		{
			u32 elapsed = (u32)ticks_to_millisecs(gettime() - start);
			if (elapsed >= (u32)timeout_ms)
			{
				fprintf(stderr, "poll: timeout after %ums\n", elapsed);
				fflush(stderr);
				return 0;
			}
		}

		usleep(5000);
	}
}
