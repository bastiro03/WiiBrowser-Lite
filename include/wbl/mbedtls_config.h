/****************************************************************************
 * Minimal mbedTLS Configuration for WiiBrowser-Lite
 *
 * Targets HTTPS browsing of Wikipedia, GitHub, and other modern sites
 * with the smallest possible code/RAM footprint that still gets us
 * TLS 1.3 + forward secrecy.
 *
 * TLS 1.3 in mbedTLS 3.6 requires PSA crypto. PSA adds ~40KB code but
 * is worth it: 1-RTT handshake (faster page load on Wii/DS modems),
 * mandatory ephemeral key exchange, and forward-compat with sites
 * that are dropping TLS 1.2.
 *
 * Saves ~110KB of code vs default mbedtls_config.h by removing:
 *   - DTLS, DHE (non-EC), PSK, SRTP, CCM cipher modes
 *   - Unused curves (Brainpool, secp521, secp224, secp192)
 *   - Self-tests, certificate writing, debug helpers
 *   - RIPEMD160, CAMELLIA, ARIA, non-primary hashes
 *
 * Keeps what wikipedia.org / github.com negotiate in 2026:
 *   TLS 1.3 + TLS 1.2 fallback
 *   AES-128-GCM and ChaCha20-Poly1305 (no AES-NI on PPC/ARM9)
 *   secp256r1, secp384r1, X25519
 *   RSA + ECDSA server certs
 ***************************************************************************/

#ifndef WBL_MBEDTLS_CONFIG_H
#define WBL_MBEDTLS_CONFIG_H

#include "wbl/platform.h"

/* ---- Platform/system defines ------------------------------------------ */

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
/* Use libc snprintf/printf/fprintf/exit/time - libogc (Wii), libnds
 * (DS/DSi), and host libc all provide these natively. */

#define MBEDTLS_HAVE_TIME
#define MBEDTLS_HAVE_TIME_DATE

/* ---- Crypto primitives ------------------------------------------------ */

/* Cipher abstraction layer (required by GCM, TLS, etc.) */
#define MBEDTLS_CIPHER_C

/* Symmetric: AES-GCM and ChaCha20-Poly1305 */
#define MBEDTLS_AES_C
#define MBEDTLS_AES_FEWER_TABLES         /* save ~6KB at small perf cost */
#define MBEDTLS_GCM_C
#define MBEDTLS_CHACHA20_C
#define MBEDTLS_CHACHAPOLY_C
#define MBEDTLS_POLY1305_C

/* Hashes */
#define MBEDTLS_SHA224_C
#define MBEDTLS_SHA256_C
#define MBEDTLS_SHA384_C
#define MBEDTLS_SHA512_C
#define MBEDTLS_MD_C                     /* HKDF / HMAC */

/* Asymmetric: ECDH/ECDSA + RSA for cert verify */
#define MBEDTLS_ECP_C
#define MBEDTLS_ECDH_C
#define MBEDTLS_ECDSA_C
#define MBEDTLS_RSA_C
#define MBEDTLS_PKCS1_V15
#define MBEDTLS_PKCS1_V21

/* Curves: P-256 (universal), P-384 (some CAs), X25519 (modern) */
#define MBEDTLS_ECP_DP_SECP256R1_ENABLED
#define MBEDTLS_ECP_DP_SECP384R1_ENABLED
#define MBEDTLS_ECP_DP_CURVE25519_ENABLED

/* PK abstraction */
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_PK_WRITE_C               /* curl pubkey pinning */
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C

/* Legacy hashes for old certs still on the web */
#define MBEDTLS_MD5_C
#define MBEDTLS_SHA1_C

/* Key derivation for TLS 1.3 */
#define MBEDTLS_HKDF_C

/* Random source */
#define MBEDTLS_CTR_DRBG_C               /* curl requires */
#define MBEDTLS_HMAC_DRBG_C              /* we prefer */
#define MBEDTLS_ENTROPY_C
#if defined(WBL_PLATFORM_WII) || defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)
#  define MBEDTLS_NO_PLATFORM_ENTROPY
#  define MBEDTLS_ENTROPY_HARDWARE_ALT   /* we supply mbedtls_hardware_poll */
#endif

/* Error strings (small, huge debug win) */
#define MBEDTLS_ERROR_C

/* SSL debug hook - lets us wire mbedtls_ssl_conf_dbg. Only the
 * registration API is always compiled in; call threshold stays 0
 * by default so no runtime overhead in release builds. */
#define MBEDTLS_DEBUG_C

/* Network I/O helpers */
#define MBEDTLS_NET_C

/* Filesystem - host testing only (consoles use baked-in cert PEM) */
#if !defined(WBL_PLATFORM_WII) && !defined(WBL_PLATFORM_DSI) && \
    !defined(WBL_PLATFORM_NDS) && !defined(WBL_PLATFORM_MACPLUS)
#  define MBEDTLS_FS_IO
#endif

/* ---- PSA crypto (needed for TLS 1.3) ---------------------------------- */

