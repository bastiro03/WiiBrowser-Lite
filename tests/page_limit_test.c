/* page_limit_test.c - WriteMemoryCallback cap + graceful OOM tests.
 *
 * Background: the previous WriteMemoryCallback called exit(EXIT_FAILURE)
 * when realloc returned NULL, killing the whole app on any transient
 * memory pressure (or on a runaway page). It also had no upper bound on
 * page size — a chunked response with no Content-Length could balloon
 * MemoryStruct until the app OOMed hard.
 *
 * New behavior (src/network/httplib.cpp:WriteMemoryCallback):
 *   - Hard cap at WBL_MAX_PAGE_BYTES (8 MiB).
 *   - On realloc failure OR cap breach, return 0 → curl aborts with
 *     CURLE_WRITE_ERROR, the UI can surface an error gracefully.
 *
 * This test mirrors the logic and verifies the contract without
 * pulling in libcurl.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WBL_MAX_PAGE_BYTES (8u * 1024u * 1024u)

typedef struct {
	char  *memory;
	size_t size;
	int    realloc_inject_fail;  /* test knob: force realloc to "fail" */
} MemoryStruct;

/* Mirror of src/network/httplib.cpp:WriteMemoryCallback(). */
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb,
                                  void *userp)
{
	size_t realsize = size * nmemb;
	MemoryStruct *mem = (MemoryStruct *)userp;

	if (mem->size + realsize > WBL_MAX_PAGE_BYTES)
		return 0;

	char *grown;
	if (mem->realloc_inject_fail)
		grown = NULL;  /* simulated OOM */
	else
		grown = (char *)realloc(mem->memory, mem->size + realsize + 1);

	if (grown == NULL)
		return 0;

	mem->memory = grown;
	memcpy(&(mem->memory[mem->size]), contents, realsize);
	mem->memory[mem->size += realsize] = 0;
	return realsize;
}

static int passed, failed;

#define ASSERT(cond, msg) do { \
	if (!(cond)) { fprintf(stderr, "  FAIL: %s\n", msg); failed++; } \
	else         { fprintf(stderr, "  PASS: %s\n", msg); passed++; } \
} while (0)

int main(void)
{
	MemoryStruct m = {0};
	size_t rc;

	fprintf(stderr, "Testing WriteMemoryCallback cap + OOM:\n\n");

	/* ---- Normal small write accepted ---- */
	fprintf(stderr, "[normal small response]\n");
	m.memory = calloc(1, 1); m.size = 0;
	rc = WriteMemoryCallback("hello", 1, 5, &m);
	ASSERT(rc == 5, "5-byte chunk returns realsize");
	ASSERT(m.size == 5, "size advanced to 5");
	ASSERT(memcmp(m.memory, "hello", 5) == 0, "data buffered correctly");
	ASSERT(m.memory[5] == 0, "buffer is NUL-terminated");
	free(m.memory); memset(&m, 0, sizeof(m));

	/* ---- Multiple small writes accumulate ---- */
	fprintf(stderr, "\n[streaming small chunks]\n");
	m.memory = calloc(1, 1); m.size = 0;
	WriteMemoryCallback("abc", 1, 3, &m);
	WriteMemoryCallback("def", 1, 3, &m);
	rc = WriteMemoryCallback("ghi", 1, 3, &m);
	ASSERT(m.size == 9, "accumulated size is 9 bytes after 3x 3-byte writes");
	ASSERT(memcmp(m.memory, "abcdefghi", 9) == 0, "chunks concatenated in order");
	ASSERT(rc == 3, "last call returned its own realsize (3)");
	free(m.memory); memset(&m, 0, sizeof(m));

	/* ---- Cap rejects oversize response ---- */
	fprintf(stderr, "\n[cap at WBL_MAX_PAGE_BYTES]\n");
	m.memory = calloc(1, 1); m.size = 0;
	/* Pretend 7.99 MiB is already buffered, then try to add 20 KiB */
	char filler[20 * 1024];
	memset(filler, 'x', sizeof(filler));
	m.size = WBL_MAX_PAGE_BYTES - 1024;  /* 8 MiB - 1 KiB */
	m.memory = realloc(m.memory, m.size + 1);
	rc = WriteMemoryCallback(filler, 1, sizeof(filler), &m);
	ASSERT(rc == 0,
	       "chunk that would push past 8 MiB cap is rejected (returns 0)");
	/* m.size unchanged — rejection must not advance the buffer */
	ASSERT(m.size == WBL_MAX_PAGE_BYTES - 1024,
	       "rejected chunk does NOT advance mem->size");
	free(m.memory); memset(&m, 0, sizeof(m));

	/* ---- Cap allows exactly-at-limit write ---- */
	fprintf(stderr, "\n[cap boundary — exactly at limit]\n");
	m.memory = calloc(1, 1); m.size = 0;
	m.size = WBL_MAX_PAGE_BYTES - 100;
	m.memory = realloc(m.memory, m.size + 1);
	char little[100];
	memset(little, 'y', sizeof(little));
	rc = WriteMemoryCallback(little, 1, sizeof(little), &m);
	ASSERT(rc == 100,
	       "chunk that exactly fills the cap is accepted");
	ASSERT(m.size == WBL_MAX_PAGE_BYTES,
	       "mem->size is exactly at cap after accept");
	free(m.memory); memset(&m, 0, sizeof(m));

	/* ---- realloc failure returns 0 (no exit()) ---- */
	fprintf(stderr, "\n[graceful OOM: no exit(), curl aborts transfer]\n");
	m.memory = calloc(1, 1); m.size = 0;
	m.realloc_inject_fail = 1;
	rc = WriteMemoryCallback("data", 1, 4, &m);
	ASSERT(rc == 0,
	       "realloc failure returns 0 (not exit() — transfer aborts only)");
	ASSERT(m.size == 0, "realloc failure does NOT advance size");
	free(m.memory); memset(&m, 0, sizeof(m));

	fprintf(stderr, "\n========================================\n");
	fprintf(stderr, "Summary: %d passed, %d failed\n", passed, failed);
	fprintf(stderr, "RESULT %s\n", failed == 0 ? "PASS" : "FAIL");
	return failed > 0 ? 1 : 0;
}
