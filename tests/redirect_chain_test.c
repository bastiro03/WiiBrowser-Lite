/* redirect_chain_test.c - Tests multi-hop redirect chains with varying
 * hostnames, protocols, and subdomains.
 *
 * The goal: catch redirect-chain bugs up-front in unit tests so we don't
 * need full e2e Wii testing for each new redirect pattern. Real sites
 * chain redirects unpredictably:
 *   - http://x.com → https://x.com (protocol upgrade)
 *   - https://x.com → https://www.x.com (bare → www)
 *   - https://www.x.com → https://ssl.x.com (arbitrary subdomain)
 *   - https://ssl.x.com → https://www.x.com/home (different path, same host)
 *
 * The static www/bare fix in wbl_build_resolve_list() handles cases 1+2
 * but NOT arbitrary-subdomain redirects (case 3). For those we need a
 * DYNAMIC resolution hook that extracts hostnames from curl's
 * "Issue another request" debug log and appends resolve entries on
 * the fly.
 *
 * This test exercises the parsing + append logic without requiring a
 * real network (which we can't control in CI).
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <curl/curl.h>

/* ---------------------------------------------------------------------
 * Logic under test (mirrored from wbl_resolve.c - would be extracted
 * there once the hook is in place).
 * --------------------------------------------------------------------- */

/* Extract a hostname from a curl-debug "Issue another request" log line.
 * The message format is:
 *   "Issue another request to this URL: 'https://www.x.com/'\n"
 * We return the hostname ("www.x.com") in out_host. Returns 1 on success,
 * 0 if the line isn't a redirect message. */
static int wbl_resolve_extract_host_from_debug(const char *data, size_t size,
                                               char *out_host, size_t out_size)
{
	static const char MARKER[] = "Issue another request to this URL: '";
	size_t marker_len = sizeof(MARKER) - 1;

	if (size <= marker_len) return 0;
	if (memcmp(data, MARKER, marker_len) != 0) return 0;

	const char *url_start = data + marker_len;
	const char *url_end = (const char *)memchr(url_start, '\'', size - marker_len);
	if (!url_end) return 0;

	size_t url_len = url_end - url_start;
	char url[512];
	if (url_len >= sizeof(url)) return 0;
	memcpy(url, url_start, url_len);
	url[url_len] = 0;

	/* Parse URL to get hostname. */
	CURLU *u = curl_url();
	if (!u) return 0;
	if (curl_url_set(u, CURLUPART_URL, url, 0) != CURLUE_OK) {
		curl_url_cleanup(u);
		return 0;
	}
	char *host = NULL;
	if (curl_url_get(u, CURLUPART_HOST, &host, 0) != CURLUE_OK || !host) {
		curl_url_cleanup(u);
		return 0;
	}
	int ok = (strlen(host) < out_size);
	if (ok) snprintf(out_host, out_size, "%s", host);
	curl_free(host);
	curl_url_cleanup(u);
	return ok;
}

/* Append CURLOPT_RESOLVE entries for a hostname. Returns the new list
 * head. In production, ip[4] is resolved via net_gethostbyname. */
static struct curl_slist *wbl_resolve_append_host(struct curl_slist *list,
                                                  const char *host,
                                                  const unsigned char ip[4])
{
	/* Add for ports 80 and 443 so http→https redirects work. */
	static const unsigned short ports[] = {80, 443, 0};
	for (int i = 0; ports[i] != 0; i++) {
		char entry[256];
		snprintf(entry, sizeof(entry), "%s:%u:%u.%u.%u.%u",
		         host, (unsigned)ports[i], ip[0], ip[1], ip[2], ip[3]);
		list = curl_slist_append(list, entry);
	}
	return list;
}

/* Helper: check if the resolve list contains an entry for host:port. */
static int has_resolve_entry(struct curl_slist *list, const char *host, unsigned port)
{
	char prefix[256];
	snprintf(prefix, sizeof(prefix), "%s:%u:", host, port);
	for (struct curl_slist *p = list; p; p = p->next) {
		if (strstr(p->data, prefix) == p->data)
			return 1;
	}
	return 0;
}

