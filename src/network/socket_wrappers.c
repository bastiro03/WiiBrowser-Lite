// Libc-compatible wrappers around libogc's net_* for IOS network sockets.
//
// Provides:
//   - fcntl()  -> net_fcntl with O_NONBLOCK translation
//   - select() -> SO_ERROR + MSG_PEEK loop (bypasses net_poll)
//   - poll()   -> same, iterating pollfd array
//
// Why we override libnetport.a's select()/poll():
//   libnetport's select() delegates to net_select() which is internally
//   implemented via net_poll()/SO_POLL. Dolphin's IOS emulation does
//   not handle all the IOS-specific POLL event flags that libogc emits
//   (e.g. 0xc0000, 0x88a00), so SO_POLL becomes a no-op and curl spins
//   in a 15-second timeout loop before giving up. Providing our own
//   select()/poll() that check socket state via SO_ERROR (for writable/
//   connect-complete) and MSG_PEEK (for readable) sidesteps the broken
//   poll path entirely.
//
// Defining BOTH select() and poll() here prevents libnetport's
// select.o (which bundles both) from being pulled in, avoiding the
// "multiple definition" linker error.

#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <sys/select.h>
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

// Check if a socket is writable: connect has completed (successfully or
// with an error). Uses SO_ERROR to probe pending connect status.
// Returns: 1 if writable, -1 if error (socket unusable), 0 if still
// pending. On error, *err_out receives the pending error code.
static int socket_writable(int fd, int *err_out)
{
	int err = 0;
	socklen_t len = sizeof(err);
	int rc = net_getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
	if (rc < 0)
	{
		// SO_ERROR query failed. On Dolphin this can happen if the
		// option isn't fully supported. Treat as "not ready yet"
		// rather than fatal error, so select() keeps polling.
		return 0;
	}
	if (err_out)
		*err_out = err;
	if (err == 0)
		return 1;
	// EINPROGRESS/EAGAIN/EALREADY mean the connect is still pending.
	// Anything else is a terminal error we should surface via except.
	if (err == 36 /* EINPROGRESS */ ||
	    err == 35 /* EWOULDBLOCK/EAGAIN on IOS */ ||
	    err == 37 /* EALREADY */)
		return 0;
	return -1;
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
			return ready;

		if (timeout_ms == 0)
			return 0;

		if (timeout_ms > 0)
		{
			u32 elapsed = (u32)ticks_to_millisecs(gettime() - start);
			if (elapsed >= (u32)timeout_ms)
				return 0;
		}

		usleep(5000); // 5ms poll interval
	}
}

int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms)
    __attribute__((externally_visible, used));

int poll(struct pollfd *fds, nfds_t nfds, int timeout_ms)
{
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
			return ready;

		if (timeout_ms == 0)
			return 0;

		if (timeout_ms > 0)
		{
			u32 elapsed = (u32)ticks_to_millisecs(gettime() - start);
			if (elapsed >= (u32)timeout_ms)
				return 0;
		}

		usleep(5000);
	}
}
