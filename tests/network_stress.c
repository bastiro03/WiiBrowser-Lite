/* network_stress.c - Invariant tests for InitNetwork().
 *
 * Background: the production InitNetwork() (in src/network/networkop.cpp)
 * was unconditionally setting networkinit=false on every call. Callers
 * in menu.cpp / transfer.cpp have this polling pattern:
 *
 *   while (!networkinit && net_timeout > 0) {
 *     if (LWP_ThreadIsSuspended(networkthread))
 *         InitNetwork();
 *     usleep(50000);
 *   }
 *
 * The bug: there's a window after NetworkThread sets networkinit=true
 * but before the caller observes it. If the caller sees the thread
 * already suspended (i.e. NetworkThread finished and suspended itself)
 * in that window, it calls InitNetwork() - which *resets networkinit
 * back to false* - and we loop again pointlessly, consuming IOS state
 * each cycle. In Dolphin on macOS this cycled net_init/deinit for 2+
 * minutes until the heap corrupted.
 *
 * Full race-condition reproduction is hard to make deterministic
 * (thread scheduling dependent), so these tests instead verify the
 * specific INVARIANT the fix guarantees: calling InitNetwork() on an
 * already-running thread MUST NOT reset networkinit to false.
 *
 * We use function pointers to test both the buggy and fixed versions
 * and verify the buggy version FAILS the invariant while the fixed
 * version PASSES it. That way the test actively demonstrates what the
 * fix changes.
 *
 * Compiled with -fsanitize=address,undefined: catches any heap bugs
 * in the test harness itself, so if the test ever grows new allocation
 * paths, ASAN will surface them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* --- Shared state (same names as production code for clarity) --- */
static bool networkinit;
static int  networkThreadHalt;

/* Simulate the libogc thread handle - we don't actually create threads
 * in these tests; we just track whether a "thread" is conceptually
 * running (created) or not. */
typedef int lwp_t;
#define LWP_THREAD_NULL 0
static lwp_t networkthread;
static int thread_was_created;
static int thread_was_resumed;

/* Mock libogc thread primitives. Real code calls
 * LWP_CreateThread/LWP_ResumeThread - we just count invocations. */
static void mock_LWP_CreateThread(lwp_t *th) {
	*th = 1;
	thread_was_created++;
}
static void mock_LWP_ResumeThread(lwp_t th) {
	(void)th;
	thread_was_resumed++;
}

/* --- The BUGGY InitNetwork (pre-fix from commit <= 7cd6548) --- */
static void InitNetwork_buggy(void) {
	networkThreadHalt = 0;
	networkinit = false;  /* <-- BUG: unconditional reset */
	if (networkthread == LWP_THREAD_NULL)
		mock_LWP_CreateThread(&networkthread);
	else
		mock_LWP_ResumeThread(networkthread);
}

/* --- The FIXED InitNetwork (post-fix from commit fbef4ad) --- */
static void InitNetwork_fixed(void) {
	networkThreadHalt = 0;
	if (networkthread == LWP_THREAD_NULL) {
		networkinit = false;  /* only reset on new thread creation */
		mock_LWP_CreateThread(&networkthread);
	} else {
		mock_LWP_ResumeThread(networkthread);
		/* DO NOT touch networkinit - thread owns that state now */
	}
}

/* --- Test helpers --- */
static void reset_state(void) {
	networkinit = true;   /* real initial value from networkop.cpp:17 */
	networkThreadHalt = 0;
	networkthread = LWP_THREAD_NULL;
	thread_was_created = 0;
	thread_was_resumed = 0;
}

/* --- Tests --- */

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (cond) { \
		passed++; \
		printf("  PASS: %s\n", msg); \
	} else { \
		failed++; \
		printf("  FAIL: %s\n", msg); \
	} \
} while (0)

/* Invariant 1: First call creates the thread and networkinit goes false.
 * Both buggy and fixed should pass this - it's the intended behavior
 * when we genuinely need to (re)start networking. */
static void test_first_call_creates_thread(const char *name,
                                           void (*init_fn)(void)) {
	printf("\n[%s] first call creates thread:\n", name);
	reset_state();
	init_fn();
	ASSERT(thread_was_created == 1, "thread created on first call");
	ASSERT(thread_was_resumed == 0, "thread NOT resumed on first call");
	ASSERT(networkinit == false,    "networkinit reset to false on create");
}

