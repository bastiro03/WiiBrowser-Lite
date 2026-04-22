#include "networkop.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>             // F_GETFL, F_SETFL
#include <ogc/lwp_watchdog.h>  // gettime() / ticks_to_millisecs()
#define host "www.google.com"

// libogc/IOS uses its own non-blocking flag that doesn't match newlib's
// O_NONBLOCK (04000U). See include/mplayer/stream/tcp.c which uses the
// same value — the only known-working pattern on Wii hardware.
#ifndef IOS_O_NONBLOCK
#define IOS_O_NONBLOCK 0x04
#endif

// libogc returns BSD-style errno values from net_connect / net_socket,
// NOT newlib's values. This matters because newlib's <errno.h> uses
// Linux-ish numbers (EINPROGRESS=119, EISCONN=127, etc.) while libogc
// passes through the IOS socket driver's BSD numbers. Compare against
// these constants when interpreting libogc network return codes.
//
// Verified against Dolphin IOS_NET logs: a successful connect progresses
// 36 (EINPROGRESS) → 37 (EALREADY) → 56 (EISCONN). Without using these
// values explicitly, we'd miss the EISCONN and abort a good connection.
#define IOS_EAGAIN        6   /* resource temporarily unavailable       */
#define IOS_EINPROGRESS   36  /* connect handshake in flight            */
#define IOS_EALREADY      37  /* connect already in progress            */
#define IOS_EISCONN       56  /* socket is connected (success terminator) */

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
		// BSD-numbered values that libogc passes through from IOS.
		// Duplicated with the newlib-named cases below because the
		// numeric constants differ (EINPROGRESS: 36 BSD vs 119 newlib).
		case IOS_EINPROGRESS: return "EINPROGRESS (BSD/IOS 36 - connect in flight)";
		case IOS_EALREADY:    return "EALREADY (BSD/IOS 37 - connect in progress)";
		case IOS_EISCONN:     return "EISCONN (BSD/IOS 56 - already connected, SUCCESS)";
		case EINPROGRESS:    return "EINPROGRESS (newlib 119 - async connect pending)";
		case EALREADY:       return "EALREADY (newlib 120 - connect on connecting socket)";
		case ETIMEDOUT:      return "ETIMEDOUT (no reply - ARP/SYN retries exhausted)";
		case ECONNREFUSED:   return "ECONNREFUSED (TCP RST - host reachable, port closed)";
		case EHOSTUNREACH:   return "EHOSTUNREACH (ICMP Dest Unreachable - host code 1)";
		case ENETUNREACH:    return "ENETUNREACH (ICMP Dest Unreachable - net code 0)";
		case EACCES:         return "EACCES (ICMP admin prohibited / firewall)";
		case ENETDOWN:       return "ENETDOWN (network interface down)";
		case ENOTCONN:       return "ENOTCONN (socket not connected)";
		case EADDRNOTAVAIL:  return "EADDRNOTAVAIL (no local IP - DHCP failed?)";
		case EINVAL:         return "EINVAL (bad argument)";
		case EIO:            return "EIO (IOS network I/O error)";
		case EAGAIN:         return "EAGAIN (would block)";
		case EFAULT:         return "EFAULT (bad address)";
		case ENOMEM:         return "ENOMEM (out of memory in IOS)";
		case ENOBUFS:        return "ENOBUFS (no buffer space in IOS)";
		case EPROTONOSUPPORT: return "EPROTONOSUPPORT (IPPROTO not recognized - using IPPROTO_IP fallback)";
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
	fprintf(stderr, "NetworkThread: started\n");
	fflush(stderr);

	s32 res = -1;
	int retry;
	int wait;
	static bool prevInit = false;

	while (!networkThreadHalt)
	{
		retry = 5;

		// Clean up any previous network state ONCE before starting
		// the retry loop. Do NOT deinit/init on every retry - that
		// causes rapid heap churn that corrupts unrelated objects.
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
			// net_init itself failed - wait and let outer loop retry
			g_netdiag.stage        = NET_STAGE_INIT_ASYNC;
			g_netdiag.last_res     = res;
			g_netdiag.retries_used = 0;
			sleep(1);
			continue;  // Jump to outer loop → deinit and try again
		}

		// Network stack initialized successfully. Now retry the
		// connection check WITHOUT tearing down the stack each time.
		while (retry > 0 && !networkThreadHalt)
		{
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
					break;  // Success - exit inner loop
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
			usleep(500000);  // 500ms delay between retries
		}

		if (!networkThreadHalt)
			LWP_SuspendThread(networkthread);
	}
	return nullptr;
}

