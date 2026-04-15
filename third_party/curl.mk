# libcurl 8.16 build configuration for WiiBrowser-Lite
#
# Minimal curl with mbedTLS backend, suitable for embedded HTTPS browsing.
# Disables every protocol/feature not needed for fetching web pages.

CURL_DIR := third_party/curl

# Build flags: keep small, only HTTP/HTTPS, no cookies-on-disk, no proxy auth
CURL_CONFIGURE_FLAGS := \
    --host=powerpc-eabi \
    --target=powerpc-eabi \
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
    --without-ssl \
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
    --disable-verbose \
    --disable-sspi \
    --disable-ntlm \
    --disable-ntlm-wb \
    --disable-tls-srp \
    --disable-unix-sockets \
    --disable-cookies \
    --disable-socketpair \
    --disable-progress-meter \
    --disable-dateparse \
    --disable-dnsshuffle \
    --disable-doh \
    --disable-mime \
    --disable-bindlocal \
    --disable-form-api \
    --disable-headers-api \
    --disable-hsts \
    --disable-alt-svc

CURL_LIB := $(CURL_DIR)/lib/.libs/libcurl.a

$(CURL_LIB): | $(MBEDTLS_LIB)
	cd $(CURL_DIR) && autoreconf -fi
	cd $(CURL_DIR) && CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" \
	    CFLAGS="$(CFLAGS) -I$(CURDIR)/$(MBEDTLS_DIR)/include -Iinclude" \
	    LDFLAGS="-L$(CURDIR)/$(MBEDTLS_DIR)/library" \
	    ./configure $(CURL_CONFIGURE_FLAGS)
	$(MAKE) -C $(CURL_DIR)

.PHONY: curl-clean
curl-clean:
	-$(MAKE) -C $(CURL_DIR) distclean