/* ---------------------------------------------------------------------
 * Test: basic debug-line parsing
 * --------------------------------------------------------------------- */
static void test_extract_redirect_hostname(void)
{
	char host[256];
	const char *msg1 = "Issue another request to this URL: 'https://www.x.com/'";
	assert(wbl_resolve_extract_host_from_debug(msg1, strlen(msg1), host, sizeof(host)));
	assert(strcmp(host, "www.x.com") == 0);

	const char *msg2 = "Issue another request to this URL: 'http://m.wikipedia.org/wiki/Main'";
	assert(wbl_resolve_extract_host_from_debug(msg2, strlen(msg2), host, sizeof(host)));
	assert(strcmp(host, "m.wikipedia.org") == 0);

	const char *msg3 = "Issue another request to this URL: 'https://ssl.x.com:8443/'";
	assert(wbl_resolve_extract_host_from_debug(msg3, strlen(msg3), host, sizeof(host)));
	assert(strcmp(host, "ssl.x.com") == 0);

	/* Negative: non-redirect debug messages should not match. */
	const char *msg4 = "SSL connected";
	assert(!wbl_resolve_extract_host_from_debug(msg4, strlen(msg4), host, sizeof(host)));

	const char *msg5 = "Connected to wikipedia.org (103.102.166.224) port 443";
	assert(!wbl_resolve_extract_host_from_debug(msg5, strlen(msg5), host, sizeof(host)));

	/* Malformed: missing closing quote. */
	const char *msg6 = "Issue another request to this URL: 'https://broken";
	assert(!wbl_resolve_extract_host_from_debug(msg6, strlen(msg6), host, sizeof(host)));
}

/* ---------------------------------------------------------------------
 * Test: full redirect chain simulation
 *
 * Chain: http://x.com → https://x.com → https://www.x.com
 *      → https://ssl.x.com → https://www.x.com/help
 *
 * After walking the chain, our resolve list should contain entries
 * for ALL hostnames encountered (x.com, www.x.com, ssl.x.com).
 * --------------------------------------------------------------------- */
static void test_full_redirect_chain(void)
{
	struct curl_slist *list = NULL;
	const unsigned char ip[4] = {104, 244, 42, 193};  /* x.com's real IP */

	/* Hop 0: initial URL. Seed x.com entries. */
	list = wbl_resolve_append_host(list, "x.com", ip);

	/* Hop 1: http://x.com → https://x.com. Same host, port upgrade.
	 * Already covered by the seed. */
	char host[256];
	const char *debug1 = "Issue another request to this URL: 'https://x.com/'";
	assert(wbl_resolve_extract_host_from_debug(debug1, strlen(debug1), host, sizeof(host)));
	/* Only append if new. */
	if (!has_resolve_entry(list, host, 443))
		list = wbl_resolve_append_host(list, host, ip);

	/* Hop 2: https://x.com → https://www.x.com */
	const char *debug2 = "Issue another request to this URL: 'https://www.x.com/'";
	assert(wbl_resolve_extract_host_from_debug(debug2, strlen(debug2), host, sizeof(host)));
	assert(strcmp(host, "www.x.com") == 0);
	if (!has_resolve_entry(list, host, 443))
		list = wbl_resolve_append_host(list, host, ip);

	/* Hop 3: https://www.x.com → https://ssl.x.com (arbitrary subdomain) */
	const char *debug3 = "Issue another request to this URL: 'https://ssl.x.com/'";
	assert(wbl_resolve_extract_host_from_debug(debug3, strlen(debug3), host, sizeof(host)));
	assert(strcmp(host, "ssl.x.com") == 0);
	if (!has_resolve_entry(list, host, 443))
		list = wbl_resolve_append_host(list, host, ip);

	/* Hop 4: https://ssl.x.com → https://www.x.com/help (back to www, path change).
	 * www.x.com already in list; should not duplicate. */
	const char *debug4 = "Issue another request to this URL: 'https://www.x.com/help'";
	assert(wbl_resolve_extract_host_from_debug(debug4, strlen(debug4), host, sizeof(host)));
	assert(strcmp(host, "www.x.com") == 0);
	int was_there = has_resolve_entry(list, host, 443);
	if (!was_there)
		list = wbl_resolve_append_host(list, host, ip);
	assert(was_there);  /* already had it from hop 2 */

	/* Assertion: all three hostnames are in the resolve list, at both
	 * ports. This is what curl needs to successfully follow the chain. */
	assert(has_resolve_entry(list, "x.com", 80));
	assert(has_resolve_entry(list, "x.com", 443));
	assert(has_resolve_entry(list, "www.x.com", 80));
	assert(has_resolve_entry(list, "www.x.com", 443));
	assert(has_resolve_entry(list, "ssl.x.com", 80));
	assert(has_resolve_entry(list, "ssl.x.com", 443));

	/* And we should NOT have duplicated entries (6 hosts × 2 ports = 6 entries,
	 * since x.com, www.x.com, ssl.x.com × 2 ports each). */
	int count = 0;
	for (struct curl_slist *p = list; p; p = p->next) count++;
	printf("  total resolve entries: %d (expect 6)\n", count);
	assert(count == 6);

	curl_slist_free_all(list);
}

