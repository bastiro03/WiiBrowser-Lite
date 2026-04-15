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
#
# `gcc -print-prog-name=X` returns either the full path to X if found
# or the literal name "X" if not found. We check existence on disk
# rather than just relying on the return value being empty.
AR_LTO_PROBE     := $(shell $(CC) -print-prog-name=gcc-ar 2>/dev/null)
RANLIB_LTO_PROBE := $(shell $(CC) -print-prog-name=gcc-ranlib 2>/dev/null)
AR_LTO     := $(if $(wildcard $(AR_LTO_PROBE)),$(AR_LTO_PROBE),$(AR))
RANLIB_LTO := $(if $(wildcard $(RANLIB_LTO_PROBE)),$(RANLIB_LTO_PROBE),$(RANLIB))

# Patches from third_party/mbedtls-patches/ must be applied to the
# submodule SOURCE TREE before any compile, because some patches affect
# headers that downstream consumers (curl, our app) #include directly.
# This must happen even when CI cache restored the .a files: the
# headers from the un-patched submodule would still be wrong.
#
# Use a stamp file (.wbl-patches-applied) to make application
# idempotent. Stamp is invalidated when any patch file changes.
MBEDTLS_PATCH_STAMP := $(MBEDTLS_DIR)/.wbl-patches-applied

$(MBEDTLS_PATCH_STAMP): $(MBEDTLS_PATCHES)
	@echo "Applying mbedtls patches to $(MBEDTLS_DIR):"
	@cd $(MBEDTLS_DIR) && git checkout -- . 2>/dev/null || true
	@for p in $(abspath $(MBEDTLS_PATCHES)); do \
	    if (cd $(MBEDTLS_DIR) && git apply --check "$$p" 2>/dev/null); then \
	        cd $(MBEDTLS_DIR) && git apply "$$p" && echo "  applied: $$(basename $$p)"; \
	    else \
	        echo "  SKIP (does not apply or already applied): $$(basename $$p)"; \
	    fi; \
	done
	@touch $@

# Make the lib targets depend on the patch stamp so patches apply
# before compilation. The stamp is also a public dep so other rules
# (e.g. curl.mk) can depend on it.
$(MBEDTLS_LIB) $(MBEDX509_LIB) $(MBEDCRYPTO_LIB): $(MBEDTLS_PATCH_STAMP)
	$(MAKE) -C $(MBEDTLS_DIR)/library \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) $(MBEDTLS_CFLAGS)" \
	    no_test=1 static

.PHONY: mbedtls-clean
mbedtls-clean:
	$(MAKE) -C $(MBEDTLS_DIR)/library clean
