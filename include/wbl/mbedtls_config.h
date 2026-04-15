/****************************************************************************
 * Minimal mbedTLS Configuration for WiiBrowser-Lite
 *
 * Targets HTTPS browsing of Wikipedia, GitHub, and other modern sites
 * with the smallest possible code/RAM footprint.
 *
 * Saves ~150KB of code vs default mbedtls_config.h by removing:
 *   - DTLS, DHE, PSK, SRTP, CCM cipher modes
 *   - Old hashes (MD5, SHA-1 for non-cert use, RIPEMD160)
 *   - Unused curves (Brainpool, secp521, etc.)
 *   - PSA crypto API
 *   - Self-tests, certificate writing, debug
 *
 * Keeps what wikipedia.org / github.com actually negotiate in 2026:
 *   TLS 1.3 (preferred) and TLS 1.2 (fallback)
 *   ECDHE-ECDSA-AES128-GCM-SHA256
 *   ECDHE-RSA-AES128-GCM-SHA256
 *   TLS_AES_128_GCM_SHA256 (TLS 1.3)
 *   TLS_CHACHA20_POLY1305_SHA256 (TLS 1.3, no AES-NI on PowerPC)
 ***************************************************************************/

#ifndef WBL_MBEDTLS_CONFIG_H
#define WBL_MBEDTLS_CONFIG_H

#include "wbl/platform.h"

/* ---- Platform/system defines ------------------------------------------ */

#if defined(WBL_BIG_ENDIAN)
#  define MBEDTLS_HAVE_INT32
#endif

/* No mlock/madvise on game consoles - skip platform memcheck */
#define MBEDTLS_PLATFORM_C
#define MBEDTLS_PLATFORM_MEMORY
#define MBEDTLS_PLATFORM_NO_STD_FUNCTIONS
#define MBEDTLS_PLATFORM_PRINTF_ALT
#define MBEDTLS_PLATFORM_FPRINTF_ALT
#define MBEDTLS_PLATFORM_EXIT_ALT
#define MBEDTLS_PLATFORM_TIME_ALT

/* ---- Crypto primitives ------------------------------------------------ */

/* Symmetric: AES-GCM and ChaCha20-Poly1305 are all wikipedia.org needs */
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
#define MBEDTLS_MD_C                     /* still needed for HKDF */

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
/* NOTE: omitting secp521, brainpool, secp192/224 - saves ~30KB */

/* PK abstraction */
#define MBEDTLS_PK_C
#define MBEDTLS_PK_PARSE_C
#define MBEDTLS_OID_C
#define MBEDTLS_ASN1_PARSE_C
#define MBEDTLS_ASN1_WRITE_C
#define MBEDTLS_BIGNUM_C

/* Key derivation */
#define MBEDTLS_HKDF_C
#define MBEDTLS_MD5_C                    /* legacy cert support */
#define MBEDTLS_SHA1_C                   /* legacy cert support */

/* Random source - HMAC_DRBG smaller than CTR_DRBG */
#define MBEDTLS_HMAC_DRBG_C
#define MBEDTLS_ENTROPY_C
#define MBEDTLS_NO_PLATFORM_ENTROPY      /* we provide our own seed */
#define MBEDTLS_ENTROPY_HARDWARE_ALT

/* ---- TLS protocol ----------------------------------------------------- */

#define MBEDTLS_SSL_TLS_C
#define MBEDTLS_SSL_CLI_C                /* client only - no server */
#define MBEDTLS_SSL_PROTO_TLS1_2
#define MBEDTLS_SSL_PROTO_TLS1_3
#define MBEDTLS_SSL_TLS1_3_COMPATIBILITY_MODE

/* TLS 1.3 features */
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_ENABLED
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_EPHEMERAL_ENABLED
#define MBEDTLS_SSL_TLS1_3_KEY_EXCHANGE_MODE_PSK_EPHEMERAL_ENABLED

/* TLS extensions */
#define MBEDTLS_SSL_SERVER_NAME_INDICATION
#define MBEDTLS_SSL_ALPN
#define MBEDTLS_SSL_RENEGOTIATION
#define MBEDTLS_SSL_KEEP_PEER_CERTIFICATE
#define MBEDTLS_SSL_SESSION_TICKETS

/* Tighten record size to reduce RAM (16K -> 4K). Wiki articles fit. */
#define MBEDTLS_SSL_IN_CONTENT_LEN  4096
#define MBEDTLS_SSL_OUT_CONTENT_LEN 4096

/* Cipher modes - GCM only, no CBC/CCM */
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
/*  No DTLS, no PSK, no DHE, no static-RSA-key-exchange,
 *  no CCM/CBC suites, no truncated HMAC, no SRTP,
 *  no PSA crypto, no self-tests, no debug helpers,
 *  no CRL parsing, no cert writing.
 */

/* ---- Finalize --------------------------------------------------------- */

#include "mbedtls/check_config.h"

#endif /* WBL_MBEDTLS_CONFIG_H */
