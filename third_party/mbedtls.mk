# mbedTLS build configuration for WiiBrowser-Lite
#
# Builds mbedTLS 3.6.6 LTS with a minimal config for modern HTTPS.
# CFLAGS inherited from top-level Makefile (has -Os -flto
# -ffunction-sections -fdata-sections), so object files carry LTO IR.

MBEDTLS_DIR := third_party/mbedtls

MBEDTLS_CFLAGS := \
    -DMBEDTLS_CONFIG_FILE='"wbl/mbedtls_config.h"' \
    -I$(MBEDTLS_DIR)/include \
    -I$(CURDIR)/third_party/mbedtls-wbl-config

MBEDTLS_LIB    := $(MBEDTLS_DIR)/library/libmbedtls.a
MBEDX509_LIB   := $(MBEDTLS_DIR)/library/libmbedx509.a
MBEDCRYPTO_LIB := $(MBEDTLS_DIR)/library/libmbedcrypto.a

# Use gcc-ar/gcc-ranlib so LTO bitcode in each .o ends up properly
# indexed in the archive symbol table. Plain 'ar' drops the LTO
# symbol index and the final link silently falls back to non-LTO
# across the library boundary.
AR_LTO     := $(shell $(CC) -print-prog-name=gcc-ar 2>/dev/null)
RANLIB_LTO := $(shell $(CC) -print-prog-name=gcc-ranlib 2>/dev/null)
ifeq ($(AR_LTO),)
AR_LTO     := $(AR)
RANLIB_LTO := $(RANLIB)
endif

# Invoke upstream mbedTLS library Makefile with our toolchain + config.
$(MBEDTLS_LIB) $(MBEDX509_LIB) $(MBEDCRYPTO_LIB):
	$(MAKE) -C $(MBEDTLS_DIR)/library \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) $(MBEDTLS_CFLAGS)" \
	    no_test=1 static

.PHONY: mbedtls-clean
mbedtls-clean:
	$(MAKE) -C $(MBEDTLS_DIR)/library clean
