# Host-platform smoke tests
#
# These build with the host toolchain (gcc/clang) using WBL_PLATFORM_HOST.
# They're meant to catch bugs in our cross-platform wrappers and config
# before we pay the cost of a full Wii CI build.
#
# Usage:
#   cd tests
#   make                 # builds and runs all smoke tests
#   make tls_smoke       # just the TLS test
#   make js_smoke        # just the QuickJS test

CC ?= gcc
MBEDTLS_DIR := ../third_party/mbedtls
MBEDTLS_CFG := ../third_party/mbedtls-wbl-config
QUICKJS_DIR := ../third_party/quickjs

CFLAGS := -Os -I../include \
          -I$(MBEDTLS_DIR)/include -I$(MBEDTLS_CFG) \
          -I$(QUICKJS_DIR) \
          -DMBEDTLS_CONFIG_FILE='"wbl/mbedtls_config.h"' \
          -DWBL_PLATFORM_HOST=1 \
          -Wall -Wno-unused-function

MBEDTLS_LIBS := $(MBEDTLS_DIR)/library/libmbedtls.a \
                $(MBEDTLS_DIR)/library/libmbedx509.a \
                $(MBEDTLS_DIR)/library/libmbedcrypto.a

QUICKJS_LIBS := $(QUICKJS_DIR)/libquickjs.a

CURL_DIR := ../third_party/curl
CURL_LIB := $(CURL_DIR)/lib/.libs/libcurl.a

all: tls_smoke js_smoke curl_smoke
	@echo "=== tls_smoke (raw mbedTLS handshake) ==="
	./tls_smoke
	@echo ""
	@echo "=== curl_smoke (full libcurl + mbedTLS HTTPS GET) ==="
	./curl_smoke
	@echo ""
	@echo "=== js_smoke (QuickJS wrapper) ==="
	./js_smoke

# --- TLS: actually connect to a real HTTPS server and fetch /zen ---
tls_smoke: tls_smoke.c $(MBEDTLS_LIBS)
	$(CC) $(CFLAGS) $< $(MBEDTLS_LIBS) -o $@

# --- QuickJS: create context, eval simple expression, check error path ---
js_smoke: js_smoke.c ../src/wbl/js_engine.c $(QUICKJS_LIBS)
	$(CC) $(CFLAGS) $< ../src/wbl/js_engine.c $(QUICKJS_LIBS) -o $@ -lm -lpthread -ldl

# --- libcurl+mbedTLS: full HTTPS GET ---
# Note: no -I../include here because our Wii sys/socket.h would
# clash with host libcurl's needs.
curl_smoke: curl_smoke.c $(CURL_LIB) $(MBEDTLS_LIBS)
	$(CC) -Os -I$(CURL_DIR)/include -Wall $< $(CURL_LIB) $(MBEDTLS_LIBS) -lz -o $@

# --- Build mbedTLS library for host ---
$(MBEDTLS_LIBS):
	cd $(MBEDTLS_DIR)/library && \
	  $(MAKE) CFLAGS="-Os -I$$(pwd)/../include -I$$(pwd)/../../../third_party/mbedtls-wbl-config -DMBEDTLS_CONFIG_FILE='\"wbl/mbedtls_config.h\"' -DWBL_PLATFORM_HOST=1 -Wno-unused-function" static

# --- Build QuickJS library for host ---
$(QUICKJS_LIBS):
	cd $(QUICKJS_DIR) && \
	  $(CC) -Os -c -D_GNU_SOURCE -DCONFIG_VERSION=\"0.14.0-micro\" \
	    -Wno-array-bounds -Wno-format-truncation -Wno-unused-but-set-variable \
	    -fwrapv -funsigned-char \
	    quickjs.c libregexp.c libunicode.c dtoa.c && \
	  ar rcs libquickjs.a quickjs.o libregexp.o libunicode.o dtoa.o

clean:
	rm -f tls_smoke js_smoke curl_smoke *.o
	cd $(MBEDTLS_DIR)/library && $(MAKE) clean
	cd $(QUICKJS_DIR) && rm -f *.o libquickjs.a

.PHONY: all clean
