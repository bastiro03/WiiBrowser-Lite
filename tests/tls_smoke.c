/* Smoke test: can our minimal mbedTLS config actually talk to wikipedia.org?
 *
 * Build:
 *   gcc -Ithird_party/mbedtls/include -Ithird_party/mbedtls-wbl-config \
 *       -DMBEDTLS_CONFIG_FILE='"wbl/mbedtls_config.h"' -DWBL_PLATFORM_WII \
 *       /tmp/wiki_tls_test.c \
 *       third_party/mbedtls/library/libmbedtls.a \
 *       third_party/mbedtls/library/libmbedx509.a \
 *       third_party/mbedtls/library/libmbedcrypto.a \
 *       -o /tmp/wiki_tls_test
 */

#include <stdio.h>
#include <string.h>
#include "mbedtls/net_sockets.h"
#include "mbedtls/ssl.h"
#include "mbedtls/entropy.h"
#include "mbedtls/hmac_drbg.h"
#include "mbedtls/x509_crt.h"
#include "mbedtls/error.h"

#define HOST "api.github.com"
#define PORT "443"

int main(void) {
    int ret;
    mbedtls_net_context server;
    mbedtls_entropy_context entropy;
    mbedtls_hmac_drbg_context drbg;
    mbedtls_ssl_context ssl;
    mbedtls_ssl_config cfg;
    mbedtls_x509_crt cacert;
    char errbuf[256];

    mbedtls_net_init(&server);
    mbedtls_ssl_init(&ssl);
    mbedtls_ssl_config_init(&cfg);
    mbedtls_x509_crt_init(&cacert);
    mbedtls_entropy_init(&entropy);
    mbedtls_hmac_drbg_init(&drbg);

    /* 1. Seed DRBG from system entropy */
    const char *seed = "wbl-tls-test";
    ret = mbedtls_hmac_drbg_seed(&drbg,
        mbedtls_md_info_from_type(MBEDTLS_MD_SHA256),
        mbedtls_entropy_func, &entropy,
        (const unsigned char *)seed, strlen(seed));
    if (ret) {
        mbedtls_strerror(ret, errbuf, sizeof(errbuf));
        printf("seed failed: -0x%04x %s\n", (unsigned)-ret, errbuf);
        return 1;
    }
    printf("[OK] DRBG seeded\n");

    /* 2. Load system CA certs */
    ret = mbedtls_x509_crt_parse_file(&cacert, "/etc/ssl/certs/ca-certificates.crt");
    if (ret < 0) {
        printf("CA parse returned %d\n", ret);
        return 1;
    }
    printf("[OK] Loaded %d CA certs (skipped %d bad)\n",
           0, ret);  /* parse_file returns # skipped */

    /* 3. TCP connect */
    ret = mbedtls_net_connect(&server, HOST, PORT, MBEDTLS_NET_PROTO_TCP);
    if (ret) {
        mbedtls_strerror(ret, errbuf, sizeof(errbuf));
        printf("connect failed: %s\n", errbuf);
        return 1;
    }
    printf("[OK] TCP connected to %s:%s\n", HOST, PORT);

    /* 4. Configure TLS client */
    ret = mbedtls_ssl_config_defaults(&cfg,
        MBEDTLS_SSL_IS_CLIENT,
        MBEDTLS_SSL_TRANSPORT_STREAM,
        MBEDTLS_SSL_PRESET_DEFAULT);
    if (ret) { printf("config failed: -0x%04x\n", (unsigned)-ret); return 1; }

    mbedtls_ssl_conf_authmode(&cfg, MBEDTLS_SSL_VERIFY_REQUIRED);
    mbedtls_ssl_conf_ca_chain(&cfg, &cacert, NULL);
    mbedtls_ssl_conf_rng(&cfg, mbedtls_hmac_drbg_random, &drbg);

    ret = mbedtls_ssl_setup(&ssl, &cfg);
    if (ret) { printf("setup failed: -0x%04x\n", (unsigned)-ret); return 1; }

    ret = mbedtls_ssl_set_hostname(&ssl, HOST);
    if (ret) { printf("hostname failed\n"); return 1; }

    mbedtls_ssl_set_bio(&ssl, &server, mbedtls_net_send, mbedtls_net_recv, NULL);

    /* 5. Handshake */
    while ((ret = mbedtls_ssl_handshake(&ssl)) != 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            mbedtls_strerror(ret, errbuf, sizeof(errbuf));
            printf("handshake failed: -0x%04x %s\n", (unsigned)-ret, errbuf);
            return 1;
        }
    }
    printf("[OK] TLS handshake complete\n");
    printf("[OK] Protocol: %s\n", mbedtls_ssl_get_version(&ssl));
    printf("[OK] Cipher:   %s\n", mbedtls_ssl_get_ciphersuite(&ssl));

    /* 6. Verify cert */
    uint32_t flags = mbedtls_ssl_get_verify_result(&ssl);
    if (flags) {
        char vbuf[512];
        mbedtls_x509_crt_verify_info(vbuf, sizeof(vbuf), "  ! ", flags);
        printf("cert verify flags: 0x%x\n%s", flags, vbuf);
        return 1;
    }
    printf("[OK] Certificate verified\n");

    /* 7. Send GET */
    const char *req = "GET /zen HTTP/1.1\r\n"
                      "Host: " HOST "\r\n"
                      "User-Agent: WiiBrowser-Lite/2.0 (Nintendo Wii; https://github.com/matthargett/WiiBrowser-Lite)\r\n"
                      "Connection: close\r\n\r\n";
    while ((ret = mbedtls_ssl_write(&ssl, (const unsigned char *)req, strlen(req))) <= 0) {
        if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
            printf("write failed\n"); return 1;
        }
    }
    printf("[OK] Sent GET /zen\n");

    /* 8. Read response (first few KB) */
    unsigned char buf[4096];
    int total = 0;
    while (total < 4096) {
        ret = mbedtls_ssl_read(&ssl, buf, sizeof(buf) - 1);
        if (ret == MBEDTLS_ERR_SSL_WANT_READ || ret == MBEDTLS_ERR_SSL_WANT_WRITE) continue;
        if (ret == MBEDTLS_ERR_SSL_PEER_CLOSE_NOTIFY) break;
        if (ret <= 0) break;
        buf[ret] = 0;
        total += ret;
    }
    printf("[OK] Received %d bytes. First line: ", total);
    /* Print the HTTP status line */
    char *nl = strchr((char *)buf, '\n');
    if (nl) { *nl = 0; printf("%s\n", buf); }

    mbedtls_ssl_close_notify(&ssl);
    mbedtls_net_free(&server);
    mbedtls_ssl_free(&ssl);
    mbedtls_ssl_config_free(&cfg);
    mbedtls_x509_crt_free(&cacert);
    mbedtls_hmac_drbg_free(&drbg);
    mbedtls_entropy_free(&entropy);
    return 0;
}
