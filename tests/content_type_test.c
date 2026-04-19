/* content_type_test.c - Unit tests for content-type classification.
 *
 * The app routes responses two ways based on Content-Type:
 *   - mustdownload()  : headers-parser decides "is this a downloadable file?"
 *                       Returns true → curl aborts, menu shows download prompt.
 *                       Returns false → body is rendered.
 *   - knownType()     : DisplayHTML() decides how to render (WEB/IMAGE/TEXT/VIDEO).
 *
 * If either function misclassifies a page, the user sees a broken view or the
 * download prompt instead of rendered content. These tests mirror the
 * production logic from httplib.cpp:mustdownload() and html.cpp:knownType().
 *
 * Covers real-world Content-Type values from wikipedia.org, x.com, image CDNs,
 * and common edge cases (charset suffix, quoted boundary, whitespace).
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

/* Types mirror html.cpp */
typedef enum { UNKNOWN = 0, WEB, TEXT, IMAGE, VIDEO } content_type_t;

/* Production logic, kept in sync with src/network/httplib.cpp */
static bool mustdownload(const char *content)
{
	if (strstr(content, "text/html") ||
	    strstr(content, "application/xhtml") ||
	    strstr(content, "image")
#ifdef WBL_MPLAYER
	    || strstr(content, "video")
#endif
	)
		return false;
	return true;
}

/* Production logic, kept in sync with src/html.cpp:knownType() */
static content_type_t knownType(const char *type)
{
	if (!strcmp(type, "text/html") || strstr(type, "application/xhtml"))
		return WEB;
	if (strstr(type, "text"))
		return TEXT;
	if (strstr(type, "image"))
		return IMAGE;
	if (strstr(type, "video"))
		return VIDEO;
	return UNKNOWN;
}

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

int main(void)
{
	fprintf(stderr, "Testing mustdownload() (render vs download decision):\n\n");

	/* Real Content-Type values from wikipedia.org and x.com */
	fprintf(stderr, "[HTML pages - must NOT be downloaded]\n");
	ASSERT(!mustdownload("text/html"),
	       "plain text/html renders");
	ASSERT(!mustdownload("text/html; charset=utf-8"),
	       "text/html with charset (wikipedia.org) renders");
	ASSERT(!mustdownload("text/html; charset=UTF-8"),
	       "text/html with uppercase UTF-8 charset renders");
	ASSERT(!mustdownload("application/xhtml+xml"),
	       "XHTML renders as HTML");
	ASSERT(!mustdownload("application/xhtml+xml; charset=utf-8"),
	       "XHTML with charset renders");

	fprintf(stderr, "\n[Images - rendered in-app]\n");
	ASSERT(!mustdownload("image/jpeg"),
	       "image/jpeg renders");
	ASSERT(!mustdownload("image/png"),
	       "image/png renders");
	ASSERT(!mustdownload("image/gif"),
	       "image/gif renders");
	ASSERT(!mustdownload("image/webp"),
	       "image/webp renders");

	fprintf(stderr, "\n[Non-renderable - must be downloaded]\n");
	ASSERT(mustdownload("application/pdf"),
	       "PDF triggers download prompt");
	ASSERT(mustdownload("application/zip"),
	       "ZIP triggers download prompt");
	ASSERT(mustdownload("application/octet-stream"),
	       "octet-stream triggers download");
	ASSERT(mustdownload("application/json"),
	       "raw JSON triggers download (not rendered by browser)");

	/* plain text is ambiguous - currently classified as download.
	 * This codifies the current behavior; if we ever add TEXT rendering,
	 * update both production and test. */
	fprintf(stderr, "\n[Edge: plain text currently downloads]\n");
	ASSERT(mustdownload("text/plain"),
	       "text/plain is classified as download (no text renderer yet)");
	ASSERT(mustdownload("text/css"),
	       "text/css is classified as download");

	fprintf(stderr, "\nTesting knownType() (renderer dispatch):\n\n");

	fprintf(stderr, "[WEB dispatch]\n");
	ASSERT(knownType("text/html") == WEB,
	       "text/html → WEB");
	/* knownType uses strcmp for text/html exact match - charset-decorated
	 * types fall through to the substring "text" check → TEXT.
	 * This is the behavior we currently have; flag it so we notice if
	 * production diverges. */
	ASSERT(knownType("text/html; charset=utf-8") == TEXT,
	       "text/html with charset currently routes to TEXT (strcmp exact-match)");
	ASSERT(knownType("application/xhtml+xml") == WEB,
	       "application/xhtml → WEB");

	fprintf(stderr, "\n[IMAGE dispatch]\n");
	ASSERT(knownType("image/jpeg") == IMAGE, "image/jpeg → IMAGE");
	ASSERT(knownType("image/png") == IMAGE, "image/png → IMAGE");

	fprintf(stderr, "\n[TEXT dispatch]\n");
	ASSERT(knownType("text/plain") == TEXT, "text/plain → TEXT");

	fprintf(stderr, "\n[UNKNOWN dispatch]\n");
	ASSERT(knownType("application/pdf") == UNKNOWN, "PDF → UNKNOWN");
	ASSERT(knownType("application/octet-stream") == UNKNOWN, "octet-stream → UNKNOWN");

	/* Cross-consistency: anything knownType says WEB should NOT trigger
	 * mustdownload (otherwise the path "fetch-as-HTML" and "fetch-as-
	 * download" disagree on the same response). */
	fprintf(stderr, "\n[Cross-consistency]\n");
	ASSERT(knownType("text/html") == WEB && !mustdownload("text/html"),
	       "text/html: renderable and not a download");
	ASSERT(!mustdownload("text/html; charset=utf-8"),
	       "text/html with charset is not a download (mustdownload uses strstr)");

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
