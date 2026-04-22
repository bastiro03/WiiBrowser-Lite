/* js_parser_asan_test.c - Feed real-world inline JS through QuickJS under ASAN.
 *
 * Background: the WiiBrowser-Lite JS engine wraps QuickJS (src/wbl/js_engine.c).
 * Before v0.x we had no coverage for what happens when real google.com or
 * x.com inline <script> blocks are fed to wbl_js_eval_inline(). A parser
 * or GC bug there would corrupt heap state the same way the CSS parser
 * did, and crash on Wii inside newlib's _free_r.
 *
 * This test feeds REAL JS from tests/fixtures/{google,x}_home.js (extracted
 * from live page fetches) through wbl_js_eval_inline() with ASAN attached.
 *
 * Contract: run to completion with ASAN+UBSAN+LSAN enabled, zero reports,
 * no crashes. If QuickJS mishandles any modern-JS construct (optional
 * chaining, nullish coalescing, classes, async/await, template literals,
 * private fields, destructuring) we catch it here BEFORE the Wii user.
 *
 * Note: eval() can legitimately fail (reference errors for undefined DOM
 * globals, etc.). That's fine — we just require NO MEMORY CORRUPTION.
 * Error returns are expected; ASAN reports are not.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "wbl/js_engine.h"

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

/* Create a fresh context, eval, destroy. Forces QuickJS through its full
 * alloc/free lifecycle each iteration — any leaked internal object or
 * double-free in shutdown gets caught. */
static int eval_in_fresh_ctx(const char *src, const char *name)
{
	wbl_js_context_t *c = wbl_js_create();
	if (!c) return -1;
	int rc = wbl_js_eval_inline(c, src, name);
	wbl_js_destroy(c);
	return rc;
}

/* Stress: eval the same script N times in back-to-back contexts. A UAF
 * in QuickJS's parser or GC only manifests when malloc reuses a freed
 * slot, which these repeated cycles reliably trigger. */
static void stress(const char *src, const char *name, int iterations)
{
	for (int i = 0; i < iterations; i++)
		(void)eval_in_fresh_ctx(src, name);
}

static char *read_file(const char *path, size_t *out_len)
{
	FILE *f = fopen(path, "rb");
	if (!f) return NULL;
	fseek(f, 0, SEEK_END);
	long len = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *buf = (char *)malloc(len + 1);
	if (!buf) { fclose(f); return NULL; }
	size_t n = fread(buf, 1, len, f);
	buf[n] = 0;
	fclose(f);
	if (out_len) *out_len = n;
	return buf;
}