void InitNetwork()
{
	networkThreadHalt = 0;

	// Only reset networkinit to false when CREATING the thread for the
	// first time. If we're resuming an existing thread (e.g. from the
	// DownloadThread wait loop), leave networkinit at its current value
	// so we don't create a feedback loop where calling InitNetwork()
	// while polling on networkinit prevents it from ever becoming true.
	//
	// The bug: DownloadThread / MenuBrowse both have:
	//   while (!networkinit) { if (suspended) InitNetwork(); usleep(50ms); }
	// If InitNetwork unconditionally sets networkinit=false, the loop
	// resets it to false faster than NetworkThread can set it to true.
	if (networkthread == LWP_THREAD_NULL) {
		networkinit = false;  // Reset state for new init attempt
		LWP_CreateThread(&networkthread, NetworkThread, NULL, networkstack, GUITH_STACK, 30);
	} else {
		LWP_ResumeThread(networkthread);
		// Keep existing networkinit value — thread will update when done
	}
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
	// Pass protocol=0 (not IPPROTO_TCP=6). This matches the Theme-Wii
	// and libreshop-client pattern that works on Dolphin. IPPROTO_TCP
	// triggers Dolphin to return -EPROTONOSUPPORT (-44 internally, which
	// libogc's broken fallback path turns into a spurious setsockopt on
	// fd=-123 visible in IOS_NET logs).
	s32 s = net_socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in sa;

	if (s < 0)
	{
		fprintf(stderr, "CheckConnection: net_socket() failed: %d (%s)\n",
		        s, NetErrStr(s));
		fflush(stderr);
		g_netdiag.stage    = NET_STAGE_SOCKET;
		g_netdiag.last_res = s;
		return false;
	}
	fprintf(stderr, "CheckConnection: net_socket() = %d (success)\n", s);
	fflush(stderr);

	g_netdiag.target_port = NET_PROBE_PORT;
	strncpy(g_netdiag.target_ip, NET_PROBE_IP, sizeof(g_netdiag.target_ip) - 1);
	g_netdiag.target_ip[sizeof(g_netdiag.target_ip) - 1] = '\0';

	// Parse the target IP. Using libogc's inet_addr() (declared in
	// <network.h>) instead of newlib's inet_pton() because inet_pton
	// has silently returned 0 on some builds, leaving sin_addr=0 and
	// causing EINVAL-on-connect. inet_addr returns INADDR_NONE
	// (0xFFFFFFFF) on parse failure.
	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_len    = sizeof(struct sockaddr_in);
	sa.sin_port   = htons(NET_PROBE_PORT);
	u32 addr = inet_addr(NET_PROBE_IP);
	if (addr == 0xFFFFFFFFu)
	{
		fprintf(stderr, "CheckConnection: inet_addr(%s) failed\n", NET_PROBE_IP);
	fflush(stderr);
		g_netdiag.stage    = NET_STAGE_CONNECT;
		g_netdiag.last_res = -EINVAL;
		net_close(s);
		return false;
	}
	sa.sin_addr.s_addr = addr;

	// Put the socket in non-blocking mode using the libogc-idiomatic
	// net_fcntl/IOS_O_NONBLOCK pattern. Previously this code used
	// net_ioctl(FIONBIO), which libogc internally re-writes to the
	// same fcntl call - but Dolphin's IOS emulation doesn't always
	// honour the FIONBIO translation, so doing the fcntl directly
	// matches the known-working pattern in mplayer/stream/tcp.c.
	//
	// Rationale for non-blocking: on Dolphin hosts (especially macOS)
	// a blocking connect to a firewalled port can hang for ~75s while
	// the host TCP stack retries SYN. Non-blocking + polling lets us
	// bound that to our own 3s budget.
	int flags = net_fcntl(s, F_GETFL, 0);
	if (flags >= 0)
		net_fcntl(s, F_SETFL, flags | IOS_O_NONBLOCK);

	u64 connect_start = gettime();
	int res = net_connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));

	fprintf(stderr, "CheckConnection: net_connect(fd=%d, %s:%d) = %d (%s)\n",
	        s, NET_PROBE_IP, NET_PROBE_PORT, res,
	        (res == 0) ? "OK" : NetErrStr(res));
	fflush(stderr);

	bool connected = false;

	// Accept BOTH BSD-numbered (IOS_*) and newlib-numbered (E*) values
	// because _net_convert_error may or may not remap between platforms:
	//   Dolphin IOS_NET log shows 36→37→56 (BSD); libogc passes through.
	//   Real hardware may use libogc's own mapping. Belt + braces.
	#define IS_CONNECTED(r)  ((r) == 0 || (r) == -IOS_EISCONN || (r) == -EISCONN)
	#define IS_PENDING(r)    ((r) == -IOS_EINPROGRESS || (r) == -IOS_EALREADY || (r) == -IOS_EAGAIN || \
	                          (r) == -EINPROGRESS    || (r) == -EALREADY    || (r) == -EAGAIN)

	if (IS_CONNECTED(res))
	{
		connected = true;
	}
	else if (IS_PENDING(res))
	{
		// Poll net_connect() for up to 3s. This is what mplayer does
		// on libogc and is more portable to Dolphin than select().
		fprintf(stderr, "CheckConnection: socket pending (res=%d), polling connect (3s budget)...\n", res);
	fflush(stderr);
		u64 poll_start = gettime();
		while (ticks_delta_ms(poll_start, gettime()) < 3000)
		{
			usleep(20 * 1000); // 20ms between polls
			res = net_connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));
			if (IS_CONNECTED(res))
			{
				connected = true;
				break;
			}
			if (IS_PENDING(res))
				continue;
			// any other return = real error; bail
			break;
		}
		if (!connected && IS_PENDING(res))
			res = -ETIMEDOUT;
		fprintf(stderr, "CheckConnection: poll ended after %ums, final res=%d (%s)\n",
		        ticks_delta_ms(poll_start, gettime()), res,
		        connected ? "CONNECTED" : NetErrStr(res));
		fflush(stderr);
	}
	// else: res is some other negative errno - leave it for diag.

	#undef IS_CONNECTED
	#undef IS_PENDING

	g_netdiag.connect_elapsed_ms = ticks_delta_ms(connect_start, gettime());

	fprintf(stderr, "CheckConnection: closing fd=%d, elapsed=%ums, result=%s\n",
	        s, g_netdiag.connect_elapsed_ms,
	        connected ? "CONNECTED" : "FAILED");
	fflush(stderr);
	net_close(s);

	if (!connected)
	{
		g_netdiag.stage    = NET_STAGE_CONNECT;
		g_netdiag.last_res = res;
		return false;
	}

	return true;
}
