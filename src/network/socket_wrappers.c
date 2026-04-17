// Provide a libc-compatible fcntl() that delegates to libogc's net_fcntl
// for IOS network sockets. curl (and any other code using standard
// socket APIs) calls fcntl(fd, F_SETFL, flags | O_NONBLOCK) to put a
// socket in non-blocking mode for connect(). Without this wrapper, the
// call goes to newlib's fcntl stub which knows nothing about IOS
// sockets and the call fails with ENOTSOCK -- causing curl to bail out
// of TCP connect with "Socket operation on non-socket".
//
// libnetport.a wraps socket/connect/send/recv/etc. but not fcntl, so
// we provide it here. Defining it in our own object file ensures the
// linker prefers our version over newlib's.
//
// Only fcntl operations on IOS socket fds make sense in this codebase
// (no other module calls fcntl() directly). If a non-socket fd is ever
// passed, net_fcntl will return an error which the caller will see.

#include <fcntl.h>
#include <stdarg.h>
#include <network.h>

// IOS's O_NONBLOCK value differs from newlib's. newlib defines
// O_NONBLOCK as 0x4000 (_FNONBLOCK); IOS expects 0x04. Translate
// when passing F_SETFL flags through.
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
		// Translate newlib's O_NONBLOCK bit to IOS_O_NONBLOCK.
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
		// Translate back: report O_NONBLOCK in newlib terms.
		int ret = 0;
		if (ios_flags & IOS_O_NONBLOCK)
			ret |= O_NONBLOCK;
		return ret;
	}

	// Pass through unrecognised commands; net_fcntl will reject what
	// it doesn't understand.
	return net_fcntl(fd, cmd, arg);
}
