/* css_parser_asan_test.c - Feed real-world CSS through the parser under ASAN.
 *
 * Background: Wii users hit a hard crash inside newlib's _free_r after
 * visiting google.com. CSS lexer spammed "Illegal character (40)" /
 * "(41)" / "(62)" ... before the crash. Crash dump showed the classic
 * dlmalloc freelist-unlink pattern (stw r6, 12(r7); stw r7, 8(r6))
 * with DAR=0x5A — heap metadata corruption.
 *
 * The CSS parser (bison+flex, src/css/css_{lex,syntax}.c, src/css/parser.c)
 * was written for CSS1/CSS2-era syntax and chokes on modern calc(),
 * linear-gradient(), attribute selectors, custom properties, etc.
 * When the lexer emits "Illegal character" and the parser enters error
 * recovery, it can corrupt refcounts on property_t.count leading to
 * double-free in free_rulesets().
 *
 * This test feeds REAL CSS from google.com (via tests/fixtures/) and a
 * curated battery of modern-CSS traps through css_parse() +
 * free_rulesets() with ASAN attached. If either the parser leaks, double-
 * frees, uses after free, or reads out of bounds, ASAN reports it here —
 * BEFORE the Wii user hits it.
 *
 * Contract: run to completion with ASAN enabled, zero reports, zero
 * crashes, across 100 parse/free cycles (to stress the freelist).
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "../src/css/parser.h"

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

/* Parse CSS and free the result. Returns true if we got back non-NULL.
 * ASAN is expected to crash us if parser misbehaves — which is the point. */
static bool parse_and_free(const char *css, size_t len)
{
	struct selector_list_t *result = css_parse(css, (int)len);
	free_rulesets(result);
	return result != NULL;
}

/* Parse + free N times to stress freelist. On Wii we suspect double-free
 * or use-after-free that only shows up when allocations reuse addresses. */
static void stress(const char *css, size_t len, int iterations)
{
	for (int i = 0; i < iterations; i++)
	{
		struct selector_list_t *result = css_parse(css, (int)len);
		free_rulesets(result);
	}
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
	fprintf(stderr, "Testing CSS parser under ASAN:\n\n");

	/* ---- Modern CSS features the CSS2-era lexer doesn't understand ---- */
	fprintf(stderr, "[Modern CSS that triggers error recovery]\n");

	/* calc() - parens are "Illegal character" to the lexer */
	parse_and_free("div { width: calc(100% - 20px); }", 33);
	ASSERT(1, "calc() without crash/ASAN report");

	/* linear-gradient - same issue with parens + commas */
	parse_and_free(
		"div { background: linear-gradient(top, #f00, #0f0); }", 53);
	ASSERT(1, "linear-gradient() without crash");

	/* CSS attribute selector [attr=value] - brackets+equals are illegal */
	parse_and_free("input[type=\"text\"] { color: blue; }", 35);
	ASSERT(1, "attribute selector without crash");

	/* Child combinator > - illegal character */
	parse_and_free("div > p { font-size: 14px; }", 28);
	ASSERT(1, "child combinator without crash");

	/* Multiple combinators and modern selectors */
	parse_and_free("ul+li, div>.btn, a[href] { color: red; }", 40);
	ASSERT(1, "combinators + attribute selectors without crash");

	/* CSS custom property */
	parse_and_free(":root { --primary: #f00; } a { color: var(--primary); }", 56);
	ASSERT(1, "CSS variables without crash");

	/* @media query */
	parse_and_free("@media (max-width: 600px) { div { font: 12px; } }", 49);
	ASSERT(1, "@media query without crash");

	/* Webkit vendor prefix values */
	parse_and_free("a { -webkit-appearance: none; }", 31);
	ASSERT(1, "-webkit- vendor prefix without crash");

	/* ---- Pathological inputs ---- */
	fprintf(stderr, "\n[Pathological inputs]\n");

	parse_and_free("", 0);
	ASSERT(1, "empty CSS");

	parse_and_free("{", 1);
	ASSERT(1, "just an open brace");

	parse_and_free("}", 1);
	ASSERT(1, "just a close brace");

	parse_and_free("div {", 5);
	ASSERT(1, "unterminated ruleset");

	parse_and_free("div { color", 11);
	ASSERT(1, "unterminated declaration");

	parse_and_free("div { color: }", 14);
	ASSERT(1, "declaration missing value");

	parse_and_free(")))(((;;;{}", 11);
	ASSERT(1, "garbage punctuation");

	/* 1 KB of parens (trigger "Illegal character" spam like Wii) */
	char garbage[1024];
	memset(garbage, '(', sizeof(garbage) - 1);
	garbage[sizeof(garbage) - 1] = 0;
	parse_and_free(garbage, sizeof(garbage) - 1);
	ASSERT(1, "1 KB of '(' chars (stress illegal-char path)");

	/* ---- Comma-separated selector chains (shared property refcount) ---- */
	fprintf(stderr, "\n[Shared property refcount stress]\n");

	/* 10 selectors sharing one property list — refcount starts at 10,
	 * free_rulesets decrements 10 times, if any increment/decrement is
	 * off-by-one we get a double-free. */
	parse_and_free(
		"a,b,c,d,e,f,g,h,i,j { color: red; }", 35);
	ASSERT(1, "10-way comma selector share (refcount stress)");

	/* ---- Real-world fixtures: google.com, x.com, wikipedia-alike ---- */
	fprintf(stderr, "\n[Real-world CSS fixtures]\n");

	const char *fixtures[] = {
		"fixtures/google_home.css",               /* google.com styles */
		"fixtures/x_home.css",                    /* x.com desktop styles */
		"fixtures/wikipedia_representative.css",  /* MediaWiki patterns (wikipedia blocked in dev env) */
	};

	for (size_t fi = 0; fi < sizeof(fixtures) / sizeof(fixtures[0]); fi++)
	{
		size_t len = 0;
		char *css = read_file(fixtures[fi], &len);
		if (!css || len == 0)
		{
			fprintf(stderr, "  SKIP: %s (fixture missing)\n", fixtures[fi]);
			failed++;
			free(css);
			continue;
		}
		fprintf(stderr, "  %s: %zu bytes\n", fixtures[fi], len);

		parse_and_free(css, len);
		char msg1[256];
		snprintf(msg1, sizeof(msg1), "%s parses without ASAN report", fixtures[fi]);
		ASSERT(1, msg1);

		/* Stress: parse+free 100 times. A subtle UAF that only fires
		 * when malloc reuses the corrupted address shows up here. */
		stress(css, len, 100);
		char msg2[256];
		snprintf(msg2, sizeof(msg2), "%s 100-iteration freelist stress", fixtures[fi]);
		ASSERT(1, msg2);

		free(css);
	}

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
