# Host-platform smoke tests, compiled with LTO so we exercise the
# same cross-TU optimization pipeline as a real Wii/DS build.
#
# The point: prove that bitcode survives in libmbedtls.a / libcurl.a /
# libquickjs.a and that the final link can inline across them.
#
# Usage:
#   make                 # build + run all smoke tests
#   make tls_smoke       # just raw mbedTLS handshake test
#   make curl_smoke      # libcurl + mbedTLS HTTPS GET
#   make js_smoke        # QuickJS wrapper
#   make lto_check       # symbol/bitcode verification

CC      ?= gcc
CXX     ?= g++
AR      ?= gcc-ar
RANLIB  ?= gcc-ranlib

MBEDTLS_DIR := ../third_party/mbedtls
MBEDTLS_CFG := ../third_party/mbedtls-wbl-config
QUICKJS_DIR := ../third_party/quickjs
CURL_DIR    := ../third_party/curl

# LTO flags applied to every TU:
#   -flto                object files get bitcode
#   -ffunction-sections  dead-strip granularity
#   -fdata-sections
# These match what the Wii/DS builds use.
LTO_CFLAGS := -Os -flto -ffunction-sections -fdata-sections

# Final-link LTO flags:
#   -flto                  rerun optimizer across all bitcode inputs
#   -fuse-linker-plugin    linker plugin reads LTO IR from .a files
#   -flto-partition=one    whole-program (single partition) for max inlining
LTO_LDFLAGS := -flto -fuse-linker-plugin -flto-partition=one \
               -Wl,--gc-sections

# MBEDTLS_CFG is on the include path BEFORE mbedtls's own include/ so our
# override of <mbedtls/mbedtls_config.h> wins. See
# third_party/mbedtls-wbl-config/mbedtls/mbedtls_config.h - a symlink to
# our minimal config. This avoids -DMBEDTLS_CONFIG_FILE='"..."' whose
# nested quotes get mangled by autoconf/nested-make layers.
CFLAGS := $(LTO_CFLAGS) -I../include \
          -I$(MBEDTLS_CFG) -I$(MBEDTLS_DIR)/include \
          -I$(QUICKJS_DIR) \
          -DWBL_PLATFORM_HOST=1 \
          -Wall -Wno-unused-function

# ASAN/UBSAN for host smoke tests (not available on Wii cross-compile).
# ASAN catches heap bugs (use-after-free, double-free, buffer overruns).
# UBSAN catches undefined behavior (integer overflow, null deref, etc.).
# Leak sanitizer piggybacks on ASAN when available.
SANITIZE_FLAGS := -fsanitize=address,undefined -fno-omit-frame-pointer
CFLAGS += $(SANITIZE_FLAGS)

MBEDTLS_LIBS := $(MBEDTLS_DIR)/library/libmbedtls.a \
                $(MBEDTLS_DIR)/library/libmbedx509.a \
                $(MBEDTLS_DIR)/library/libmbedcrypto.a

QUICKJS_LIBS := $(QUICKJS_DIR)/libquickjs.a

CURL_LIB := $(CURL_DIR)/lib/.libs/libcurl.a

all: tls_smoke js_smoke curl_smoke network_stress test_freetype_init test_guitext_init
	@echo ""
	@echo "=== tls_smoke (raw mbedTLS handshake) ==="
	./tls_smoke
	@echo ""
	@echo "=== curl_smoke (libcurl + mbedTLS HTTPS GET) ==="
	./curl_smoke
	@echo ""
	@echo "=== js_smoke (QuickJS wrapper) ==="
	./js_smoke
	@echo ""
	@echo "=== network_stress (net_init/deinit cycling with ASAN/UBSAN) ==="
	./network_stress

# Build each test binary with LTO_LDFLAGS so the linker plugin
# runs over all of our static archives.

tls_smoke: tls_smoke.c $(MBEDTLS_LIBS)
	$(CC) $(CFLAGS) $(LTO_LDFLAGS) $< $(MBEDTLS_LIBS) -o $@

