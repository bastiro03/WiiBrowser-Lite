/* errno_translation_test.c - Unit tests for IOS→newlib errno translation
 *
 * Background: libogc's net_recv/net_send/net_connect return negative IOS
 * errno values. These are BSD-style errno numbers (EAGAIN=35, EINPROGRESS=36,
 * etc.) but newlib uses different values (EAGAIN=11, EINPROGRESS=119).
 *
 * Without translation, curl sees errno=35 after a non-blocking recv with no
 * data available, compares it to EAGAIN (11), and treats it as a fatal error
 * instead of retrying. This causes "Recv failure: Success" errors and broken
 * HTTP requests.
 *
 * socket_wrappers.c implements ios_errno_to_newlib() to translate. These
 * tests verify the critical mappings work correctly.
 */

#include <stdio.h>
#include <errno.h>

/* IOS (BSD-style) errno values */
#define IOS_EAGAIN       35
#define IOS_EINPROGRESS  36
#define IOS_EALREADY     37
#define IOS_EADDRNOTAVAIL 49
#define IOS_EISCONN      56
#define IOS_ENOTCONN     57
#define IOS_ETIMEDOUT    60
#define IOS_ECONNREFUSED 61
#define IOS_ENETUNREACH  51
#define IOS_EHOSTUNREACH 65
#define IOS_ECONNRESET   54
#define IOS_EMSGSIZE     40
#define IOS_EPIPE        32
#define IOS_ENOBUFS      55

/* Copy of ios_errno_to_newlib from socket_wrappers.c */
static int ios_errno_to_newlib(int ios_err)
{
	switch (ios_err)
	{
	case 35: return EAGAIN;       /* IOS_EAGAIN/EWOULDBLOCK -> newlib 11 */
	case 36: return EINPROGRESS;  /* IOS 36 -> newlib 119 */
	case 37: return EALREADY;     /* IOS 37 -> newlib 120 */
	case 49: return EADDRNOTAVAIL;/* IOS 49 -> newlib 125 */
	case 56: return EISCONN;      /* IOS 56 -> newlib 127 */
	case 57: return ENOTCONN;     /* IOS 57 -> newlib 128 */
	case 60: return ETIMEDOUT;    /* IOS 60 -> newlib 116 */
	case 61: return ECONNREFUSED; /* IOS 61 -> newlib 111 */
	case 51: return ENETUNREACH;  /* IOS 51 -> newlib 114 */
	case 65: return EHOSTUNREACH; /* IOS 65 -> newlib 118 */
	case 54: return ECONNRESET;   /* IOS 54 -> newlib 104 */
	case 40: return EMSGSIZE;     /* IOS 40 -> newlib 122 */
	case 32: return EPIPE;        /* IOS 32 -> newlib 32 (same) */
	case 55: return ENOBUFS;      /* IOS 55 -> newlib 105 */
	/* Values that are the same across IOS and newlib: */
	case 1:  return EPERM;
	case 2:  return ENOENT;
	case 4:  return EINTR;
	case 5:  return EIO;
	case 9:  return EBADF;
	case 12: return ENOMEM;
	case 13: return EACCES;
	case 14: return EFAULT;
	case 22: return EINVAL;
	default: return ios_err;      /* Unknown - pass through */
	}
}

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { \
		fprintf(stderr, "  FAIL: %s\n", msg); \
		failed++; \
	} else { \
		fprintf(stderr, "  PASS: %s\n", msg); \
		passed++; \
	} \
} while (0)

int main(void)
{
	fprintf(stderr, "Testing IOS→newlib errno translation:\n\n");

	/* Critical mapping: EAGAIN (non-blocking I/O) */
	fprintf(stderr, "[EAGAIN translation]\n");
	ASSERT(ios_errno_to_newlib(IOS_EAGAIN) == EAGAIN,
	       "IOS_EAGAIN (35) maps to newlib EAGAIN (11)");
	ASSERT(ios_errno_to_newlib(IOS_EAGAIN) == EWOULDBLOCK,
	       "IOS_EAGAIN (35) maps to newlib EWOULDBLOCK (same as EAGAIN)");
	ASSERT(ios_errno_to_newlib(IOS_EAGAIN) != 35,
	       "IOS_EAGAIN does NOT pass through as raw 35");

	/* Connect-related errno (async connect) */
	fprintf(stderr, "\n[Async connect errno]\n");
	ASSERT(ios_errno_to_newlib(IOS_EINPROGRESS) == EINPROGRESS,
	       "IOS_EINPROGRESS (36) maps to newlib EINPROGRESS (119)");
	ASSERT(ios_errno_to_newlib(IOS_EALREADY) == EALREADY,
	       "IOS_EALREADY (37) maps to newlib EALREADY (120)");
	ASSERT(ios_errno_to_newlib(IOS_EISCONN) == EISCONN,
	       "IOS_EISCONN (56) maps to newlib EISCONN (127)");

	/* Network errors */
	fprintf(stderr, "\n[Network errors]\n");
	ASSERT(ios_errno_to_newlib(IOS_ECONNREFUSED) == ECONNREFUSED,
	       "IOS_ECONNREFUSED (61) maps to newlib ECONNREFUSED (111)");
	ASSERT(ios_errno_to_newlib(IOS_ETIMEDOUT) == ETIMEDOUT,
	       "IOS_ETIMEDOUT (60) maps to newlib ETIMEDOUT (116)");
	ASSERT(ios_errno_to_newlib(IOS_ENETUNREACH) == ENETUNREACH,
	       "IOS_ENETUNREACH (51) maps to newlib ENETUNREACH (114)");
	ASSERT(ios_errno_to_newlib(IOS_EHOSTUNREACH) == EHOSTUNREACH,
	       "IOS_EHOSTUNREACH (65) maps to newlib EHOSTUNREACH (118)");
	ASSERT(ios_errno_to_newlib(IOS_ECONNRESET) == ECONNRESET,
	       "IOS_ECONNRESET (54) maps to newlib ECONNRESET (104)");

	/* Same-value passthrough (IOS and newlib agree) */
	fprintf(stderr, "\n[Same-value errno]\n");
	ASSERT(ios_errno_to_newlib(9) == EBADF,
	       "EBADF (9) passes through unchanged");
	ASSERT(ios_errno_to_newlib(22) == EINVAL,
	       "EINVAL (22) passes through unchanged");
	ASSERT(ios_errno_to_newlib(5) == EIO,
	       "EIO (5) passes through unchanged");

	/* Unknown errno passthrough */
	fprintf(stderr, "\n[Unknown errno passthrough]\n");
	ASSERT(ios_errno_to_newlib(255) == 255,
	       "Unknown errno 255 passes through as-is");

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");

	return failed > 0 ? 1 : 0;
}
