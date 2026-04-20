/* fuzz_js_eval.c - libFuzzer harness for the QuickJS wrapper.
 *
 * Target: wbl_js_eval_inline() in src/wbl/js_engine.c. Already exercised
 * with real google/x.com inline JS; this harness feeds arbitrary bytes
 * to find parser corner cases (unclosed regex, template literal
 * edge cases, unusual whitespace, invalid UTF-8).
 *
 * Bugs this should catch:
 *   - QuickJS parser OOB on malformed tokens
 *   - GC inconsistency on eval of very-deep / wide expressions
 *   - Leaks from JSValue references that escape eval context
 *   - Stack exhaustion from extreme nesting (harness caps to 16 KB)
 */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "wbl/js_engine.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size == 0 || size > 16384) return 0;
	if (!wbl_js_available()) return 0;

	/* wbl_js_eval_inline expects NUL-terminated source. */
	char *src = (char *)malloc(size + 1);
	if (!src) return 0;
	memcpy(src, data, size);
	src[size] = 0;

	wbl_js_context_t *c = wbl_js_create();
	if (c)
	{
		(void)wbl_js_eval_inline(c, src, "fuzz.js");
		wbl_js_destroy(c);
	}

	free(src);
	return 0;
}
