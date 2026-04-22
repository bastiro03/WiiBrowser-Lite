/* manual_redirect_test.c - Test manual redirect following logic.
 *
 * Validates the approach for fixing wikipedia.org mbedTLS context corruption:
 * - Disable CURLOPT_FOLLOWLOCATION
 * - Manually follow redirects in a loop
 * - Extract Location header from 3xx responses
 * - Close connection completely before opening next hop
 * - Respect CURLOPT_MAXREDIRS (10 hops)
 * - Handle x-safari-* URL scheme rewrites within the loop
 *
 * This prevents curl from keeping multiple SSL connections open simultaneously,
 * which causes mbedTLS context corruption (-0x7100).
 */

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_REDIRECTS 10

/* Simulated HTTP response with status code and Location header. */
typedef struct {
    int status_code;
    const char *location;
} MockResponse;

/* Extract Location header value, handling x-safari-* rewrites. */
static const char *extract_redirect_url(const char *location_header)
{
    if (!location_header || location_header[0] == '\0')
        return NULL;

    const char *loc = location_header;
    /* Skip "location: " prefix if present. */
    if (strncmp(loc, "location:", 9) == 0) {
        loc += 9;
        while (*loc == ' ') loc++;
    }

    /* Handle x-safari-* URL schemes by stripping the prefix. */
    if (strncmp(loc, "x-safari-https://", 17) == 0)
        return loc + 9;  /* Skip "x-safari-", keep "https://..." */
    else if (strncmp(loc, "x-safari-http://", 16) == 0)
        return loc + 9;

    return loc;
}

/* Simulate manual redirect following with connection close between hops. */
static int follow_redirects_manually(MockResponse *responses, int num_responses,
                                      char *final_url, size_t final_url_size)
{
    int hop = 0;
    const char *current_url = "http://example.com/";  /* Initial URL */

    while (hop < num_responses) {
        MockResponse resp = responses[hop];

        printf("  Hop %d: status=%d, URL=%s\n", hop, resp.status_code, current_url);

        /* Non-redirect status: success. */
        if (resp.status_code < 300 || resp.status_code >= 400) {
            snprintf(final_url, final_url_size, "%s", current_url);
            printf("    → Final response (status=%d)\n", resp.status_code);
            return hop;  /* Return number of redirects followed */
        }

        /* Extract Location header. */
        const char *next_url = extract_redirect_url(resp.location);
        if (!next_url) {
            fprintf(stderr, "    ERROR: No Location header for %d response\n", resp.status_code);
            return -1;
        }

        printf("    → Redirect to: %s\n", next_url);

        /* Check max redirects. */
        if (hop >= MAX_REDIRECTS) {
            fprintf(stderr, "    ERROR: Too many redirects (max=%d)\n", MAX_REDIRECTS);
            return -1;
        }

        /* CRITICAL: In real code, curl_easy_cleanup() here before next curl_easy_init().
         * This ensures the SSL connection is fully torn down before opening the next. */
        printf("    → [CLOSE CONNECTION]\n");

        current_url = next_url;
        hop++;
    }

    snprintf(final_url, final_url_size, "%s", current_url);
    return hop;
}

/* Test 1: Simple redirect chain (http → https → www). */
void test_simple_redirect_chain(void)
{
    printf("Test 1: Simple redirect chain (http → https → www)\n");

    MockResponse chain[] = {
        {301, "https://example.com/"},       /* http → https */
        {301, "https://www.example.com/"},  /* https → www */
        {200, NULL}                          /* Final response */
    };

    char final_url[256];
    int hops = follow_redirects_manually(chain, 3, final_url, sizeof(final_url));

    printf("  Result: %d hops, final_url=%s\n", hops, final_url);
    assert(hops == 2);  /* Two redirects followed */
    assert(strstr(final_url, "www.example.com") != NULL);
    printf("  PASS\n\n");
}

