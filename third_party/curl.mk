# libcurl 8.16 build configuration for WiiBrowser-Lite
#
# Minimal curl built with the mbedTLS backend, compiled with LTO so
# bitcode ends up in libcurl.a and the final link optimizes across
# the app <-> libcurl <-> libmbedtls boundaries.

CURL_DIR := third_party/curl

# Host triplet from platform/*/build.mk. For Wii it's powerpc-eabi,
# for DS/DSi it's arm-none-eabi, for Mac Plus m68k-apple-macos.
CURL_HOST_TRIPLET ?= powerpc-eabi

CURL_CONFIGURE_FLAGS := \
    --host=$(CURL_HOST_TRIPLET) \
    --disable-shared \
    --enable-static \
    --without-libpsl \
    --without-libidn2 \
    --without-librtmp \
    --without-zstd \
    --without-brotli \
    --without-libssh2 \
    --without-nghttp2 \
    --without-ngtcp2 \
    --with-mbedtls=$(CURDIR)/$(MBEDTLS_DIR) \
    --enable-http \
    --enable-proxy \
    --disable-ftp \
    --disable-file \
    --disable-ldap \
    --disable-ldaps \
    --disable-rtsp \
    --disable-dict \
    --disable-telnet \
    --disable-tftp \
    --disable-pop3 \
    --disable-imap \
    --disable-smb \
    --disable-smtp \
    --disable-gopher \
    --disable-mqtt \
    --disable-manual \
    --disable-libcurl-option \
    --disable-sspi \
    --disable-ntlm \
    --disable-ntlm-wb \
    --disable-tls-srp \
    --disable-unix-sockets \
    --disable-cookies \
    --disable-dnsshuffle \
    --disable-doh \
    --disable-mime \
    --disable-bindlocal \
    --disable-form-api \
    --disable-headers-api \
    --disable-hsts \
    --disable-alt-svc

CURL_LIB := $(CURL_DIR)/lib/.libs/libcurl.a

# Apply our curl source patch (gate tls13 session-tickets on TLS 1.3).
# Idempotent: if already applied, `git apply` reports "patch does not
# apply" which we ignore - the patch is in a known state.
CURL_PATCHES := $(wildcard third_party/curl-patches/*.patch)

# Same LTO archiver as mbedtls.mk uses.
AR_LTO     := $(shell $(CC) -print-prog-name=gcc-ar 2>/dev/null)
RANLIB_LTO := $(shell $(CC) -print-prog-name=gcc-ranlib 2>/dev/null)
ifeq ($(AR_LTO),)
AR_LTO     := $(AR)
RANLIB_LTO := $(RANLIB)
endif

$(CURL_LIB): | $(MBEDTLS_LIB)
	cd $(CURL_DIR) && \
	    for p in $(abspath $(CURL_PATCHES)); do \
	        git apply --check "$$p" 2>/dev/null && git apply "$$p"; \
	    done; true
	cd $(CURL_DIR) && [ -x ./configure ] || autoreconf -fi
	cd $(CURL_DIR) && \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) -I$(CURDIR)/$(MBEDTLS_DIR)/include -I$(CURDIR)/third_party/mbedtls-wbl-config -DMBEDTLS_CONFIG_FILE='\"wbl/mbedtls_config.h\"'" \
	    LDFLAGS="$(LDFLAGS) -L$(CURDIR)/$(MBEDTLS_DIR)/library" \
	    ./configure $(CURL_CONFIGURE_FLAGS)
	$(MAKE) -C $(CURL_DIR) \
	    AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)"

.PHONY: curl-clean
curl-clean:
	-$(MAKE) -C $(CURL_DIR) distclean 2>/dev/null
	-cd $(CURL_DIR) && git clean -fdx 2>/dev/null
