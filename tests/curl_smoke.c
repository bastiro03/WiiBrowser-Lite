/* libcurl + mbedTLS smoke test.
 *
 * Validates the full HTTPS pipeline we'll ship on Wii:
 *   libcurl 8.16 -> mbedTLS 3.6 -> kernel sockets
 *
 * Build: via tests/Makefile (uses the host-built libcurl.a from
 * third_party/curl/lib/.libs/ and our mbedtls .a files). */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <curl/curl.h>

/* Write callback: discard body, just count bytes */
static size_t drain(void *ptr, size_t size, size_t nmemb, void *userp) {
    size_t *total = (size_t *)userp;
    *total += size * nmemb;
    return size * nmemb;
}

int main(void) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL *c = curl_easy_init();
    if (!c) { fprintf(stderr, "curl_easy_init failed\n"); return 1; }

    size_t total = 0;
    long status = 0;
    char ct[128] = {0}, *ct_ptr = ct;

    /* Use a site that returns a predictable 200 for any UA. */
    curl_easy_setopt(c, CURLOPT_URL, "https://example.com/");
    curl_easy_setopt(c, CURLOPT_WRITEFUNCTION, drain);
    curl_easy_setopt(c, CURLOPT_WRITEDATA, &total);
    curl_easy_setopt(c, CURLOPT_CAINFO, "/etc/ssl/certs/ca-certificates.crt");
    curl_easy_setopt(c, CURLOPT_USERAGENT,
                     "Mozilla/5.0 (iPhone; CPU iPhone OS 6_1_6 like Mac OS X) "
                     "AppleWebKit/536.26 (KHTML, like Gecko) Mobile/10B500");
    curl_easy_setopt(c, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(c, CURLOPT_CONNECTTIMEOUT, 10L);
    curl_easy_setopt(c, CURLOPT_TIMEOUT, 30L);

    CURLcode rc = curl_easy_perform(c);
    if (rc != CURLE_OK) {
        fprintf(stderr, "[FAIL] curl: %s\n", curl_easy_strerror(rc));
        return 1;
    }

    curl_easy_getinfo(c, CURLINFO_RESPONSE_CODE, &status);
    curl_easy_getinfo(c, CURLINFO_CONTENT_TYPE, &ct_ptr);

    printf("[OK] HTTPS GET https://example.com/\n");
    printf("     HTTP %ld, %zu bytes, type=%s\n",
           status, total, ct_ptr ? ct_ptr : "(none)");

    if (status != 200) return 1;
    if (total == 0) { fprintf(stderr, "[FAIL] empty body\n"); return 1; }

    curl_easy_cleanup(c);
    curl_global_cleanup();
    return 0;
}