/* ---------------------------------------------------------------------
 * Test: cross-brand redirects (x.com → twitter.com).
 *
 * Even with completely different TLDs, the dynamic extraction should
 * pick up the new hostname. In production, net_gethostbyname() would
 * need to succeed on the new host — this test just validates that we
 * queue the hostname for resolution.
 * --------------------------------------------------------------------- */
static void test_cross_domain_redirect(void)
{
	char host[256];
	const char *msg = "Issue another request to this URL: 'https://twitter.com/login'";
	assert(wbl_resolve_extract_host_from_debug(msg, strlen(msg), host, sizeof(host)));
	assert(strcmp(host, "twitter.com") == 0);
}

/* ---------------------------------------------------------------------
 * Test: protocol-relative Location redirects.
 *
 * Some servers return Location: //www.x.com/path (no scheme). curl
 * resolves this against the request URL to build the new absolute URL.
 * Our debug log will see the resolved absolute URL.
 * --------------------------------------------------------------------- */
static void test_protocol_relative_redirect(void)
{
	char host[256];
	/* curl logs the resolved absolute URL, not the relative Location. */
	const char *msg = "Issue another request to this URL: 'https://www.x.com/path'";
	assert(wbl_resolve_extract_host_from_debug(msg, strlen(msg), host, sizeof(host)));
	assert(strcmp(host, "www.x.com") == 0);
}

/* ---------------------------------------------------------------------
 * Test: malformed debug messages don't crash or match.
 *
 * If curl changes its debug log format, we must fail-safe (return 0)
 * rather than matching garbage. Also: no buffer overflow on very long
 * hostnames.
 * --------------------------------------------------------------------- */
static void test_malformed_debug_input(void)
{
	char host[256];

	/* Empty input. */
	assert(!wbl_resolve_extract_host_from_debug("", 0, host, sizeof(host)));

	/* Marker present but no URL. */
	const char *m1 = "Issue another request to this URL: ";
	assert(!wbl_resolve_extract_host_from_debug(m1, strlen(m1), host, sizeof(host)));

	/* Empty URL between quotes. */
	const char *m2 = "Issue another request to this URL: ''";
	assert(!wbl_resolve_extract_host_from_debug(m2, strlen(m2), host, sizeof(host)));

	/* Unterminated quote. */
	const char *m3 = "Issue another request to this URL: 'https://";
	assert(!wbl_resolve_extract_host_from_debug(m3, strlen(m3), host, sizeof(host)));

	/* URL with no scheme (curl_url_set rejects). */
	const char *m4 = "Issue another request to this URL: 'just text'";
	assert(!wbl_resolve_extract_host_from_debug(m4, strlen(m4), host, sizeof(host)));

	/* Partial marker (prefix match must be exact). */
	const char *m5 = "Issue another";
	assert(!wbl_resolve_extract_host_from_debug(m5, strlen(m5), host, sizeof(host)));

	/* Very long hostname — should not overflow out_host. */
	char bigurl[1024];
	strcpy(bigurl, "Issue another request to this URL: 'https://");
	for (int i = 0; i < 400; i++) strcat(bigurl, "a");
	strcat(bigurl, ".x.com/'");
	char small_host[32];
	/* With small buffer, extraction should fail-safe (return 0). */
	assert(!wbl_resolve_extract_host_from_debug(bigurl, strlen(bigurl), small_host, sizeof(small_host)));
}