/* Invariant 2: THE BUG. After thread is running and has successfully
 * set networkinit=true, a subsequent InitNetwork() call (e.g. from the
 * polling loop seeing the thread suspended) must NOT reset networkinit
 * back to false. This is the exact bug we fixed.
 *
 * This is where buggy and fixed DIVERGE:
 *   - buggy: clobbers networkinit to false (creates feedback loop)
 *   - fixed: preserves networkinit=true (caller can exit wait loop) */
static void test_resume_preserves_success(const char *name,
                                          void (*init_fn)(void)) {
	printf("\n[%s] resume preserves networkinit=true:\n", name);
	reset_state();

	/* Simulate the sequence: first InitNetwork creates thread,
	 * NetworkThread eventually runs and succeeds (networkinit=true),
	 * then polling loop sees thread suspended and calls InitNetwork
	 * again (thinking it needs restart). */
	init_fn();                    /* first call: creates thread */
	networkinit = true;           /* NetworkThread set this after success */
	init_fn();                    /* second call: should RESUME only */

	ASSERT(thread_was_created == 1, "thread created exactly once");
	ASSERT(thread_was_resumed == 1, "thread resumed on second call");
	ASSERT(networkinit == true,
	       "networkinit PRESERVED after resume (this is the bug fix)");
}

/* Invariant 3: Many resumes after a success must not clobber networkinit.
 * Models the DownloadThread poll loop that may call InitNetwork many
 * times before observing the winning networkinit=true. */
static void test_many_resumes_preserve_success(const char *name,
                                               void (*init_fn)(void)) {
	printf("\n[%s] 100 resumes preserve networkinit=true:\n", name);
	reset_state();

	init_fn();             /* create */
	networkinit = true;    /* thread succeeded */
	for (int i = 0; i < 100; i++)
		init_fn();         /* each one should be a no-op for networkinit */

	ASSERT(thread_was_created == 1,        "thread created exactly once");
	ASSERT(thread_was_resumed == 100,      "resumed 100 times");
	ASSERT(networkinit == true,
	       "networkinit still true after 100 resumes");
}

int main(void) {
	printf("========================================================\n");
	printf("Testing BUGGY InitNetwork - these tests SHOULD FAIL\n");
	printf("to demonstrate that the pre-fix code has the bug.\n");
	printf("========================================================\n");
	int expected_buggy_failures_before = failed;

	test_first_call_creates_thread("BUGGY", InitNetwork_buggy);
	test_resume_preserves_success("BUGGY", InitNetwork_buggy);
	test_many_resumes_preserve_success("BUGGY", InitNetwork_buggy);

	int buggy_failures = failed - expected_buggy_failures_before;
	printf("\nBuggy version failed %d assertions (expected: >0)\n",
	       buggy_failures);

	printf("\n========================================================\n");
	printf("Testing FIXED InitNetwork - these tests SHOULD PASS\n");
	printf("========================================================\n");
	int fixed_failures_before = failed;

	test_first_call_creates_thread("FIXED", InitNetwork_fixed);
	test_resume_preserves_success("FIXED", InitNetwork_fixed);
	test_many_resumes_preserve_success("FIXED", InitNetwork_fixed);

	int fixed_failures = failed - fixed_failures_before;
	printf("\nFixed version failed %d assertions (expected: 0)\n",
	       fixed_failures);

	printf("\n========================================================\n");
	printf("Summary: %d passed, %d failed (%d in buggy, %d in fixed)\n",
	       passed, failed, buggy_failures, fixed_failures);

	/* Test passes iff:
	 *   - buggy version has at least one invariant failure (bug reproduced)
	 *   - fixed version has zero failures (bug fixed)
	 *
	 * If buggy_failures == 0: our test doesn't actually catch the bug.
	 * If fixed_failures > 0:  our fix is incomplete or wrong. */
	if (buggy_failures == 0) {
		printf("RESULT FAIL: buggy version passed all tests - "
		       "test doesn't actually catch the bug!\n");
		return 1;
	}
	if (fixed_failures > 0) {
		printf("RESULT FAIL: fixed version has failures - "
		       "regression detected!\n");
		return 1;
	}
	printf("RESULT PASS: bug reproduced by buggy version, "
	       "absent in fixed version\n");
	return 0;
}
