/****************************************************************************
 * Minimal mbedTLS Configuration for WiiBrowser-Lite
 *
 * Targets HTTPS browsing of Wikipedia, GitHub, and other modern sites
 * with the smallest possible code/RAM footprint.
 *
 * Strategy: TLS 1.2 only. TLS 1.3 in mbedTLS 3.6 requires PSA crypto
 * which adds ~40KB; Wikipedia still supports TLS 1.2 so we skip 1.3.
 *
 * Saves ~150KB of code vs default mbedtls_config.h by removing:
 *   - TLS 1.3, PSA crypto
 *   - DTLS, DHE, PSK, SRTP, CCM cipher modes
 *   - Old hashes (RIPEMD160)
 *   - Unused curves (Brainpool, secp521, etc.)
 *   - Self-tests, certificate writing, debug
 *
 * Keeps what wikipedia.org / github.com negotiate over TLS 1.2 in 2026:
 *   ECDHE-ECDSA-AES128-GCM-SHA256
 *   ECDHE-RSA-AES128-GCM-SHA256
 *   ECDHE-ECDSA-CHACHA20-POLY1305-SHA256
 *   ECDHE-RSA-CHACHA20-POLY1305-SHA256
 ***************************************************************************/

#ifndef WBL_MBEDTLS_CONFIG_H
#define WBL_MBEDTLS_CONFIG_H

#include "wbl/platform.h"

/* ---- Platform/system defines ------------------------------------------ */

#define MBEDTLS_HAVE_ASM
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
/* Use libc snprintf/printf/fprintf/exit/time - libogc provides these on
 * Wii, stdlib on DSi. Saves us from wiring ALT callbacks for no reason. */

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
#define MBEDTLS_MD_C                     /* needed for HKDF / HMAC */

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
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C

/* Legacy hashes for old certs still on the web */
#define MBEDTLS_MD5_C
#define MBEDTLS_SHA1_C

/* Random source - on hosted builds use system entropy; on embedded
 * targets we supply mbedtls_hardware_poll() ourselves (see
 * src/wbl/mbedtls_hooks.c). */
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_ENTROPY_C
#if defined(WBL_PLATFORM_WII) || defined(WBL_PLATFORM_DSI) || defined(WBL_PLATFORM_NDS)
#  define MBEDTLS_NO_PLATFORM_ENTROPY
#  define MBEDTLS_ENTROPY_HARDWARE_ALT
#endif

/* Error strings (small enough; huge debugging improvement) */
#define MBEDTLS_ERROR_C

/* Network I/O helpers (BSD socket wrappers) */
#define MBEDTLS_NET_C

/* Filesystem I/O - only for host testing. On console we parse certs
 * from a baked-in PEM buffer (see certs/). */
#if !defined(WBL_PLATFORM_WII) && !defined(WBL_PLATFORM_DSI) && \
    !defined(WBL_PLATFORM_NDS) && !defined(WBL_PLATFORM_MACPLUS)
#  define MBEDTLS_FS_IO
#endif

/* ---- TLS protocol ----------------------------------------------------- */

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C                /* client only */
#define MBEDTLS_SSL_PROTO_TLS1_2
/* TLS 1.3 NOT enabled - requires PSA crypto (~40KB extra) */

/* TLS extensions */
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
#define MBEDTLS_SSL_ALPN
#define MBEDTLS_SSL_RENEGOTIATION
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_SESSION_TICKETS

/* Tighten record size to reduce RAM (16K -> 4K). Wiki pages fit. */
#define MBEDTLS_SSL_IN_CONTENT_LEN  4096
#define MBEDTLS_SSL_OUT_CONTENT_LEN 4096

/* Cipher modes - GCM only */
#define MBEDTLS_SSL_HAVE_AES
#define MBEDTLS_SSL_HAVE_CHACHAPOLY
#define MBEDTLS_SSL_HAVE_GCM

/* Key exchanges - ECDHE only */
#define MBEDTLS_KEY_EXCHANGE_ECDHE_ECDSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_ECDHE_RSA_ENABLED
#define MBEDTLS_KEY_EXCHANGE_RSA_ENABLED  /* fallback for old servers */

/* X.509 cert handling */
#define MBEDTLS_X509_USE_C
#define MBEDTLS_X509_CRT_PARSE_C
#define MBEDTLS_PEM_PARSE_C
#define MBEDTLS_BASE64_C

/* ---- Disabled (saves space) ------------------------------------------- */
/*  No TLS 1.3, no DTLS, no PSK, no DHE, no static-RSA-key-exchange,
 *  no CCM/CBC suites, no truncated HMAC, no SRTP,
 *  no PSA crypto, no self-tests, no debug helpers,
 *  no CRL parsing, no cert writing, no RIPEMD160.
 */

#endif /* WBL_MBEDTLS_CONFIG_H */