js_smoke: js_smoke.c ../src/wbl/js_engine.c $(QUICKJS_LIBS)
	$(CC) $(CFLAGS) $(LTO_LDFLAGS) $< ../src/wbl/js_engine.c $(QUICKJS_LIBS) \
	    -o $@ -lm -lpthread -ldl

# curl_smoke intentionally does NOT include ../include on the search
# path because our Wii-specific sys/socket.h would collide with
# libcurl's portability header expectations.
curl_smoke: curl_smoke.c $(CURL_LIB) $(MBEDTLS_LIBS)
	$(CC) $(LTO_CFLAGS) $(LTO_LDFLAGS) -I$(CURL_DIR)/include -Wall \
	    $< $(CURL_LIB) $(MBEDTLS_LIBS) -lz -o $@

# network_stress: tests the InitNetwork() invariant that the bug fix
# establishes. Verifies the BUGGY version fails the invariant checks
# (bug reproduced) and the FIXED version passes them (bug gone). ASAN
# also catches any heap bugs in the test harness itself.
network_stress: network_stress.c
	$(CC) $(CFLAGS) $(SANITIZE_FLAGS) $< -o $@

# test_freetype_init: Reproduces the FreeTypeGX fontSystem[] array
# initialization bug where slots 50-100 were left uninitialized, causing
# dereference of garbage pointers when rendering text with size >= 50.
# Crash manifested as DSI exception at PC=0x800066C4 with r3=0xC (garbage
# this pointer) in RB-tree::find() when GuiFileBrowser tried to render.
test_freetype_init: test_freetype_init.cpp
	$(CXX) -std=c++11 -O0 -g $(SANITIZE_FLAGS) $< -o $@

# test_guitext_init: Reproduces the GuiText constructor bug where
# `font` (FreeTypeGX*) and `currentSize` (int) members are left
# uninitialized. Combined with gui_longtext.cpp's
#   ftgx = (font ? font : fontSystem[currentSize])
# line, this causes dereference of garbage pointers → DSI crash
# with r3 = 0xC (offset of FreeTypeGX::fontData).
test_guitext_init: test_guitext_init.cpp
	$(CXX) -std=c++11 -O0 -g $(SANITIZE_FLAGS) $< -o $@

