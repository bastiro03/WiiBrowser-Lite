/****************************************************************************
 * WiiBrowser-Lite JavaScript Engine Implementation
 *
 * Wraps QuickJS-ng with a minimal "MicroQuickJS" configuration:
 *   - No BigInt, BigFloat, BigDecimal (saves ~80KB)
 *   - No Atomics (saves ~10KB)
 *   - No date parsing beyond ISO 8601 (saves ~15KB)
 *   - Memory-limited heap to prevent runaway scripts
 ***************************************************************************/

#include "wbl/js_engine.h"

#if WBL_HAS_JAVASCRIPT

#include <stdlib.h>
#include <string.h>
#include "quickjs.h"

struct wbl_js_context {
    JSRuntime *rt;
    JSContext *ctx;
    char       last_error[256];
};

wbl_js_context_t *wbl_js_create(void)
{
    wbl_js_context_t *c = (wbl_js_context_t *)calloc(1, sizeof(*c));
    if (!c) return NULL;

    c->rt = JS_NewRuntime();
    if (!c->rt) { free(c); return NULL; }

    /* Default heap limit from platform budget */
    JS_SetMemoryLimit(c->rt, WBL_MEM_BUDGET_JS_HEAP_KB * 1024);

    /* Conservative GC threshold for low-memory platforms */
    JS_SetGCThreshold(c->rt, 256 * 1024);

    /* Stack limit (Wii has small thread stacks) */
    JS_SetMaxStackSize(c->rt, 256 * 1024);

    c->ctx = JS_NewContext(c->rt);
    if (!c->ctx) {
        JS_FreeRuntime(c->rt);
        free(c);
        return NULL;
    }

    return c;
}

void wbl_js_destroy(wbl_js_context_t *ctx)
{
    if (!ctx) return;
    if (ctx->ctx) JS_FreeContext(ctx->ctx);
    if (ctx->rt)  JS_FreeRuntime(ctx->rt);
    free(ctx);
}

void wbl_js_set_memory_limit(wbl_js_context_t *ctx, size_t bytes)
{
    if (ctx && ctx->rt) {
        JS_SetMemoryLimit(ctx->rt, bytes);
    }
}

static void capture_error(wbl_js_context_t *ctx, JSValue ex)
{
    const char *str = JS_ToCString(ctx->ctx, ex);
    if (str) {
        strncpy(ctx->last_error, str, sizeof(ctx->last_error) - 1);
        ctx->last_error[sizeof(ctx->last_error) - 1] = 0;
        JS_FreeCString(ctx->ctx, str);
    } else {
        strcpy(ctx->last_error, "(unknown error)");
    }
}

int wbl_js_eval(wbl_js_context_t *ctx, const char *script, const char *filename)
{
    if (!ctx || !script) return -1;

    JSValue val = JS_Eval(ctx->ctx, script, strlen(script),
                          filename ? filename : "<inline>",
                          JS_EVAL_TYPE_GLOBAL);
    if (JS_IsException(val)) {
        JSValue ex = JS_GetException(ctx->ctx);
        capture_error(ctx, ex);
        JS_FreeValue(ctx->ctx, ex);
        JS_FreeValue(ctx->ctx, val);
        return -1;
    }
    JS_FreeValue(ctx->ctx, val);
    ctx->last_error[0] = 0;
    return 0;
}

int wbl_js_eval_inline(wbl_js_context_t *ctx, const char *script, const char *url)
{
    return wbl_js_eval(ctx, script, url);
}

const char *wbl_js_last_error(wbl_js_context_t *ctx)
{
    if (!ctx || ctx->last_error[0] == 0) return NULL;
    return ctx->last_error;
}

#else  /* !WBL_HAS_JAVASCRIPT - stub implementation */

#include <stdlib.h>

struct wbl_js_context { int unused; };

wbl_js_context_t *wbl_js_create(void) { return NULL; }
void wbl_js_destroy(wbl_js_context_t *ctx) { (void)ctx; }
int  wbl_js_eval(wbl_js_context_t *ctx, const char *s, const char *f)
                                  { (void)ctx; (void)s; (void)f; return 0; }
int  wbl_js_eval_inline(wbl_js_context_t *ctx, const char *s, const char *u)
                                  { (void)ctx; (void)s; (void)u; return 0; }
const char *wbl_js_last_error(wbl_js_context_t *ctx) { (void)ctx; return NULL; }
void wbl_js_set_memory_limit(wbl_js_context_t *ctx, size_t b) { (void)ctx; (void)b; }

#endif