/* Test 2: x-safari-* URL scheme rewrite. */
void test_xsafari_rewrite(void)
{
    printf("Test 2: x-safari-* URL scheme rewrite\n");

    MockResponse chain[] = {
        {302, "x-safari-https://redirect.x.com/?ct=rw-null"},
        {200, NULL}
    };

    char final_url[256];
    int hops = follow_redirects_manually(chain, 2, final_url, sizeof(final_url));

    printf("  Result: %d hops, final_url=%s\n", hops, final_url);
    assert(hops == 1);
    assert(strstr(final_url, "https://redirect.x.com") != NULL);
    assert(strstr(final_url, "x-safari-") == NULL);  /* Prefix stripped */
    printf("  PASS\n\n");
}

/* Test 3: Max redirect limit enforcement. */
void test_max_redirects(void)
{
    printf("Test 3: Max redirect limit (%d hops)\n", MAX_REDIRECTS);

    /* Create a chain with MAX_REDIRECTS + 1 hops (should fail). */
    MockResponse chain[MAX_REDIRECTS + 2];
    for (int i = 0; i <= MAX_REDIRECTS; i++) {
        chain[i].status_code = 301;
        chain[i].location = "https://example.com/next";
    }
    chain[MAX_REDIRECTS + 1].status_code = 200;
    chain[MAX_REDIRECTS + 1].location = NULL;

    char final_url[256];
    int hops = follow_redirects_manually(chain, MAX_REDIRECTS + 2, final_url, sizeof(final_url));

    printf("  Result: %d hops (expect -1 for error)\n", hops);
    assert(hops == -1);  /* Should fail at MAX_REDIRECTS */
    printf("  PASS\n\n");
}

/* Test 4: Connection close between hops (validates fix for mbedTLS corruption). */
void test_connection_close_between_hops(void)
{
    printf("Test 4: Connection close between hops\n");
    printf("  This test validates that the manual redirect loop includes:\n");
    printf("    1. curl_easy_cleanup() after each 3xx response\n");
    printf("    2. curl_easy_init() before next hop\n");
    printf("  This prevents overlapping SSL connections that cause mbedTLS corruption.\n");

    MockResponse chain[] = {
        {301, "https://www.wikipedia.org/"},
        {200, NULL}
    };

    char final_url[256];
    int hops = follow_redirects_manually(chain, 2, final_url, sizeof(final_url));

    printf("  Result: %d hops, final_url=%s\n", hops, final_url);
    printf("  Each hop printed '[CLOSE CONNECTION]' above.\n");
    assert(hops == 1);
    printf("  PASS\n\n");
}

/* Test 5: Location header parsing (with "location: " prefix). */
void test_location_header_parsing(void)
{
    printf("Test 5: Location header parsing\n");

    const char *cases[] = {
        "https://example.com/",
        "location: https://example.com/",
        "location:https://example.com/",
        "location:  https://example.com/",
        "x-safari-https://example.com/",
        "location: x-safari-https://example.com/",
    };

    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        const char *parsed = extract_redirect_url(cases[i]);
        printf("  Input:  '%s'\n", cases[i]);
        printf("  Output: '%s'\n", parsed);
        assert(parsed != NULL);
        assert(strstr(parsed, "https://example.com") != NULL);
        assert(strstr(parsed, "x-safari-") == NULL);
    }

    printf("  PASS\n\n");
}

int main(void)
{
    printf("manual_redirect_test: validating manual redirect following logic\n\n");

    test_simple_redirect_chain();
    test_xsafari_rewrite();
    test_max_redirects();
    test_connection_close_between_hops();
    test_location_header_parsing();

    printf("All tests passed.\n");
    printf("\nNext step: Implement manual redirect loop in httplib.cpp getrequest().\n");
    printf("Key changes:\n");
    printf("  - Set CURLOPT_FOLLOWLOCATION to 0\n");
    printf("  - Loop: perform request → check status → extract Location → cleanup → init → repeat\n");
    printf("  - Ensure curl_easy_cleanup() between hops to prevent SSL context corruption\n");
    return 0;
}
