/* parseline_test.c - Unit tests for HTTP header-parser logic.
 *
 * parseline (in src/network/httplib.cpp) runs once per HTTP header line.
 * Contract:
 *   - Lowercases the current line in-place (headers are case-insensitive).
 *   - On "content-type:", classifies the response and sets download flag.
 *   - On "content-disposition:", captures filename for download.
 *   - On "\r\n" (end-of-headers):
 *       * returns 0       → curl aborts body transfer (download path)
 *       * returns realsize → curl continues, body gets rendered
 *   - Otherwise returns realsize so curl keeps streaming headers.
 *
 * A regression here is catastrophic: return-value of 0 on a renderable
 * Content-Type would abort EVERY page fetch with "Recv failure: Success"
 * equivalent. This test mirrors the parseline() logic so we catch any
 * accidental inversion of the download flag or return-value flip.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct {
	char  *memory;
	size_t size;
	bool   download;
	bool   in_redirect;   /* true while parsing headers of a 3xx response */
	char   filename[256];
} HeaderStruct;

/* mustdownload mirrored from httplib.cpp */
static bool mustdownload(const char *content)
{
	if (strstr(content, "text/html") ||
	    strstr(content, "application/xhtml") ||
	    strstr(content, "image"))
		return false;
	return true;
}

/* findChr mirrored from httplib.cpp */
static char *findChr(char *str, char chr)
{
	char *c = strchr(str, chr);
	if (c) *c = 0;
	return c;
}

/* parseline mirrored from httplib.cpp:parseline(). Must stay in sync. */
static int parseline(HeaderStruct *mem, size_t realsize)
{
	/* Upper bound is `<`, not `<=`, to avoid tolower'ing the NUL past data */
	size_t i;
	for (i = mem->size - realsize; i < mem->size; i++)
		mem->memory[i] = tolower(mem->memory[i]);

	char *line = &mem->memory[mem->size - realsize];
	char buff[128];
	memset(buff, 0, sizeof(buff));

	/* Status line ("HTTP/1.1 302 ...", "HTTP/2 302 ...") starts a new
	 * response. curl calls HEADERFUNCTION once per header line INCLUDING
	 * intermediate redirect responses. Two regressions to avoid:
	 *   1. Leaking download=true from a 3xx hop's Content-Type to a later
	 *      hop's end-of-headers — reset download on every status line.
	 *   2. Setting download=true on a 3xx hop (e.g. x.com 302 text/plain)
	 *      which would abort curl's redirect follow — track in_redirect
	 *      and skip Content-Type classification while in a 3xx response. */
	if (!strncmp(line, "http/", 5))
	{
		mem->download = false;
		int code = 0;
		/* "http/1.1 302 ..." or "http/2 302 ..." — sscanf handles both */
		sscanf(line, "http/%*s %d", &code);
		mem->in_redirect = (code >= 300 && code < 400);
	}
	else if (!strncmp(line, "content-type", 12))
	{
		/* Skip Content-Type classification on 3xx responses: the Content-Type
		 * there describes the redirect-explainer body, not the final target. */
		if (!mem->in_redirect)
		{
			/* Bounded width so a pathological Content-Type cannot overrun */
			sscanf(line, "content-type: %127s", buff);
			findChr(buff, ';');
			if (mustdownload(buff))
				mem->download = true;
		}
	}
	else if (!strncmp(line, "content-disposition", 19))
	{
		snprintf(mem->filename, sizeof(mem->filename), "%s", line);
	}
	else if (!strncmp(line, "\r\n", 2))
		return (!mem->download) * realsize;

	return realsize;
}

/* Feed a single HTTP header line into a fresh HeaderStruct. */
static int feed_header(HeaderStruct *h, const char *line)
{
	size_t n = strlen(line);
	size_t off = h->size;
	h->memory = realloc(h->memory, h->size + n + 1);
	memcpy(h->memory + off, line, n);
	h->size += n;
	h->memory[h->size] = 0;
	return parseline(h, n);
}

static void hs_reset(HeaderStruct *h)
{
	free(h->memory);
	h->memory = calloc(1, 1);
	h->size = 0;
	h->download = false;
	h->in_redirect = false;
	h->filename[0] = 0;
}

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

