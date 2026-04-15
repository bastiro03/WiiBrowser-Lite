/****************************************************************************
 * WiiBrowser-Lite JavaScript Engine Wrapper
 *
 * Thin abstraction over QuickJS-ng (minimal "MicroQuickJS" build).
 * On platforms without WBL_HAS_JAVASCRIPT, calls become no-ops.
 *
 * Memory footprint: ~200KB code + WBL_MEM_BUDGET_JS_HEAP_KB heap
 ***************************************************************************/

#ifndef WBL_JS_ENGINE_H
#define WBL_JS_ENGINE_H

#include "wbl/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Opaque handle - on no-JS platforms this is just a stub pointer */
typedef struct wbl_js_context wbl_js_context_t;

/* Lifecycle */
wbl_js_context_t *wbl_js_create(void);
void wbl_js_destroy(wbl_js_context_t *ctx);

/* Execute a script. Returns 0 on success, -1 on error.
 * On no-JS platforms, always returns 0 without doing anything. */
int wbl_js_eval(wbl_js_context_t *ctx,
                const char *script,
                const char *filename);

/* Execute inline <script>...</script> content from an HTML page */
int wbl_js_eval_inline(wbl_js_context_t *ctx,
                       const char *script,
                       const char *url);

/* Get last error string (NULL if no error) */
const char *wbl_js_last_error(wbl_js_context_t *ctx);

/* Memory limit (call before eval). Honored on platforms with JS support. */
void wbl_js_set_memory_limit(wbl_js_context_t *ctx, size_t bytes);

/* Capability check at runtime */
static inline int wbl_js_available(void) {
#if WBL_HAS_JAVASCRIPT
    return 1;
#else
    return 0;
#endif
}

#ifdef __cplusplus
}
#endif

#endif /* WBL_JS_ENGINE_H */
