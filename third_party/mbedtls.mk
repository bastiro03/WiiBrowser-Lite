# mbedTLS build configuration for WiiBrowser-Lite
#
# Builds mbedTLS 3.6.6 LTS with a minimal config for modern HTTPS.
# CFLAGS inherited from top-level Makefile (has -Os -flto
# -ffunction-sections -fdata-sections), so object files carry LTO IR.

MBEDTLS_DIR := third_party/mbedtls

# We override mbedTLS's default include <mbedtls/mbedtls_config.h>
# with our own file at third_party/mbedtls-wbl-config/mbedtls/mbedtls_config.h,
# picked up by putting our include path AHEAD of mbedtls's own include/.
# This avoids the -DMBEDTLS_CONFIG_FILE='"..."' approach which gets its
# quotes mangled by successive Make/shell invocations when nested build
# systems pass CFLAGS through.
MBEDTLS_CFLAGS := \
    -I$(CURDIR)/third_party/mbedtls-wbl-config \
    -I$(MBEDTLS_DIR)/include

MBEDTLS_LIB    := $(MBEDTLS_DIR)/library/libmbedtls.a
MBEDX509_LIB   := $(MBEDTLS_DIR)/library/libmbedx509.a
MBEDCRYPTO_LIB := $(MBEDTLS_DIR)/library/libmbedcrypto.a

MBEDTLS_PATCHES := $(sort $(wildcard third_party/mbedtls-patches/*.patch))

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

# Apply any discrete upstream-submittable patches from
# third_party/mbedtls-patches/ before compiling.
$(MBEDTLS_LIB) $(MBEDX509_LIB) $(MBEDCRYPTO_LIB):
	@for p in $(abspath $(MBEDTLS_PATCHES)); do \
	    (cd $(MBEDTLS_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(MBEDTLS_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	$(MAKE) -C $(MBEDTLS_DIR)/library \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) $(MBEDTLS_CFLAGS)" \
	    no_test=1 static

.PHONY: mbedtls-clean
mbedtls-clean:
	$(MAKE) -C $(MBEDTLS_DIR)/library clean