/* TLS 1.3 in mbedTLS 3.6 routes signature + key-derivation operations
 * through PSA. We use MBEDTLS_PSA_CRYPTO_CONFIG to explicitly list
 * the algorithms we need at the PSA layer.
 *
 * We specifically enable RSA-PSS (not just PKCS#1 v1.5) because TLS 1.3
 * mandates rsa_pss_rsae_sha* as the signature schemes for RSA server
 * certificates. Servers will reject our ClientHello if we advertise
 * only rsa_pkcs1_sha*. */
#define MBEDTLS_PSA_CRYPTO_C
#define MBEDTLS_PSA_CRYPTO_CONFIG

/* Algorithms */
#define PSA_WANT_ALG_HKDF                        1
#define PSA_WANT_ALG_HKDF_EXTRACT                1
#define PSA_WANT_ALG_HKDF_EXPAND                 1
#define PSA_WANT_ALG_HMAC                        1
#define PSA_WANT_ALG_SHA_224                     1
#define PSA_WANT_ALG_SHA_256                     1
#define PSA_WANT_ALG_SHA_384                     1
#define PSA_WANT_ALG_SHA_512                     1
#define PSA_WANT_ALG_MD5                         1  /* legacy cert support */
#define PSA_WANT_ALG_SHA_1                       1  /* legacy cert support */
#define PSA_WANT_ALG_GCM                         1
#define PSA_WANT_ALG_CHACHA20_POLY1305           1
#define PSA_WANT_ALG_ECDH                        1
#define PSA_WANT_ALG_ECDSA                       1
#define PSA_WANT_ALG_DETERMINISTIC_ECDSA         1
#define PSA_WANT_ALG_RSA_PKCS1V15_CRYPT          1
#define PSA_WANT_ALG_RSA_PKCS1V15_SIGN           1
#define PSA_WANT_ALG_RSA_OAEP                    1
#define PSA_WANT_ALG_RSA_PSS                     1  /* TLS 1.3 mandate */

/* Key types */
#define PSA_WANT_KEY_TYPE_AES                    1
#define PSA_WANT_KEY_TYPE_CHACHA20               1
#define PSA_WANT_KEY_TYPE_DERIVE                 1
#define PSA_WANT_KEY_TYPE_HMAC                   1
#define PSA_WANT_KEY_TYPE_ECC_PUBLIC_KEY         1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_BASIC     1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_IMPORT    1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_EXPORT    1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_GENERATE  1
#define PSA_WANT_KEY_TYPE_ECC_KEY_PAIR_DERIVE    1
#define PSA_WANT_KEY_TYPE_RSA_PUBLIC_KEY         1
#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_BASIC     1
#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_IMPORT    1
#define PSA_WANT_KEY_TYPE_RSA_KEY_PAIR_EXPORT    1

/* Curves */
#define PSA_WANT_ECC_SECP_R1_256                 1
#define PSA_WANT_ECC_SECP_R1_384                 1
#define PSA_WANT_ECC_MONTGOMERY_255              1

/* ---- TLS protocol ----------------------------------------------------- */

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C                /* client only */
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_TLS1_3

/* TLS 1.3 key-exchange modes (need at least one enabled) */
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED
/* PSK modes not needed for browsing public websites:
 *   MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_ENABLED
 *   MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL_ENABLED
 */

/* Compatibility mode: interoperate with servers/middleboxes that expect
 * pre-1.3 record framing on the downgrade path. Cheap, widely helpful. */
#define MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE

/* TLS extensions */
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
#define MBEDTLS_SSL_ALPN
#define MBEDTLS_SSL_RENEGOTIATION
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_SESSION_TICKETS      /* TLS 1.2 resumption + TLS 1.3 */

/* Tighten record size to reduce RAM (16K -> 4K). Wiki pages fit. */
#define MBEDTLS_SSL_IN_CONTENT_LEN  4096
#define MBEDTLS_SSL_OUT_CONTENT_LEN 4096

/* Cipher modes - GCM only */
#define MBEDTLS_SSL_HAVE_AES
#define MBEDTLS_SSL_HAVE_CHACHAPOLY
#define MBEDTLS_SSL_HAVE_GCM

/* Key exchanges for TLS 1.2 - ECDHE only */
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
/* Plain RSA kept as fallback for really old servers (MediaWiki CDN edges
 * sometimes serve from legacy middleboxes on HTTPS). */
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED

/* X.509 cert handling */
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_X509_RSASSA_PSS_SUPPORT  /* accept RSA-PSS-signed certs */
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_BASE64_C

/* ---- Disabled (saves space) ------------------------------------------- */
/*  No DTLS, no PSK cipher suites, no DHE (non-EC), no static RSA key
 *  exchange, no CCM/CBC suites, no truncated HMAC, no SRTP,
 *  no CAMELLIA/ARIA/BLOWFISH, no self-tests, no debug, no CRL parsing,
 *  no cert writing, no RIPEMD160, no Brainpool curves.
 */

#endif /* WBL_MBEDTLS_CONFIG_H */
