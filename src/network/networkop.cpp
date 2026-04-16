#include "networkop.h"
#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>         // FIONBIO for non-blocking socket setup
#include <ogc/lwp_watchdog.h>  // gettime() / ticks_to_millisecs()
#define host "www.google.com"

// Target for the TCP reachability probe. We use 8.8.8.8:443 because:
//   - 8.8.8.8 is Google DNS anycast - globally reachable, no DNS lookup.
//   - Port 80 on 8.8.8.8 is unreliable: many networks (including some
//     home ISPs) silently drop it. When it's firewalled a blocking
//     connect() can hang for ~75s on macOS hosts (Dolphin's emulated
//     SO_CONNECT inherits the host TCP connect timeout).
//   - Port 443 is actively served by Google DNS for DoT/DoH and is
//     not commonly blocked outbound. User-confirmed reachable.
#define NET_PROBE_IP   "8.8.8.8"
#define NET_PROBE_PORT 443

u8 networkstack[GUITH_STACK] ATTRIBUTE_ALIGN(32);
lwp_t networkthread = LWP_THREAD_NULL;

int networkThreadHalt = 0;
bool networkinit = true;

// Last-failure snapshot. Written from NetworkThread (and CheckConnection
// when called from the main thread). Read from the UI when we need to
// tell the user why the connect test failed.
static struct NetDiag g_netdiag = {
	/* stage */              NET_STAGE_NONE,
	/* last_res */           0,
	/* retries_used */       0,
	/* status_wait_ms */     0,
	/* connect_elapsed_ms */ 0,
	/* target_ip */          NET_PROBE_IP,
	/* target_port */        NET_PROBE_PORT,
};

void GetNetDiag(struct NetDiag *out)
{
	if (out)
		*out = g_netdiag;
}

// Many libogc/IOS network errors arrive as negated POSIX errno. TCP
// connect() in particular maps ICMP responses onto errno:
//   ECONNREFUSED  = peer replied with RST (host up, port closed)
//   EHOSTUNREACH  = ICMP Dest Unreachable (host code)
//   ENETUNREACH   = ICMP Dest Unreachable (net code)
//   ETIMEDOUT     = no reply, ARP/TCP SYN retry exhausted
//   EACCES        = ICMP admin prohibited / firewalled
// On Dolphin the emulated IOS_NET sometimes returns error codes that
// libogc translates to plain numeric values not matching our header's
// named constants (seen: -7 on Dolphin for a blocking connect to an
// unreachable host). We fall back to a persistent "errno N" string so
// the UI shows an actionable number instead of "unknown".
const char *NetErrStr(int neg_errno)
{
	int e = (neg_errno < 0) ? -neg_errno : neg_errno;
	switch (e) {
		case 0:              return "OK";
		case EBUSY:          return "EBUSY (init still in progress)";
		case EINPROGRESS:    return "EINPROGRESS (async connect pending - wait for select)";
		case EALREADY:       return "EALREADY (connect on a socket already connecting)";
		case ETIMEDOUT:      return "ETIMEDOUT (no reply - ARP/SYN retries exhausted)";
		case ECONNREFUSED:   return "ECONNREFUSED (TCP RST - host reachable, port closed)";
		case EHOSTUNREACH:   return "EHOSTUNREACH (ICMP Dest Unreachable - host code 1)";
		case ENETUNREACH:    return "ENETUNREACH (ICMP Dest Unreachable - net code 0)";
		case EACCES:         return "EACCES (ICMP admin prohibited / firewall)";
		case ENETDOWN:       return "ENETDOWN (network interface down)";
		case ENOTCONN:       return "ENOTCONN (socket not connected)";
		case EADDRNOTAVAIL:  return "EADDRNOTAVAIL (no local IP - DHCP failed?)";
		case EINVAL:         return "EINVAL (bad argument - init race?)";
		case EIO:            return "EIO (IOS network I/O error)";
		case EAGAIN:         return "EAGAIN (would block)";
		case EFAULT:         return "EFAULT (bad address)";
		case ENOMEM:         return "ENOMEM (out of memory in IOS)";
		case ENOBUFS:        return "ENOBUFS (no buffer space in IOS)";
		default: {
			// Static thread-local-ish scratch for the numeric
			// fallback. This is fine for a UI diagnostic: we
			// only format one error at a time and the result is
			// copied into the modal's text immediately.
			static char buf[48];
			snprintf(buf, sizeof(buf),
			         "errno %d (not in WBL table; see libogc/_net_convert_error)", e);
			return buf;
		}
	}
}

