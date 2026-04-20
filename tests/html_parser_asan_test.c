/* html_parser_asan_test.c - Feed real-world HTML through htmlcxx under ASAN.
 *
 * Background: After fixing CSS and JS parser heap corruption, Wii user hits
 * another crash loading x.com. Dolphin log shows screen filled with infinitely
 * repeating "<meta content=\"width=device-width,maximum-scale=1.0,initial-scale=1.0,mini"
 * tag followed by DSI exception in _free_r at DAR=0x0C (12-byte offset through
 * a corrupted/null pointer).
 *
 * This pattern suggests buffer overflow or infinite loop in htmlcxx's ParserDom
 * or entity decoder. The meta tag should appear once in the source HTML but
 * something in SAX parsing or tree building is duplicating it infinitely.
 *
 * This test feeds REAL HTML from google.com and x.com through
 * htmlcxx::HTML::ParserDom::parseTree() under ASAN. If htmlcxx has a buffer
 * overflow, UAF, or infinite allocation loop, ASAN catches it here.
 *
 * Contract: run to completion with ASAN enabled, zero reports, zero crashes.
 * If the parser can't handle modern HTML5 (data-* attrs, void elements like
 * <meta> without closing tags, UTF-8 emoji, etc.) we catch it BEFORE the Wii.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

/* C wrapper for C++ htmlcxx::HTML::ParserDom::parseTree().
 * Returns number of nodes in the resulting tree, or -1 on failure. */
