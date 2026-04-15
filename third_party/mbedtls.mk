# mbedTLS build configuration for WiiBrowser-Lite
#
# Builds mbedTLS 3.6 LTS with a minimal config suitable for HTTPS browsing
# of modern sites (Wikipedia, GitHub).
#
# Includes only TLS 1.2 + 1.3 cipher suites needed in 2026:
#   - ECDHE-ECDSA / ECDHE-RSA + AES-GCM
#   - X25519 / secp256r1 / secp384r1
#   - SHA-256 / SHA-384

MBEDTLS_DIR := third_party/mbedtls

# Use our trimmed config instead of mbedTLS default (saves ~150KB)
MBEDTLS_CFLAGS := \
    -DMBEDTLS_CONFIG_FILE='"wbl/mbedtls_config.h"' \
    -I$(MBEDTLS_DIR)/include \
    -Iinclude \
    -Os -ffunction-sections -fdata-sections

# Build mbedTLS via its CMake/make system, in tree to avoid PIC issues
MBEDTLS_LIB := $(MBEDTLS_DIR)/library/libmbedtls.a
MBEDX509_LIB := $(MBEDTLS_DIR)/library/libmbedx509.a
MBEDCRYPTO_LIB := $(MBEDTLS_DIR)/library/libmbedcrypto.a

# We invoke the upstream Makefile but with our config + cross-compile vars
$(MBEDTLS_LIB) $(MBEDX509_LIB) $(MBEDCRYPTO_LIB):
	$(MAKE) -C $(MBEDTLS_DIR)/library \
	    CC="$(CC)" AR="$(AR)" \
	    CFLAGS="$(CFLAGS) $(MBEDTLS_CFLAGS)" \
	    no_test=1 lib

.PHONY: mbedtls-clean
mbedtls-clean:
	$(MAKE) -C $(MBEDTLS_DIR) clean