// Helper: ticks delta -> ms. libogc's gettime() returns 40.5MHz ticks
// on the Wii so this is the portable way.
static unsigned ticks_delta_ms(u64 start, u64 end)
{
	return static_cast<unsigned>(ticks_to_millisecs(end - start));
}

/****************************************************************************
 * NetworkThread
 *
 * Thread to handle connection.
 ***************************************************************************/
void *NetworkThread(void *arg)
{
	s32 res = -1;
	int retry;
	int wait;
	static bool prevInit = false;

	while (!networkThreadHalt)
	{
		retry = 5;

		while (retry > 0 && !networkThreadHalt)
		{
			net_deinit();

			if (prevInit)
			{
				prevInit = false;  // only call net_wc24cleanup once
				net_wc24cleanup(); // kill wc24
				usleep(10000);
			}

			res = net_init_async(NULL, NULL);

			if (res != 0)
			{
				g_netdiag.stage        = NET_STAGE_INIT_ASYNC;
				g_netdiag.last_res     = res;
				g_netdiag.retries_used = 5 - retry;
				sleep(1);
				retry--;
				continue;
			}

			u64 status_start = gettime();
			res = net_get_status();
			wait = 500; // only wait 10 sec

			while (res == -EBUSY && wait > 0 && !networkThreadHalt)
			{
				usleep(20 * 1000);
				res = net_get_status();
				wait--;
			}

			g_netdiag.status_wait_ms = ticks_delta_ms(status_start, gettime());

			if (res == 0)
			{
				if (CheckConnection())
				{
					g_netdiag.stage    = NET_STAGE_NONE;
					g_netdiag.last_res = 0;
					networkinit = true;
					prevInit = true;
					break;
				}
				// CheckConnection() populates its own diag stage on failure
			}
			else
			{
				// get_status failed or timed out
				g_netdiag.stage        = NET_STAGE_GET_STATUS;
				g_netdiag.last_res     = res;
				g_netdiag.retries_used = 5 - retry;
			}

			retry--;
			usleep(2000);
		}

		if (!networkThreadHalt)
			LWP_SuspendThread(networkthread);
	}
	return nullptr;
}

void InitNetwork()
{
	networkThreadHalt = 0;
	networkinit = false;

	if (networkthread == LWP_THREAD_NULL)
		LWP_CreateThread(&networkthread, NetworkThread, NULL, networkstack, GUITH_STACK, 30);
	else
		LWP_ResumeThread(networkthread);
}

void StopNetwork()
{
	if (networkthread == LWP_THREAD_NULL)
		return;

	networkThreadHalt = 1;
	LWP_ResumeThread(networkthread);

	LWP_JoinThread(networkthread, NULL);
	networkthread = LWP_THREAD_NULL;
}