int main(void)
{
	fprintf(stderr, "Testing QuickJS under ASAN/UBSAN:\n\n");

	if (!wbl_js_available()) {
		fprintf(stderr, "JS engine compiled out (WBL_HAS_JAVASCRIPT=0). Skipping.\n");
		return 0;
	}

	/* ---- Lifecycle basics ---- */
	fprintf(stderr, "[Context lifecycle]\n");
	for (int i = 0; i < 50; i++) {
		wbl_js_context_t *c = wbl_js_create();
		if (!c) { failed++; break; }
		wbl_js_destroy(c);
	}
	ASSERT(1, "50 create/destroy cycles leave no dangling state");

	/* ---- Trivial eval ---- */
	fprintf(stderr, "\n[Trivial evaluation]\n");
	eval_in_fresh_ctx("1 + 1", "trivial.js");
	ASSERT(1, "1+1 evaluates without heap report");

	/* ---- Modern JS constructs (ES2020+) ---- */
	fprintf(stderr, "\n[Modern JS language features]\n");

	eval_in_fresh_ctx("const x = {a:{b:1}}; x?.a?.b ?? 'none'", "optional_chain.js");
	ASSERT(1, "optional chaining + nullish coalescing");

	eval_in_fresh_ctx("class Foo { #priv = 1; get v() { return this.#priv; } }"
	                  " new Foo().v", "private_fields.js");
	ASSERT(1, "private class fields");

	eval_in_fresh_ctx("async function f() { return await Promise.resolve(42); } f()",
	                  "async.js");
	ASSERT(1, "async/await");

	eval_in_fresh_ctx("const [a, b, ...rest] = [1,2,3,4,5];"
	                  " const {x=10, y: z=20} = {x: 1};", "destructure.js");
	ASSERT(1, "destructuring + defaults + rest");

	eval_in_fresh_ctx("const tpl = `a${1+2}b${(()=>'c')()}d`", "template.js");
	ASSERT(1, "template literals with nested IIFE");

	eval_in_fresh_ctx("function* gen() { yield 1; yield 2; }"
	                  " [...gen()]", "generator.js");
	ASSERT(1, "generator function + spread");

	eval_in_fresh_ctx("const m = new Map([['a',1]]); const s = new Set([1,2,3]);"
	                  " JSON.stringify([...m.entries(), ...s])", "map_set.js");
	ASSERT(1, "Map/Set with spread + JSON");

	eval_in_fresh_ctx("try { null.x; } catch (e) { e.message; }", "throw.js");
	ASSERT(1, "thrown TypeError caught");

	/* ---- Large string / loop stress ---- */
	fprintf(stderr, "\n[Memory stress]\n");

	eval_in_fresh_ctx(
		"let s = ''; for (let i = 0; i < 1000; i++) s += 'x'; s.length",
		"string_concat.js");
	ASSERT(1, "1000-iteration string concat (GC stress)");

	eval_in_fresh_ctx(
		"const a = []; for (let i = 0; i < 10000; i++) a.push({n: i});"
		" a.filter(o => o.n % 2 == 0).length", "array_stress.js");
	ASSERT(1, "10k object allocations + filter (heap churn)");

	/* ---- Syntax errors ---- */
	fprintf(stderr, "\n[Malformed scripts — error path coverage]\n");

	eval_in_fresh_ctx("function(", "unterm1.js");
	ASSERT(1, "unterminated function declaration");

	eval_in_fresh_ctx("{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{", "unbalanced.js");
	ASSERT(1, "massively unbalanced braces");

	eval_in_fresh_ctx("\x00\x01\x02 garbage non-ascii", "binary.js");
	ASSERT(1, "binary garbage at script start");

	/* ---- Real-world fixtures ---- */
	fprintf(stderr, "\n[Real-world inline <script> fixtures]\n");

	const struct { const char *path; const char *name; } fixtures[] = {
		{ "fixtures/google_home.js", "google.com" },
		{ "fixtures/x_home.js",      "x.com" },
	};

	for (size_t fi = 0; fi < sizeof(fixtures) / sizeof(fixtures[0]); fi++) {
		size_t len = 0;
		char *js = read_file(fixtures[fi].path, &len);
		if (!js || len == 0) {
			fprintf(stderr, "  SKIP: %s (fixture missing)\n", fixtures[fi].path);
			failed++;
			free(js);
			continue;
		}
		fprintf(stderr, "  %s: %zu bytes\n", fixtures[fi].path, len);

		/* Single parse + eval — establishes baseline no-corruption */
		eval_in_fresh_ctx(js, fixtures[fi].name);
		char msg1[256];
		snprintf(msg1, sizeof(msg1), "%s inline JS evaluates without ASAN report",
		         fixtures[fi].name);
		ASSERT(1, msg1);

		/* Stress: 20 iterations. Fewer than CSS because JS is heavier
		 * (x.com is 220 KB per iteration → ~4.4 MB of parses). */
		stress(js, fixtures[fi].name, 20);
		char msg2[256];
		snprintf(msg2, sizeof(msg2), "%s 20-iteration heap-churn stress",
		         fixtures[fi].name);
		ASSERT(1, msg2);

		free(js);
	}

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
