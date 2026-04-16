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

# Apply discrete upstream-submittable patches from third_party/curl-patches/.
# Order is determined by filename (0001-, 0002-, ...).
# Application is idempotent: `git apply --check` gates application so
# re-running the build does not fail on already-patched files.
CURL_PATCHES := $(sort $(wildcard third_party/curl-patches/*.patch))

# Same LTO archiver as mbedtls.mk uses (see comment there).
AR_LTO_PROBE     := $(shell $(CC) -print-prog-name=gcc-ar 2>/dev/null)
RANLIB_LTO_PROBE := $(shell $(CC) -print-prog-name=gcc-ranlib 2>/dev/null)
AR_LTO     := $(if $(wildcard $(AR_LTO_PROBE)),$(AR_LTO_PROBE),$(AR))
RANLIB_LTO := $(if $(wildcard $(RANLIB_LTO_PROBE)),$(RANLIB_LTO_PROBE),$(RANLIB))

# Cross-compile autoconf cache variables. When building for PPC/ARM,
# curl's configure script wants to run test programs on the target to
# feature-detect (malloc(0), recv/send signatures, etc.). Cross builds
# can't execute target code, so we pre-answer the tests with values
# that match the devkitPPC + libogc (or devkitARM + libnds) network
# stack. These are bare-metal embedded targets with a BSD-ish sockets
# API provided by libogc.net / dswifi, but NOT full Linux extensions
# like accept4/recvmmsg/sendmmsg/epoll/eventfd.
CURL_CROSS_CACHE := \
    ac_cv_func_malloc_0_nonnull=yes \
    ac_cv_func_realloc_0_nonnull=yes \
    ac_cv_func_memcmp_working=yes \
    curl_cv_func_recv=yes \
    curl_cv_func_recv_args="int,void*,size_t,int,int" \
    curl_cv_func_send=yes \
    curl_cv_func_send_args="int,const void*,size_t,int,int" \
    curl_cv_recv=yes \
    curl_cv_send=yes \
    ac_cv_func_accept4=no \
    ac_cv_func_recvmmsg=no \
    ac_cv_func_sendmmsg=no \
    ac_cv_func_epoll_create=no \
    ac_cv_func_epoll_create1=no \
    ac_cv_func_eventfd=no \
    ac_cv_func_fsetxattr=no \
    ac_cv_func_pipe2=no \
    ac_cv_func_if_nametoindex=no

$(CURL_LIB): | $(MBEDTLS_LIB)
	@for p in $(abspath $(CURL_PATCHES)); do \
	    (cd $(CURL_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(CURL_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	cd $(CURL_DIR) && [ -x ./configure ] || autoreconf -fi
	# Use include-path override for mbedTLS config rather than
	# -DMBEDTLS_CONFIG_FILE='"..."' (nested quotes get eaten by autoconf).
	cd $(CURL_DIR) && env $(CURL_CROSS_CACHE) \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) -I$(CURDIR)/third_party/mbedtls-wbl-config -I$(CURDIR)/$(MBEDTLS_DIR)/include" \
	    LDFLAGS="$(LDFLAGS) -L$(CURDIR)/$(MBEDTLS_DIR)/library" \
	    ./configure $(CURL_CONFIGURE_FLAGS)
	$(MAKE) -C $(CURL_DIR) \
	    AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)"

.PHONY: curl-clean
curl-clean:
	-$(MAKE) -C $(CURL_DIR) distclean 2>/dev/null
	-cd $(CURL_DIR) && git clean -fdx 2>/dev/null
