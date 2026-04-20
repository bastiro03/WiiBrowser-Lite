/* omnibox_test.c - Test the URL-vs-search heuristic and buffer sizing.
 *
 * Background: On the Wii, entering "x.com" would silently trigger a
 * Google search instead of a direct site visit when the fetch failed.
 * Worse, omniBox() had a heap-overflow: it allocated sizeof(new_page)+100
 * bytes for the search URL, but url_encode can expand input 3× (each
 * byte becomes %XX). A long query overflowed the buffer and corrupted
 * adjacent allocations — a plausible trigger for the _free_r crash the
 * user hit (DAR=0x0C).
 *
 * This test covers:
 *   1. looksLikeUrl() distinguishes URL-looking input from search queries.
 *   2. The buffer sizing used in omniBox() (3×+256) accommodates worst
 *      case URL-encoded input without overflow.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

/* Mirror of looksLikeUrl() in src/menu.cpp */
static bool looksLikeUrl(const char *input)
{
	if (!input || !*input) return false;
	if (strncmp(input, "http://", 7) == 0 || strncmp(input, "https://", 8) == 0)
		return true;
	bool has_space = false, has_dot = false;
	for (const char *p = input; *p; p++)
	{
		if (*p == ' ' || *p == '\t') has_space = true;
		else if (*p == '.') has_dot = true;
	}
	return has_dot && !has_space;
}

/* Mirror of url_encode() in src/urlcode.c */
static char to_hex(char code)
{
	static const char hex[] = "0123456789ABCDEF";
	return hex[code & 15];
}
static char *url_encode(const char *str)
{
	size_t len = strlen(str);
	char *buf = (char *)malloc(len * 3 + 1);
	char *pbuf = buf;
	for (const char *pstr = str; *pstr; pstr++)
	{
		if (isalnum((unsigned char)*pstr) || *pstr == '-' || *pstr == '_' ||
		    *pstr == '.' || *pstr == '~')
			*pbuf++ = *pstr;
		else if (*pstr == ' ')
			*pbuf++ = '+';
		else
		{
			*pbuf++ = '%';
			*pbuf++ = to_hex(*pstr >> 4);
			*pbuf++ = to_hex(*pstr & 15);
		}
	}
	*pbuf = '\0';
	return buf;
}

/* Fixed-version omniBox (mirror of new src/menu.cpp:1862) */
static char *fixed_omniBox(const char *input)
{
	char *encode = url_encode(input);
	size_t bufsize = strlen(encode) + 256;
	char *url = (char *)malloc(bufsize);
	snprintf(url, bufsize,
	         "http://www.google.com/search?hl=en&source=hp&biw=&bih=&q=%s&btnG=Google+Search&gbv=1",
	         encode);
	free(encode);
	return url;
}

int main(void)
{
	fprintf(stderr, "Testing omniBox URL/search heuristic + buffer sizing:\n\n");

	/* ---- looksLikeUrl heuristic ---- */
	fprintf(stderr, "[looksLikeUrl classification]\n");

	ASSERT(looksLikeUrl("x.com"),               "x.com → URL");
	ASSERT(looksLikeUrl("wikipedia.org"),       "wikipedia.org → URL");
	ASSERT(looksLikeUrl("en.wikipedia.org"),    "en.wikipedia.org → URL");
	ASSERT(looksLikeUrl("http://x.com"),        "http://x.com → URL");
	ASSERT(looksLikeUrl("https://x.com/path"),  "https://x.com/path → URL");
	ASSERT(looksLikeUrl("192.168.1.1"),         "IP address → URL");
	ASSERT(looksLikeUrl("foo.bar.baz"),         "multi-dot hostname → URL");

	ASSERT(!looksLikeUrl("hello world"),        "'hello world' → search query (space)");
	ASSERT(!looksLikeUrl("cats"),               "single word → search query");
	ASSERT(!looksLikeUrl("what is 2+2"),        "multi-word → search query");
	ASSERT(!looksLikeUrl(""),                   "empty → not URL");
	ASSERT(!looksLikeUrl(NULL),                 "NULL → not URL");
	ASSERT(!looksLikeUrl("x.com with space"),   "'x.com with space' → query (space present)");

	/* ---- Buffer sizing: old bug scenario ---- */
	fprintf(stderr, "\n[omniBox buffer sizing]\n");

	/* Worst-case input: all non-alnum chars → each becomes 3 bytes encoded.
	 * MAXLEN=512 input → 1536 encoded + 90-char format = 1626 bytes total.
	 * Previous alloc of sizeof(new_page)+100 = 612 bytes overflowed by 1000.
	 * Test: a 500-char pathological input with special chars should not
	 * overflow with the new sizing. */
	char bad_input[501];
	for (int i = 0; i < 500; i++) bad_input[i] = '?';   /* ? → %3F → 3× expansion */
	bad_input[500] = 0;

	char *u = fixed_omniBox(bad_input);
	size_t ulen = strlen(u);
	char msg1[128];
	snprintf(msg1, sizeof(msg1), "pathological 500-char query → %zu-byte URL (no overflow)", ulen);
	ASSERT(ulen > 1500, msg1);                   /* sanity: real expansion happened */
	ASSERT(ulen < 2000, "URL fits well within new buffer");
	free(u);

	/* Also trivial case */
	u = fixed_omniBox("cats");
	ASSERT(strstr(u, "q=cats") != NULL, "simple query 'cats' appears in URL");
	free(u);

	u = fixed_omniBox("hello world");
	ASSERT(strstr(u, "q=hello+world") != NULL, "spaces → + in encoded query");
	free(u);

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
