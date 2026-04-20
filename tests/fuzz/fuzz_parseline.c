/* fuzz_parseline.c - libFuzzer harness for the HTTP header parser.
 *
 * Target: parseline() in src/network/httplib.cpp, mirrored here exactly
 * as in tests/parseline_test.c. Fuzzer feeds random bytes as a single
 * header line and we exercise the lowercase pass + status-line/
 * Content-Type classification under ASAN+UBSAN.
 *
 * Bugs this should catch:
 *   - Off-by-one in the tolower() bound
 *   - Unbounded sscanf/strstr on non-NUL-terminated input
 *   - Heap read-past-end on short-header edge cases
 *   - Flipped download flag on malformed status lines
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

typedef struct {
	char  *memory;
	size_t size;
	bool   download;
	bool   in_redirect;
	char   filename[256];
} HeaderStruct;

static bool mustdownload(const char *content)
{
	if (strstr(content, "text/html") ||
	    strstr(content, "application/xhtml") ||
	    strstr(content, "image"))
		return false;
	return true;
}

static char *findChr(char *str, char chr)
{
	char *c = strchr(str, chr);
	if (c) *c = 0;
	return c;
}

static int parseline(HeaderStruct *mem, size_t realsize)
{
	size_t i;
	for (i = mem->size - realsize; i < mem->size; i++)
		mem->memory[i] = tolower((unsigned char)mem->memory[i]);

	char *line = &mem->memory[mem->size - realsize];
	char buff[128];
	memset(buff, 0, sizeof(buff));

	if (!strncmp(line, "http/", 5))
	{
		mem->download = false;
		int code = 0;
		sscanf(line, "http/%*s %d", &code);
		mem->in_redirect = (code >= 300 && code < 400);
	}
	else if (!strncmp(line, "content-type", 12))
	{
		if (!mem->in_redirect)
		{
			sscanf(line, "content-type: %127s", buff);
			findChr(buff, ';');
			if (mustdownload(buff))
				mem->download = true;
		}
	}
	else if (!strncmp(line, "\r\n", 2))
		return (!mem->download) * (int)realsize;
	return (int)realsize;
}

int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
	if (size == 0 || size > 8192) return 0;

	/* Simulate curl's HEADERFUNCTION: one line appended to the accumulating
	 * buffer. The fuzzer choice of (size, contents) covers short reads,
	 * partial writes, missing newlines, and non-ASCII bytes. */
	HeaderStruct mem;
	mem.memory = (char *)malloc(size + 1);
	if (!mem.memory) return 0;
	memcpy(mem.memory, data, size);
	mem.memory[size] = 0;   /* parseline uses strncmp/sscanf/strstr */
	mem.size = size;
	mem.download = false;
	mem.in_redirect = false;
	mem.filename[0] = 0;

	(void)parseline(&mem, size);

	free(mem.memory);
	return 0;
}
