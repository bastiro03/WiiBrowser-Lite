#include "networkop.h"
#include <string.h>
#include <stdio.h>
#include <ogc/lwp_watchdog.h>  // gettime() / ticks_to_millisecs()
#define host "www.google.com"

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
	/* target_ip */          "8.8.8.8",
	/* target_port */        80,
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
		default:             return "unknown (see errno on host)";
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

	memset(&sa, 0, sizeof(struct sockaddr_in));
	sa.sin_family = AF_INET;
	sa.sin_len = sizeof(struct sockaddr_in);
	sa.sin_port = htons(80);
	/* 8.8.8.8 = Google DNS anycast, always reachable on any global route.
	 * Using inet_pton() because GCC 15 libc doesn't ship inet_aton by default. */
	inet_pton(AF_INET, "8.8.8.8", &sa.sin_addr);

	g_netdiag.target_port = 80;
	strncpy(g_netdiag.target_ip, "8.8.8.8", sizeof(g_netdiag.target_ip) - 1);
	g_netdiag.target_ip[sizeof(g_netdiag.target_ip) - 1] = '\0';

	// Dolphin's IOS_NET emulator treats SO_CONNECT as asynchronous: the
	// first call returns immediately (often with -EINPROGRESS), and the
	// handshake completes in the background. Subsequent connect() calls
	// on the same socket return -EALREADY ("Operation already in
	// progress") until the first one finishes.
	//
	// On real Wii hardware the behavior is similar - libogc's net_connect
	// can return early if the IOS socket is in non-blocking mode.
	//
	// Correct pattern: issue connect(), then use select() to wait for
	// writability (= handshake complete) with a bounded timeout. Treat
	// EINPROGRESS/EALREADY on the initial connect as "pending, go wait"
	// rather than "failed".
	u64 connect_start = gettime();
	int res = net_connect(s, (struct sockaddr *)&sa, sizeof(struct sockaddr_in));

	bool connected = false;
	if (res == 0)
	{
		// Fast path: some IOS builds actually block until complete.
		connected = true;
	}
	else if (res == -EINPROGRESS || res == -EALREADY)
	{
		// Handshake running in background - wait up to 3s for the
		// socket to become writable, then check SO_ERROR to confirm
		// the connect actually succeeded (and wasn't silently rejected).
		fd_set wset;
		FD_ZERO(&wset);
		FD_SET(s, &wset);

		struct timeval tv;
		tv.tv_sec = 3;
		tv.tv_usec = 0;

		int sel = net_select(s + 1, NULL, &wset, NULL, &tv);
		if (sel > 0 && FD_ISSET(s, &wset))
		{
			int so_err = 0;
			socklen_t elen = sizeof(so_err);
			int go = net_getsockopt(s, SOL_SOCKET, SO_ERROR,
			                        &so_err, &elen);
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
			res = -ETIMEDOUT;
		}
		else
		{
			// select() itself failed
			res = sel;
		}
	}
	// else: res is some other negative errno, leave it for diag

	g_netdiag.connect_elapsed_ms = ticks_delta_ms(connect_start, gettime());

	net_close(s);

	if (!connected)
	{
		g_netdiag.stage    = NET_STAGE_CONNECT;
		g_netdiag.last_res = res;
		return false;
	}

	return true;
}
