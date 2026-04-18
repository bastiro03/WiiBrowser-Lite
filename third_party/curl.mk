# libcurl 8.16 build configuration for WiiBrowser-Lite
#
# Minimal curl built with the mbedTLS backend, compiled with LTO so
# bitcode ends up in libcurl.a and the final link optimizes across
# the app <-> libcurl <-> libmbedtls boundaries.

CURL_DIR := third_party/curl

# For use inside $(filter-out ...) where literal commas are needed.
COMMA := ,

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
    --without-gsasl \
    --without-libgsasl \
    --without-gssapi \
    --without-libssh \
    --without-wolfssh \
    --without-msh3 \
    --without-quiche \
    --without-openssl-quic \
    --without-ca-bundle \
    --without-ca-path \
    --without-ca-fallback \
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
AR_LTO     := $(patsubst %-gcc,%-gcc-ar,$(CC))
RANLIB_LTO := $(patsubst %-gcc,%-gcc-ranlib,$(CC))
ifeq ($(wildcard $(AR_LTO)),)
AR_LTO     := $(AR)
RANLIB_LTO := $(RANLIB)
endif

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
    ac_cv_func_if_nametoindex=no \
    ac_cv_func_socketpair=no \
    ac_cv_func_getifaddrs=no \
    ac_cv_func_fork=no \
    ac_cv_func_setrlimit=no \
    curl_disallow_socketpair=yes \
    curl_disallow_getifaddrs=yes \
    curl_disallow_fork=yes \
    curl_disallow_pipe=yes \
    curl_disallow_alarm=yes \
    curl_disallow_sigaction=yes \
    curl_disallow_sigsetjmp=yes \
    curl_disallow_siginterrupt=yes \
    curl_disallow_poll=yes \
    ac_cv_func_poll=no \
    ac_cv_header_poll_h=no \
    curl_disallow_getsockname=yes \
    ac_cv_func_connect=yes

# Depend on curl.mk itself and the patch stack so that any change to
# our configure flags, CURL_CROSS_CACHE variables, or patches triggers
# a full curl rebuild. Without this, a CI cache restore-keys fallback
# or an incremental local rebuild would accept a stale libcurl.a that
# was configured with obsolete settings (e.g. HAVE_GETSOCKNAME=1 before
# we added curl_disallow_getsockname=yes).
$(CURL_LIB): third_party/curl.mk $(CURL_PATCHES) | $(MBEDTLS_LIB)
	@for p in $(abspath $(CURL_PATCHES)); do \
	    (cd $(CURL_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(CURL_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	cd $(CURL_DIR) && [ -x ./configure ] || autoreconf -fi
	# Force a full distclean + config wipe whenever this rule fires.
	# Previously we only distcleaned when config.status existed, which
	# meant a partial cache restore (cached libcurl.a + config.status)
	# could skip reconfiguration and reuse stale feature-detection
	# results like HAVE_GETSOCKNAME=1 from before commit 151ca1a.
	cd $(CURL_DIR) && { [ -f Makefile ] && $(MAKE) distclean || true; }
	cd $(CURL_DIR) && rm -f config.status config.log config.cache
	# Use include-path override for mbedTLS config rather than
	# -DMBEDTLS_CONFIG_FILE='"..."' (nested quotes get eaten by autoconf).
	#
	# Link-path notes for Wii (libogc):
	# - libogc's sys/socket.h declares socket(), connect(), etc. but the
	#   actual symbols are inside libnetport (which delegates to libogc's
	#   net_socket()). Without -lnetport on LDFLAGS, curl's configure
	#   link test for socket() fails and transfer.c fires
	#   "#error We cannot compile without socket() support!".
	# - libnetport itself requires -logc for net_*, so list both.
	# CPPFLAGS needs libogc + portlib + mbedtls-wbl-config paths so
	# curl's preprocessor-based feature checks (e.g. "is socket
	# prototyped") can find <sys/socket.h> from libogc. Without it,
	# curl thinks sockets are unavailable and fires
	# "#error We cannot compile without socket() support!"
	cd $(CURL_DIR) && env $(CURL_CROSS_CACHE) \
	    CC="$(CC)" AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)" \
	    CFLAGS="$(CFLAGS) -I$(CURDIR)/third_party/mbedtls-wbl-config -I$(CURDIR)/$(MBEDTLS_DIR)/include" \
	    CPPFLAGS="-I$(LIBOGC_INC) -I$(PORTLIBS_INC) -I$(CURDIR)/third_party/mbedtls-wbl-config -I$(CURDIR)/$(MBEDTLS_DIR)/include" \
	    LDFLAGS="$(filter-out -Wl$(COMMA)--wrap=%,$(LDFLAGS)) -L$(CURDIR)/$(MBEDTLS_DIR)/library -L$(CURDIR)/libs/wii -L$(LIBOGC_LIB)" \
	    LIBS="-lnetport -logc" \
	    ./configure $(CURL_CONFIGURE_FLAGS)
	# Build ONLY the libcurl archive, not the `curl` CLI binary.
	# The CLI would link against mbedtls and need mbedtls_hardware_poll
	# + mbedtls_ms_time from our hooks - but those live in the app,
	# not in a standalone libmbedtls.a. We only need libcurl.a anyway.
	$(MAKE) -C $(CURL_DIR)/lib \
	    AR="$(AR_LTO)" RANLIB="$(RANLIB_LTO)"

.PHONY: curl-clean
curl-clean:
	-$(MAKE) -C $(CURL_DIR) distclean 2>/dev/null
	-cd $(CURL_DIR) && git clean -fdx 2>/dev/null