/* ---------------------------------------------------------------------
 * Test: static www/bare fix only — documents the gap.
 *
 * This test shows that our CURRENT (static) fix does NOT handle the
 * ssl.x.com case, justifying the dynamic hook.
 * --------------------------------------------------------------------- */
static struct curl_slist *wbl_build_resolve_list_static(const char *url,
                                                        const unsigned char ip[4])
{
	CURLU *u = curl_url();
	if (!u) return NULL;
	if (curl_url_set(u, CURLUPART_URL, url, 0) != CURLUE_OK) {
		curl_url_cleanup(u);
		return NULL;
	}
	char *host = NULL;
	curl_url_get(u, CURLUPART_HOST, &host, 0);
	struct curl_slist *list = NULL;
	if (host) {
		list = wbl_resolve_append_host(list, host, ip);
		/* Static www/bare fix. */
		if (strncmp(host, "www.", 4) == 0)
			list = wbl_resolve_append_host(list, host + 4, ip);
		else {
			char www_host[256];
			snprintf(www_host, sizeof(www_host), "www.%s", host);
			list = wbl_resolve_append_host(list, www_host, ip);
		}
		curl_free(host);
	}
	curl_url_cleanup(u);
	return list;
}

static void test_static_fix_gap(void)
{
	const unsigned char ip[4] = {104, 244, 42, 193};
	struct curl_slist *list = wbl_build_resolve_list_static("https://x.com/", ip);

	/* Static fix covers these: */
	assert(has_resolve_entry(list, "x.com", 443));
	assert(has_resolve_entry(list, "www.x.com", 443));

	/* Static fix does NOT cover these — documenting the gap: */
	assert(!has_resolve_entry(list, "ssl.x.com", 443));
	assert(!has_resolve_entry(list, "mobile.x.com", 443));
	assert(!has_resolve_entry(list, "m.x.com", 443));

	printf("  static fix covers: x.com, www.x.com (good)\n");
	printf("  static fix MISSING: ssl.x.com, mobile.x.com, m.x.com (dynamic hook needed)\n");

	curl_slist_free_all(list);
}

int main(void)
{
	printf("redirect_chain_test: multi-hop redirect handling\n\n");

	printf("Test 1: extract hostname from curl debug log\n");
	test_extract_redirect_hostname();
	printf("  PASS\n\n");

	printf("Test 2: full chain (x.com → www.x.com → ssl.x.com → www.x.com/help)\n");
	test_full_redirect_chain();
	printf("  PASS\n\n");

	printf("Test 3: cross-domain redirect (x.com → twitter.com)\n");
	test_cross_domain_redirect();
	printf("  PASS\n\n");

	printf("Test 4: protocol-relative Location header resolution\n");
	test_protocol_relative_redirect();
	printf("  PASS\n\n");

	printf("Test 5: malformed debug input — fail-safe\n");
	test_malformed_debug_input();
	printf("  PASS\n\n");

	printf("Test 6: static fix gap — documents why dynamic hook is needed\n");
	test_static_fix_gap();
	printf("  PASS\n\n");

	printf("All tests passed. Redirect chain handling validated.\n");
	return 0;
}
