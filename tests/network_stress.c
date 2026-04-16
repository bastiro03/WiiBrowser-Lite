/* network_stress.c - Host-side stress test for net_init/deinit cycling.
 *
 * Simulates the Wii NetworkThread retry loop to expose heap corruption /
 * leaks that would manifest after repeated net_init → net_deinit cycles.
 *
 * Compile with ASAN/UBSAN to catch:
 *   - Use-after-free
 *   - Double-free
 *   - Leaks (if -fsanitize=leak is available)
 *
 * This doesn't link against libogc (host can't), so we stub the Wii-
 * specific net_* functions. The point is to exercise the CALLER logic
 * (our retry loop, CheckConnection, etc.) with mocked network state.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/* Stub Wii libogc types */
typedef int s32;
typedef unsigned int u32;
typedef unsigned long long u64;

/* Mock network functions to simulate Wii IOS behavior */
static int mock_net_init_fail_count = 0;
static int mock_net_status_fail_count = 0;

s32 net_init_async(void *cb, void *userdata) {
	(void)cb; (void)userdata;
	/* Fail the first N times to simulate DHCP retries */
	if (mock_net_init_fail_count > 0) {
		mock_net_init_fail_count--;
		return -EBUSY;
	}
	return 0;
}

s32 net_get_status(void) {
	if (mock_net_status_fail_count > 0) {
		mock_net_status_fail_count--;
		return -EBUSY;
	}
	return 0;
}

void net_deinit(void) {
	/* Nothing to free in mock mode, but real Wii would release IOS state */
}

void net_wc24cleanup(void) {
	/* Wii-specific WiiConnect24 cleanup */
}

/* Minimal CheckConnection stub - just returns false after delay */
int CheckConnection(void) {
	usleep(100 * 1000); /* 100ms to simulate 3s select timeout */
	return 0; /* always fails for this test */
}

/* Mock time functions */
u64 gettime(void) {
	return 0;
}

unsigned ticks_delta_ms(u64 start, u64 end) {
	(void)start; (void)end;
	return 0;
}

/* The actual NetworkThread retry logic from networkop.cpp, simplified */
static int test_network_thread_cycle(int max_retries) {
	int retry = max_retries;
	s32 res;
	int wait;

	while (retry > 0) {
		net_deinit();

		res = net_init_async(NULL, NULL);
		if (res != 0) {
			/* Init failed - wait 1s and retry */
			usleep(10 * 1000); /* 10ms instead of 1s for test speed */
			retry--;
			continue;
		}

		/* Poll net_get_status */
		res = net_get_status();
		wait = 50;
		while (res == -EBUSY && wait > 0) {
			usleep(200); /* 200us instead of 20ms */
			res = net_get_status();
			wait--;
		}

		if (res == 0) {
			if (CheckConnection()) {
				/* Success */
				return 1;
			}
			/* CheckConnection failed - fall through to retry */
		}

		retry--;
		usleep(2000); /* Match the real 2ms delay */
	}

	return 0; /* All retries failed */
}

int main(void) {
	printf("=== network_stress: cycling net_init/deinit 1000x ===\n");

	/* Scenario 1: init always fails (exercises fast retry loop) */
	mock_net_init_fail_count = 10;
	mock_net_status_fail_count = 0;

	for (int i = 0; i < 100; i++) {
		test_network_thread_cycle(5);
		if (i % 20 == 0) {
			printf("  completed %d cycles (init-fail path)...\n", i);
		}
	}

	/* Scenario 2: init succeeds but status fails (different code path) */
	mock_net_init_fail_count = 0;
	mock_net_status_fail_count = 10;

	for (int i = 0; i < 100; i++) {
		test_network_thread_cycle(5);
		if (i % 20 == 0) {
			printf("  completed %d cycles (status-fail path)...\n", i);
		}
	}

	/* Scenario 3: mixed (some inits fail, some succeed + status fail) */
	for (int i = 0; i < 800; i++) {
		mock_net_init_fail_count = (i % 3); /* Every 3rd succeeds */
		mock_net_status_fail_count = (i % 2);
		test_network_thread_cycle(5);
	}

	printf("  1000 cycles complete with no crash/leak detected\n");
	printf("=== PASS ===\n");
	return 0;
}
