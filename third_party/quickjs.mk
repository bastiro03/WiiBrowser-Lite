# MicroQuickJS build configuration for WiiBrowser-Lite
#
# Builds a minimal QuickJS suitable for embedded targets.
# Saves ~120KB vs full build by disabling BigNum/Atomics/Date parsing.

QUICKJS_DIR := third_party/quickjs

QUICKJS_SRCS := \
    $(QUICKJS_DIR)/quickjs.c \
    $(QUICKJS_DIR)/libregexp.c \
    $(QUICKJS_DIR)/libunicode.c \
    $(QUICKJS_DIR)/cutils.c \
    $(QUICKJS_DIR)/dtoa.c \
    $(QUICKJS_DIR)/xsum.c

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

# Apply size optimization aggressively to QuickJS
QUICKJS_CFLAGS += -Os -ffunction-sections -fdata-sections

# Compile rule
build/quickjs/%.o: $(QUICKJS_DIR)/%.c | build/quickjs
	$(CC) $(CFLAGS) $(QUICKJS_CFLAGS) -I$(QUICKJS_DIR) -c $< -o $@

build/quickjs:
	mkdir -p $@

# Static archive
build/libquickjs.a: $(QUICKJS_OBJS)
	$(AR) rcs $@ $^

.PHONY: quickjs-clean
quickjs-clean:
	rm -rf build/quickjs build/libquickjs.a
