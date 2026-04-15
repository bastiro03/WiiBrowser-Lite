# MicroQuickJS build configuration for WiiBrowser-Lite
#
# Builds a minimal QuickJS suitable for embedded targets.
# Saves ~120KB vs full build by disabling BigNum/Atomics/Date parsing.

QUICKJS_DIR := third_party/quickjs

QUICKJS_SRCS := \
    $(QUICKJS_DIR)/quickjs.c \
    $(QUICKJS_DIR)/libregexp.c \
    $(QUICKJS_DIR)/libunicode.c \
    $(QUICKJS_DIR)/dtoa.c

QUICKJS_OBJS := $(patsubst $(QUICKJS_DIR)/%.c,build/quickjs/%.o,$(QUICKJS_SRCS))

# MicroQuickJS configuration:
#   CONFIG_VERSION    - version string
#   No CONFIG_BIGNUM  - drops BigInt/BigFloat/BigDecimal (~80KB)
#   No CONFIG_ATOMICS - drops SharedArrayBuffer/Atomics (~10KB)
#   No CONFIG_STORAGE - drops localStorage shim
QUICKJS_CFLAGS := \
    -DCONFIG_VERSION=\"0.14.0-micro\" \
    -DCONFIG_STACK_CHECK \
    -D_GNU_SOURCE \
    -fwrapv \
    -funsigned-char \
    -Wno-array-bounds \
    -Wno-format-truncation \
    -Wno-unused-but-set-variable

# CFLAGS from top-level Makefile already carries -flto -Os -ffunction-sections.
# We inherit it and add only QuickJS-specific flags. Object files will
# contain LTO bitcode so the final link can inline across QuickJS -> app.

# Compile rule - use the compiler's ar wrapper so the linker plugin
# knows how to look up LTO symbols in the archive.
AR_LTO := $(shell $(CC) -print-prog-name=ar 2>/dev/null)
ifeq ($(AR_LTO),)
AR_LTO := $(AR)
endif

build/quickjs/%.o: $(QUICKJS_DIR)/%.c | build/quickjs
	$(CC) $(CFLAGS) $(QUICKJS_CFLAGS) -I$(QUICKJS_DIR) -c $< -o $@

build/quickjs:
	mkdir -p $@

# Static archive - use gcc-ar (via -print-prog-name) so LTO bitcode
# indexing works. Plain 'ar' drops the symbol table for IR symbols.
build/libquickjs.a: $(QUICKJS_OBJS)
	$(AR_LTO) rcs $@ $^

.PHONY: quickjs-clean
quickjs-clean:
	rm -rf build/quickjs build/libquickjs.a