int main(void)
{
	HeaderStruct h = {0};
	int rc;

	fprintf(stderr, "Testing parseline() header handling:\n\n");

	/* ---- HTML page: text/html → render ---- */
	fprintf(stderr, "[text/html page (wikipedia.org)]\n");
	hs_reset(&h);
	rc = feed_header(&h, "HTTP/1.1 200 OK\r\n");
	ASSERT(rc == (int)strlen("HTTP/1.1 200 OK\r\n"),
	       "status line returns realsize");
	rc = feed_header(&h, "Content-Type: text/html; charset=UTF-8\r\n");
	ASSERT(!h.download,
	       "text/html with charset does NOT set download flag");
	ASSERT(rc > 0, "content-type line returns realsize");
	rc = feed_header(&h, "\r\n");
	ASSERT(rc > 0,
	       "end-of-headers returns realsize (>0) when NOT a download");
	ASSERT(rc == (int)strlen("\r\n"),
	       "end-of-headers return value equals realsize exactly");

	/* ---- PDF: triggers download path ---- */
	fprintf(stderr, "\n[application/pdf download]\n");
	hs_reset(&h);
	rc = feed_header(&h, "Content-Type: application/pdf\r\n");
	ASSERT(h.download, "application/pdf sets download flag");
	rc = feed_header(&h, "\r\n");
	ASSERT(rc == 0,
	       "end-of-headers returns 0 → curl aborts for download path");

	/* ---- Image: no download ---- */
	fprintf(stderr, "\n[image/png inline]\n");
	hs_reset(&h);
	rc = feed_header(&h, "Content-Type: image/png\r\n");
	ASSERT(!h.download, "image/png does NOT set download flag");
	rc = feed_header(&h, "\r\n");
	ASSERT(rc > 0, "image end-of-headers returns realsize (>0)");

	/* ---- Case-insensitive header handling ---- */
	fprintf(stderr, "\n[case-insensitive headers]\n");
	hs_reset(&h);
	rc = feed_header(&h, "CONTENT-TYPE: TEXT/HTML\r\n");
	ASSERT(!h.download,
	       "uppercase CONTENT-TYPE: TEXT/HTML still recognized (tolower)");

	/* ---- XHTML page ---- */
	fprintf(stderr, "\n[application/xhtml+xml]\n");
	hs_reset(&h);
	rc = feed_header(&h, "Content-Type: application/xhtml+xml; charset=utf-8\r\n");
	ASSERT(!h.download, "application/xhtml does NOT set download flag");

	/* ---- Reset between pages must clear download flag ---- */
	fprintf(stderr, "\n[fresh HeaderStruct clears prior state]\n");
	hs_reset(&h);
	ASSERT(h.download == false, "hs_reset clears download flag");
	ASSERT(h.filename[0] == 0,  "hs_reset clears filename[0]");

	/* ---- Long Content-Disposition does NOT overflow filename[256] ---- */
	fprintf(stderr, "\n[long Content-Disposition header]\n");
	hs_reset(&h);
	char long_line[512];
	/* RFC 5987 filename*=UTF-8''<percent-encoded...> can be very long */
	memset(long_line, 0, sizeof(long_line));
	strcpy(long_line, "Content-Disposition: attachment; filename=\"");
	size_t prefix_len = strlen(long_line);
	memset(long_line + prefix_len, 'A', 400);
	strcpy(long_line + prefix_len + 400, "\"\r\n");
	feed_header(&h, long_line);
	ASSERT(strlen(h.filename) < sizeof(h.filename),
	       "filename[] stays within bounds for a 400-byte filename header");
	ASSERT(h.filename[sizeof(h.filename) - 1] == 0 ||
	       strlen(h.filename) == sizeof(h.filename) - 1,
	       "filename[] is NUL-terminated after truncation");

	/* ---- Long Content-Type does NOT overflow buff ---- */
	fprintf(stderr, "\n[pathological long Content-Type]\n");
	hs_reset(&h);
	char long_ct[512];
	strcpy(long_ct, "Content-Type: ");
	size_t ct_prefix = strlen(long_ct);
	memset(long_ct + ct_prefix, 'x', 400);
	strcpy(long_ct + ct_prefix + 400, "\r\n");
	/* If the sscanf width spec is missing, this overruns buff[128] on
	 * stack. With the %127s cap the write is bounded and we just get a
	 * truncated "xxxx..." in buff (classified as download, not HTML). */
	feed_header(&h, long_ct);
	feed_header(&h, "\r\n");  /* end of headers */
	/* We're mainly testing that we didn't crash with -fsanitize=address.
	 * A 400-char "xxx..." doesn't match "text/html" so it gets flagged
	 * as a download — that's fine; the cap is about safety, not content. */
	ASSERT(h.download == true,
	       "long bogus Content-Type is classified as download");

	/* ---- Case-insensitive recognition through parseline+mustdownload ---- */
	fprintf(stderr, "\n[Case-insensitive end-to-end through parseline]\n");
	hs_reset(&h);
	/* parseline lowercases the header line, so even TEXT/HTML reaches
	 * mustdownload as "text/html" — but if the tolower loop regressed,
	 * this would flip to download=true. */
	feed_header(&h, "Content-Type: TEXT/HTML; charset=UTF-8\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(!h.download,
	       "uppercase Content-Type after tolower reaches mustdownload as lowercase");
	ASSERT(rc > 0,
	       "uppercase text/html → end-of-headers returns realsize");

	/* ---- x.com 302 redirect regression ----
	 * x.com returns 302 with Content-Type: text/plain + Location header.
	 * If we abort here (download=true because text/plain→download), curl
	 * NEVER follows the Location redirect. Result: user sees Download modal
	 * instead of the redirected page. The fix: reset download flag on each
	 * new status line so only the FINAL response's Content-Type decides.
	 *
	 * Real x.com response observed 2026-04-19 with iOS watchOS UA:
	 *   HTTP/1.1 302 Found
	 *   content-type: text/plain; charset=utf-8
	 *   location: x-safari-https://redirect.x.com/?ct=rw-null
	 */
	fprintf(stderr, "\n[x.com 302 redirect: must NOT abort for text/plain mid-redirect]\n");
	hs_reset(&h);
	feed_header(&h, "HTTP/1.1 302 Found\r\n");
	feed_header(&h, "Content-Type: text/plain; charset=utf-8\r\n");
	feed_header(&h, "Location: https://redirect.x.com/?ct=rw-null\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(rc > 0,
	       "302 end-of-headers returns realsize (don't abort 3xx, let curl follow Location)");
	ASSERT(!h.download,
	       "3xx redirect response does NOT set download flag (cleared by status line)");

	/* ---- Multi-hop redirect chain: final 200 text/html should render ----
	 * After following a 301/302 chain, the final 200 response's Content-Type
	 * is what matters. If we leaked download=true from an earlier hop with
	 * Content-Type: text/plain, we'd incorrectly download an HTML page. */
	fprintf(stderr, "\n[Redirect chain 301 text/plain → 200 text/html]\n");
	hs_reset(&h);
	feed_header(&h, "HTTP/1.1 301 Moved Permanently\r\n");
	feed_header(&h, "Content-Type: text/plain\r\n");
	feed_header(&h, "Location: https://example.com/\r\n");
	feed_header(&h, "\r\n");
	/* New redirect hop: status line resets download flag */
	feed_header(&h, "HTTP/1.1 200 OK\r\n");
	feed_header(&h, "Content-Type: text/html; charset=utf-8\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(!h.download,
	       "final 200 text/html renders (prior 301 text/plain cleared)");
	ASSERT(rc > 0,
	       "final response end-of-headers returns realsize");

	/* ---- Final response IS a download: 200 application/pdf ----
	 * After optional redirects, if the final response is a PDF, we SHOULD
	 * abort with download flag. Verify this still works. */
	fprintf(stderr, "\n[Redirect chain 302 → 200 application/pdf: download]\n");
	hs_reset(&h);
	feed_header(&h, "HTTP/1.1 302 Found\r\n");
	feed_header(&h, "Content-Type: text/html\r\n");
	feed_header(&h, "Location: https://cdn.example.com/file.pdf\r\n");
	feed_header(&h, "\r\n");
	feed_header(&h, "HTTP/1.1 200 OK\r\n");
	feed_header(&h, "Content-Type: application/pdf\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(h.download,
	       "final 200 application/pdf sets download flag");
	ASSERT(rc == 0,
	       "final application/pdf end-of-headers returns 0 to abort");

	/* ---- Single 200 response (no redirect) still works ----
	 * The status-line reset should not regress plain non-redirect responses. */
	fprintf(stderr, "\n[Single 200 response: regression check]\n");
	hs_reset(&h);
	feed_header(&h, "HTTP/1.1 200 OK\r\n");
	feed_header(&h, "Content-Type: application/pdf\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(h.download, "200 PDF still triggers download after status-line reset");
	ASSERT(rc == 0, "200 PDF still returns 0 at end-of-headers");

	/* ---- HTTP/2 status line ---- */
	fprintf(stderr, "\n[HTTP/2 status line recognition]\n");
	hs_reset(&h);
	feed_header(&h, "HTTP/2 302\r\n");
	feed_header(&h, "Content-Type: text/plain\r\n");
	feed_header(&h, "Location: https://example.com/\r\n");
	rc = feed_header(&h, "\r\n");
	ASSERT(!h.download,
	       "HTTP/2 302 response: text/plain does not trigger download");
	ASSERT(rc > 0, "HTTP/2 302 end-of-headers returns realsize");

	/* ---- Cleanup ---- */
	free(h.memory);

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