bool CheckConnection()
{
	s32 s = net_socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
	struct sockaddr_in sa;

	if (s < 0)
	{
		g_netdiag.stage    = NET_STAGE_SOCKET;
		g_netdiag.last_res = s;
		return false;
	}

	// Force the socket into non-blocking mode BEFORE calling
	// net_connect(). Otherwise Dolphin's emulated SO_CONNECT invokes
	// the host-OS connect() synchronously - on macOS that means a
	// ~75-second TCP connect timeout when the target port is
	// firewalled/dropped, during which our UI believes the network
	// thread has hung. With FIONBIO=1, net_connect() returns
	// immediately with -EINPROGRESS and we drive completion from
	// select() with a bounded wall-clock timeout we control.
	u32 nonblocking = 1;
	int ioctl_ret = net_ioctl(s, FIONBIO, &nonblocking);
	if (ioctl_ret != 0)
	{
		// Not fatal - fall through with blocking semantics and
		// hope libogc/IOS gives us EINPROGRESS anyway. Real Wii
		// hardware reportedly does; Dolphin host-side generally
		// doesn't.
		fprintf(stderr, "CheckConnection: FIONBIO ioctl failed: %d\n", ioctl_ret);
	}

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_len = sizeof(struct sockaddr_in);
	sa.sin_port = htons(NET_PROBE_PORT);
	/* Using inet_pton() because GCC 15 libc doesn't ship inet_aton by default. */
	inet_pton(AF_INET, NET_PROBE_IP, &sa.sin_addr);

	g_netdiag.target_port = NET_PROBE_PORT;
	strncpy(g_netdiag.target_ip, NET_PROBE_IP, sizeof(g_netdiag.target_ip) - 1);
	g_netdiag.target_ip[sizeof(g_netdiag.target_ip) - 1] = '\0';

	u64 connect_start = gettime();
	int res = net_connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));

	fprintf(stderr, "CheckConnection: net_connect(fd=%d, %s:%d) = %d (%s)\n",
	        s, NET_PROBE_IP, NET_PROBE_PORT, res,
	        (res == 0) ? "OK" : NetErrStr(res));

	bool connected = false;

	// With non-blocking mode active, the expected return from a
	// just-started connect is -EINPROGRESS (or on some IOS builds
	// -EALREADY if a previous state leaked through). A return of 0
	// means the handshake already completed (loopback-fast or a
	// blocking IOS that ignored FIONBIO). Anything else negative is a
	// "real" early failure (route missing, address invalid, etc.)
	// and we don't bother select()-waiting on it.
	if (res == 0)
	{
		connected = true;
	}
	else if (res == -EINPROGRESS || res == -EALREADY)
	{
		fprintf(stderr, "CheckConnection: socket pending, select() with 3s timeout...\n");
		// Wait up to 3s for the socket to become writable, then
		// check SO_ERROR to confirm the connect actually succeeded
		// (and wasn't silently rejected in the background).
		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(s, &wset);

		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		int sel = net_select(s + 1, NULL, &wset, NULL, &tv);
		fprintf(stderr, "CheckConnection: net_select() = %d\n", sel);
		if (sel > 0 && FD_ISSET(s, &wset))
		{
			int so_err = 0;
			socklen_t elen = sizeof(so_err);
			int go = net_getsockopt(s, SOL_SOCKET, SO_ERROR,
			                        &so_err, &elen);
			fprintf(stderr, "CheckConnection: SO_ERROR = %d (getsockopt ret=%d)\n",
			        so_err, go);
			if (go == 0 && so_err == 0)
			{
				connected = true;
			}
			else
			{
				// Remember whichever layer returned the real error.
				res = (go != 0) ? go : -so_err;
			}
		}
		else if (sel == 0)
		{
			// select() timed out - handshake never completed
			fprintf(stderr, "CheckConnection: select() timeout after 3s\n");
			res = -ETIMEDOUT;
		}
		else
		{
			// select() itself failed
			fprintf(stderr, "CheckConnection: select() failed: %d\n", sel);
			res = sel;
		}
	}
	// else: res is some other negative errno - leave it for diag.

	g_netdiag.connect_elapsed_ms = ticks_delta_ms(connect_start, gettime());

	fprintf(stderr, "CheckConnection: closing fd=%d, elapsed=%ums, result=%s\n",
	        s, g_netdiag.connect_elapsed_ms,
	        connected ? "CONNECTED" : "FAILED");
	net_close(s);

	if (!connected)
	{
		g_netdiag.stage    = NET_STAGE_CONNECT;
		g_netdiag.last_res = res;
		return false;
	}

	return true;
}