extern int html_parse_and_count_nodes(const char *html, size_t len);

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
	fprintf(stderr, "Testing htmlcxx HTML parser under ASAN:\n\n");

	/* ---- HTML5 void elements and attributes ---- */
	fprintf(stderr, "[HTML5 features]\n");

	const char *html1 = "<!DOCTYPE html><html><head><meta charset=\"utf-8\"></head></html>";
	int nodes = html_parse_and_count_nodes(html1, strlen(html1));
	ASSERT(nodes > 0, "void <meta> element (no closing tag)");

	const char *html2 = "<div data-foo=\"bar\" data-id=\"123\">content</div>";
	nodes = html_parse_and_count_nodes(html2, strlen(html2));
	ASSERT(nodes > 0, "HTML5 data-* attributes");

	const char *html3 = "<img src=\"x.png\" alt=\"test\">";
	nodes = html_parse_and_count_nodes(html3, strlen(html3));
	ASSERT(nodes > 0, "void <img> element");

	const char *html4 = "<input type=\"text\" value=\"hello\">";
	nodes = html_parse_and_count_nodes(html4, strlen(html4));
	ASSERT(nodes > 0, "void <input> element");

	const char *html5 = "<br><hr><link rel=\"stylesheet\" href=\"x.css\">";
	nodes = html_parse_and_count_nodes(html5, strlen(html5));
	ASSERT(nodes > 0, "multiple void elements in sequence");

	/* ---- The repeating meta tag from the crash log ---- */
	fprintf(stderr, "\n[Crash reproducer: repeating meta tag]\n");

	/* Exact meta tag from Dolphin log (truncated in screenshot but likely full): */
	const char *meta_tag = "<meta content=\"width=device-width,maximum-scale=1.0,initial-scale=1.0,minimum-scale=1.0\">";
	nodes = html_parse_and_count_nodes(meta_tag, strlen(meta_tag));
	ASSERT(nodes > 0, "viewport meta tag (crash reproducer)");

	/* Stress: parse it 100 times to see if duplication is cumulative */
	const char *meta_stress =
		"<!DOCTYPE html><html><head>"
		"<meta content=\"width=device-width,maximum-scale=1.0,initial-scale=1.0,minimum-scale=1.0\">"
		"</head><body>test</body></html>";
	for (int i = 0; i < 100; i++)
	{
		nodes = html_parse_and_count_nodes(meta_stress, strlen(meta_stress));
		if (nodes < 0)
		{
			fprintf(stderr, "  FAIL: meta tag stress iteration %d returned error\n", i);
			failed++;
			break;
		}
	}
	ASSERT(1, "100-iteration meta tag stress (no infinite loop)");

	/* ---- Pathological inputs ---- */
	fprintf(stderr, "\n[Pathological inputs]\n");

	nodes = html_parse_and_count_nodes("", 0);
	ASSERT(nodes >= 0, "empty HTML");

	const char *p1 = "<";
	nodes = html_parse_and_count_nodes(p1, strlen(p1));
	ASSERT(nodes >= 0, "just '<'");

	const char *p2 = ">";
	nodes = html_parse_and_count_nodes(p2, strlen(p2));
	ASSERT(nodes >= 0, "just '>'");

	const char *p3 = "<<<<<>>>>>";
	nodes = html_parse_and_count_nodes(p3, strlen(p3));
	ASSERT(nodes >= 0, "garbage angle brackets");

	const char *p4 = "<div><div><div><div><div>";
	nodes = html_parse_and_count_nodes(p4, strlen(p4));
	ASSERT(nodes >= 0, "unclosed nested divs");

	const char *p5 = "</div></div></div>";
	nodes = html_parse_and_count_nodes(p5, strlen(p5));
	ASSERT(nodes >= 0, "closing tags without opens");

	/* 10 KB of '<' chars (stress tag-start path) */
	char *garbage = (char *)malloc(10240);
	memset(garbage, '<', 10239);
	garbage[10239] = 0;
	nodes = html_parse_and_count_nodes(garbage, 10239);
	ASSERT(nodes >= 0, "10 KB of '<' chars");
	free(garbage);

	/* ---- Entity decoding ---- */
	fprintf(stderr, "\n[HTML entity decoding]\n");

	const char *e1 = "&lt;&gt;&amp;&quot;&#39;";
	nodes = html_parse_and_count_nodes(e1, strlen(e1));
	ASSERT(nodes >= 0, "common HTML entities");

	const char *e2 = "&#65;&#x42;&#x1F600;";
	nodes = html_parse_and_count_nodes(e2, strlen(e2));
	ASSERT(nodes >= 0, "numeric entities (decimal + hex + emoji)");

	const char *e3 = "&nbsp;&copy;&mdash;&trade;";
	nodes = html_parse_and_count_nodes(e3, strlen(e3));
	ASSERT(nodes >= 0, "named entities");

	const char *e4 = "&unknown;&invalid123;";
	nodes = html_parse_and_count_nodes(e4, strlen(e4));
	ASSERT(nodes >= 0, "unknown entity names");

	/* ---- Real-world fixtures ---- */
	fprintf(stderr, "\n[Real-world HTML fixtures]\n");

	const char *fixtures[] = {
		"fixtures/google_home.html",  /* 30 KB */
		"fixtures/x_home.html",       /* 249 KB */
	};

	for (size_t fi = 0; fi < sizeof(fixtures) / sizeof(fixtures[0]); fi++)
	{
		size_t len = 0;
		char *html = read_file(fixtures[fi], &len);
		if (!html || len == 0)
		{
			fprintf(stderr, "  SKIP: %s (fixture missing)\n", fixtures[fi]);
			failed++;
			free(html);
			continue;
		}
		fprintf(stderr, "  %s: %zu bytes\n", fixtures[fi], len);

		nodes = html_parse_and_count_nodes(html, len);
		char msg1[256];
		snprintf(msg1, sizeof(msg1), "%s parses without ASAN report (got %d nodes)",
		         fixtures[fi], nodes);
		ASSERT(nodes > 0, msg1);

		/* Stress: parse 20 times (HTML trees are large, fewer iterations) */
		for (int i = 0; i < 20; i++)
		{
			int n = html_parse_and_count_nodes(html, len);
			if (n < 0)
			{
				fprintf(stderr, "  FAIL: iteration %d returned error\n", i);
				failed++;
				break;
			}
		}
		char msg2[256];
		snprintf(msg2, sizeof(msg2), "%s 20-iteration stress", fixtures[fi]);
		ASSERT(1, msg2);

		free(html);
	}

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}

#ifdef __cplusplus
}
#endif
