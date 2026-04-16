#ifndef __NETWORKOP_H__
#define __NETWORKOP_H__

#include <errno.h>
#include "transfer.h"

// Which network init stage most recently failed. Used so the UI error
// modal can tell the user *where* networking died, not just that it did.
enum NetStage {
	NET_STAGE_NONE         = 0,
	NET_STAGE_INIT_ASYNC   = 1,  // net_init_async() returned non-zero
	NET_STAGE_GET_STATUS   = 2,  // net_get_status() returned error or -EBUSY timeout
	NET_STAGE_CONNECT      = 3,  // net_connect() to 8.8.8.8:80 failed
	NET_STAGE_SOCKET       = 4,  // net_socket() returned <0
};

// Snapshot of the most recent network-init failure. Populated by
// NetworkThread / CheckConnection before they give up on a retry.
struct NetDiag {
	int      stage;           // NetStage enum value
	int      last_res;        // raw return code (libogc returns negated errno)
	int      retries_used;    // retry counter consumed before bail-out
	unsigned status_wait_ms;  // how long we spent polling net_get_status
	unsigned connect_elapsed_ms; // wall-clock spent inside net_connect
	char     target_ip[16];   // which IP the connect test was hitting
	unsigned short target_port;
};

void InitNetwork();
void StopNetwork();
bool CheckConnection();

// Copies last-error snapshot out for UI display. Safe to call from
// the main thread while NetworkThread is alive (fields are written
// atomically-enough for display purposes; this is a human-readable
// diagnostic, not a correctness-critical value).
void GetNetDiag(struct NetDiag *out);

// Convert a negative libogc/POSIX errno to a human string. Covers the
// TCP-connect failure modes that map to ICMP Destination Unreachable,
// plus the common async-init errors from libogc/ios.
// Returns a pointer to a static string - do not free.
const char *NetErrStr(int neg_errno);

extern lwp_t networkthread;
extern u8 networkstack[GUITH_STACK];

extern bool networkinit;

#endif
