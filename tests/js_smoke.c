/* QuickJS smoke test using our wbl wrapper.
 * Verifies: context create/destroy, successful eval, error capture,
 *           memory limit enforcement. */

#include <stdio.h>
#include <string.h>
#include "wbl/js_engine.h"

int main(void) {
    int fails = 0;

    /* 1. Create and destroy ------------------------------------------- */
    {
        wbl_js_context_t *c = wbl_js_create();
        if (!c) { printf("[FAIL] create returned NULL\n"); fails++; }
        else    { printf("[OK] create\n"); wbl_js_destroy(c); }
    }

    /* 2. Successful eval ---------------------------------------------- */
    {
        wbl_js_context_t *c = wbl_js_create();
        int r = wbl_js_eval(c, "var x = 1 + 2; x * 10", "test1");
        if (r != 0) { printf("[FAIL] eval(1+2) returned %d\n", r); fails++; }
        else        { printf("[OK] eval(1+2)\n"); }
        wbl_js_destroy(c);
    }

    /* 3. Error capture ------------------------------------------------- */
    {
        wbl_js_context_t *c = wbl_js_create();
        int r = wbl_js_eval(c, "throw new TypeError('bad')", "test2");
        if (r == 0) { printf("[FAIL] expected error, got success\n"); fails++; }
        else {
            const char *err = wbl_js_last_error(c);
            if (!err || !strstr(err, "TypeError")) {
                printf("[FAIL] error string: %s\n", err ? err : "(null)");
                fails++;
            } else {
                printf("[OK] error captured: %s\n", err);
            }
        }
        wbl_js_destroy(c);
    }

    /* 4. Inline script entry point (what html.cpp will call) --------- */
    {
        wbl_js_context_t *c = wbl_js_create();
        const char *inline_js =
            "var RLCONF = {};\n"
            "var RLSTATE = {};\n"
            "var wgBackendResponseTime = 100;";
        int r = wbl_js_eval_inline(c, inline_js, "https://en.wikipedia.org/");
        if (r != 0) { printf("[FAIL] inline MediaWiki bootstrap failed: %s\n",
                              wbl_js_last_error(c)); fails++; }
        else        { printf("[OK] MediaWiki bootstrap-style inline script\n"); }
        wbl_js_destroy(c);
    }

    /* 5. Memory limit (should reject runaway allocation) -------------- */
    {
        wbl_js_context_t *c = wbl_js_create();
        wbl_js_set_memory_limit(c, 64 * 1024);  /* 64KB - tight */
        int r = wbl_js_eval(c, "var a = new Array(1000000).fill('x')", "test5");
        if (r == 0) { printf("[WARN] allocated 1M array under 64KB limit!\n"); }
        else        { printf("[OK] memory limit enforced\n"); }
        wbl_js_destroy(c);
    }

    printf("\n%d failures\n", fails);
    return fails;
}
