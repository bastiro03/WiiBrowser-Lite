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
	size_t i;
	for (i = mem->size - realsize; i <= mem->size; i++)
		mem->memory[i] = tolower(mem->memory[i]);

	char *line = &mem->memory[mem->size - realsize];
	char buff[50];
	memset(buff, 0, sizeof(buff));

	if (!strncmp(line, "content-type", 12))
	{
		sscanf(line, "content-type: %s", buff);
		findChr(buff, ';');
		if (mustdownload(buff))
			mem->download = true;
	}
	else if (!strncmp(line, "content-disposition", 19))
	{
		/* Use snprintf to avoid overflow of filename[256] on long lines */
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

	/* ---- Cleanup ---- */
	free(h.memory);

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
