/* fuzz_css_parse.c - libFuzzer harness for the bison/flex CSS parser.
 *
 * Target: css_parse() + free_rulesets() in src/css/. Already has asan
 * integration tests with real google/x.com CSS; this harness feeds
 * arbitrary bytes to surface edge cases that curated fixtures don't
 * reach (e.g. mid-token EOF, invalid UTF-8, unicode-range syntax,
 * pathological nesting depth).
 *
 * Bugs this should catch:
 *   - Double-free in free_rulesets() error paths
 *   - Unbounded realloc / OOM on deeply nested rules
 *   - Off-by-one in the lexer's lookahead
 *   - yycheck / yytname OOB reads (like the YYERROR_VERBOSE bug that
 *     crashed the Wii)
 */

#include <stddef.h>
#include <stdint.h>
#include "../../src/css/parser.h"

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size > 65536) return 0;   /* cap: parsing >64 KB pathological
	                                 input is just exercising flex's
	                                 buffer growth, not parser logic */

	struct selector_list_t *result = css_parse((const char *)data, (int)size);
	free_rulesets(result);
	return 0;
}