MBEDTLS_PATCHES := $(sort $(wildcard ../third_party/mbedtls-patches/*.patch))
CURL_PATCHES    := $(sort $(wildcard ../third_party/curl-patches/*.patch))
QUICKJS_PATCHES := $(sort $(wildcard ../third_party/quickjs-patches/*.patch))

# --- Build mbedTLS with LTO bitcode in the archive ---
$(MBEDTLS_LIBS):
	@for p in $$(realpath $(MBEDTLS_PATCHES) 2>/dev/null); do \
	    (cd $(MBEDTLS_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(MBEDTLS_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	$(MAKE) -C $(MBEDTLS_DIR)/library \
	    CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" \
	    CFLAGS="$(LTO_CFLAGS) -I$$(pwd)/$(MBEDTLS_CFG) -I$$(pwd)/$(MBEDTLS_DIR)/include -DWBL_PLATFORM_HOST=1 -Wno-unused-function" \
	    static

# --- Build QuickJS with LTO bitcode ---
QJS_OBJS := $(addprefix $(QUICKJS_DIR)/,quickjs.o libregexp.o libunicode.o dtoa.o)

$(QUICKJS_DIR)/.wbl-patches-applied: $(QUICKJS_PATCHES)
	@for p in $$(realpath $(QUICKJS_PATCHES) 2>/dev/null); do \
	    (cd $(QUICKJS_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(QUICKJS_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	@touch $@

$(QUICKJS_DIR)/%.o: $(QUICKJS_DIR)/%.c $(QUICKJS_DIR)/.wbl-patches-applied
	$(CC) $(LTO_CFLAGS) -D_GNU_SOURCE -DCONFIG_VERSION=\"0.14.0-micro\" \
	    -Wno-array-bounds -Wno-format-truncation -Wno-unused-but-set-variable \
	    -fwrapv -funsigned-char -c $< -o $@

$(QUICKJS_LIBS): $(QJS_OBJS)
	$(AR) rcs $@ $^

# --- Build curl with LTO bitcode (delegates to upstream configure+make) ---
$(CURL_LIB): $(MBEDTLS_LIBS)
	@for p in $$(realpath $(CURL_PATCHES) 2>/dev/null); do \
	    (cd $(CURL_DIR) && git apply --check "$$p" 2>/dev/null) \
	        && (cd $(CURL_DIR) && git apply "$$p" && echo "applied $$(basename $$p)") \
	        || true; \
	done
	cd $(CURL_DIR) && [ -x ./configure ] || autoreconf -fi
	cd $(CURL_DIR) && \
	    CC="$(CC)" AR="$(AR)" RANLIB="$(RANLIB)" \
	    CFLAGS="$(LTO_CFLAGS) -I$$(pwd)/../mbedtls-wbl-config -I$$(pwd)/../mbedtls/include -DWBL_PLATFORM_HOST=1" \
	    LDFLAGS="$(LTO_LDFLAGS) -L$$(pwd)/../mbedtls/library" \
	    ./configure --disable-shared --enable-static \
	                --without-libpsl --without-libidn2 --without-librtmp \
	                --without-zstd --without-brotli --without-libssh2 \
	                --without-nghttp2 --without-ngtcp2 \
	                --with-mbedtls=$$(pwd)/../mbedtls \
	                --disable-ftp --disable-file --disable-ldap --disable-ldaps \
	                --disable-rtsp --disable-dict --disable-telnet --disable-tftp \
	                --disable-pop3 --disable-imap --disable-smb --disable-smtp \
	                --disable-gopher --disable-mqtt --disable-manual \
	                --disable-libcurl-option --disable-sspi --disable-ntlm \
	                --disable-tls-srp --disable-unix-sockets \
	                --disable-dnsshuffle --disable-doh --disable-mime \
	                --disable-bindlocal --disable-form-api --disable-headers-api \
	                --disable-hsts --disable-alt-svc
	$(MAKE) -C $(CURL_DIR) AR="$(AR)" RANLIB="$(RANLIB)"

# --- LTO verification: grep for GCC LTO bitcode marker in archives ---
.PHONY: lto_check
lto_check: $(MBEDTLS_LIBS) $(QUICKJS_LIBS) $(CURL_LIB)
	@echo "=== Verifying LTO bitcode present in static archives ==="
	@for lib in $(MBEDTLS_LIBS) $(QUICKJS_LIBS) $(CURL_LIB); do \
	    echo "--- $$lib ---"; \
	    $(AR) t $$lib | head -3; \
	    # A GCC LTO section is called .gnu.lto_<funcname>. \
	    # If we see it, the object has bitcode. \
	    $(CC) -c -x none $$lib -o /dev/null 2>&1 | head -2 || true; \
	    count=$$($(AR) t $$lib | head -10 | while read obj; do \
	        $(AR) p $$lib $$obj 2>/dev/null | readelf -S - 2>/dev/null | grep -c gnu.lto; \
	    done | awk '{s+=$$1} END {print s}'); \
	    echo "  LTO sections in first 10 objects: $$count"; \
	done

clean:
	rm -f tls_smoke js_smoke curl_smoke network_stress *.o
	cd $(MBEDTLS_DIR)/library && $(MAKE) clean
	cd $(QUICKJS_DIR) && rm -f *.o libquickjs.a
	-cd $(CURL_DIR) && $(MAKE) distclean 2>/dev/null || true
	-cd $(CURL_DIR) && git clean -fdx 2>/dev/null || true

.PHONY: all clean lto_check
