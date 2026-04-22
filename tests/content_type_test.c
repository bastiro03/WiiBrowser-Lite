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
#include <strings.h>
#include <stdbool.h>

/* Types mirror html.cpp */
typedef enum { UNKNOWN = 0, WEB, TEXT, IMAGE, VIDEO } content_type_t;

/* Production logic, kept in sync with src/network/httplib.cpp.
 * Case-insensitive so callers that don't pre-lowercase the raw value
 * from CURLINFO_CONTENT_TYPE still classify correctly. */
static bool mustdownload(const char *content)
{
	if (strcasestr(content, "text/html") ||
	    strcasestr(content, "application/xhtml") ||
	    strcasestr(content, "image")
#ifdef WBL_MPLAYER
	    || strcasestr(content, "video")
#endif
	)
		return false;
	return true;
}

/* Production logic, kept in sync with src/html.cpp:knownType()
 * (case-insensitive — RFC 7231 says Content-Type type/subtype is
 * case-insensitive, so "TEXT/HTML" is equivalent to "text/html"). */
static content_type_t knownType(const char *type)
{
	if (!strcasecmp(type, "text/html") || strcasestr(type, "application/xhtml"))
		return WEB;
	if (strcasestr(type, "text"))
		return TEXT;
	if (strcasestr(type, "image"))
		return IMAGE;
	if (strcasestr(type, "video"))
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
	/* knownType uses strcasecmp on the exact "text/html" value. In
	 * practice getrequest() calls findChr(ct, ';') which strips the
	 * charset suffix before knownType() ever sees the type, so the
	 * charset-decorated case routes via the strcasestr fallback. */
	ASSERT(knownType("text/html; charset=utf-8") == TEXT,
	       "text/html;charset=... reaches TEXT via strcasestr fallback (charset would normally be stripped earlier)");
	ASSERT(knownType("application/xhtml+xml") == WEB,
	       "application/xhtml → WEB");

	/* Case-insensitive classification (RFC 7231) */
	fprintf(stderr, "\n[Case-insensitive Content-Type (RFC 7231)]\n");
	ASSERT(knownType("TEXT/HTML") == WEB,
	       "uppercase TEXT/HTML → WEB (strcasecmp)");
	ASSERT(knownType("Text/Html") == WEB,
	       "mixed-case Text/Html → WEB");
	ASSERT(knownType("APPLICATION/XHTML+XML") == WEB,
	       "uppercase XHTML → WEB (strcasestr)");
	ASSERT(knownType("IMAGE/PNG") == IMAGE,
	       "uppercase IMAGE/PNG → IMAGE");
	ASSERT(!mustdownload("TEXT/HTML"),
	       "uppercase TEXT/HTML not classified as download");
	ASSERT(!mustdownload("Image/JPEG"),
	       "mixed-case Image/JPEG not classified as download");

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
