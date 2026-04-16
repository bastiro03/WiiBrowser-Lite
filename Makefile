#---------------------------------------------------------------------------------
# WiiBrowser-Lite cross-platform build
#
# Default platform: WII
# Override with: make WBL_PLATFORM=nds | dsi | macplus
#---------------------------------------------------------------------------------
.SUFFIXES:

WBL_PLATFORM ?= wii

# Project Name MUST be set before we include the platform-specific
# build.mk, because the platform file uses $(TARGET) to build
# target-output names like $(TARGET).dol.
TARGET ?= wiibrowserlite

# SOURCES list is used by platform/*/build.mk to add -Isrc/subdir to
# CFLAGS (so our app source can resolve #include "foo.h" relative
# to src/<subdir>). Must be set before the platform include.
#
# Exclusions (TODO: migrate to submodules / rewrite against current
# library APIs; see libs/wii/README.md for the full migration plan):
# - src/archiveoperations/: legacy libunrar+libsevenzip wrappers that
#   don't compile against current unrarlib headers. Not needed for
#   Wikipedia browsing.
# - src/utils/: old JPEG/BMP loader glue (jmemsrc.cpp has jpeg API
#   drift; easybmp.cpp + pngu.c reference MAX_TEX_WIDTH that came
#   from an old dependency). For now we rely on portlibs' libjpeg/
#   libpng directly; image decode wrapper needs rewriting.
# - src/textoperations/: text editor (TextPointer, TextEditor) calls
#   GuiText methods that no longer exist (GetFontSize, GetTextLine,
#   GetStartWidth, getCharWidth). Not a browser core feature.
SOURCES := $(filter-out \
    src/archiveoperations \
    src/textoperations \
    , $(shell find src -type d))

# Individual file excludes within kept dirs
#
# pngu.c is kept: it provides DecodePNG() used by gui_imagedata.cpp for
# the app's built-in PNG assets (buttons, backgrounds). The old comment
# about it "referencing MAX_TEX_WIDTH from an old dependency" no longer
# applies - pngu.c now provides sane #ifndef defaults for those macros.
#
# Excluded:
#  giflib.c       - legacy GIF loader, not needed for Wikipedia browsing
#  mem2_manager.c - unused mem2 heap helper
#  timer.c        - legacy perf timer (depends on wiibrowser-specific APIs)
EXCLUDE_CFILES := giflib.c mem2_manager.c timer.c
EXCLUDE_CPPFILES := Metaphrasis.cpp easybmp.cpp jmemsrc.cpp

# Default goal MUST be declared before we include any third_party/*.mk,
# because make uses the FIRST rule defined as the default target. The
# third_party makefiles define rules for libmbedtls.a etc. that would
# otherwise become the default, making `make` silently build just one
# library and exit without producing our .dol.
.DEFAULT_GOAL := all

# Include platform-specific build config
include platform/$(WBL_PLATFORM)/build.mk

# Directories
BUILD := build/$(WBL_PLATFORM)
# SOURCES set above, pre-platform-include
DATA := images images/appbar fonts sounds certs src/lang
INCLUDES := include

VPATH := $(SOURCES) $(DATA)

#---------------------------------------------------------------------------------
# Common cross-platform feature flags
#---------------------------------------------------------------------------------
WBL_CFLAGS := \
    -DWBL_PLATFORM_$(shell echo $(WBL_PLATFORM) | tr a-z A-Z) \
    -Iinclude

# Embed a build-id string in every .o so the resulting .dol is
# uniquely identifiable with `strings wiibrowserlite.dol | grep WBL_BUILD_ID`.
# This lets us distinguish the latest build from a stale CI artifact or
# a cached download. We use short SHA + UTC timestamp so even two builds
# at the same commit differ (useful when rebuilding after a cache miss).
WBL_BUILD_SHA := $(shell git rev-parse --short=12 HEAD 2>/dev/null || echo nogit)
WBL_BUILD_TS := $(shell date -u +%Y%m%dT%H%M%SZ)
WBL_BUILD_ID := WBL_BUILD_ID:$(WBL_BUILD_SHA)@$(WBL_BUILD_TS)
WBL_CFLAGS += -DWBL_BUILD_ID_STRING='"$(WBL_BUILD_ID)"'

# Headers for our third-party deps must come BEFORE portlib headers on the
# search path so we pick up our pinned/patched versions rather than the
# devkitpro portlibs copies of mbedtls/curl/etc.
#
# Note ordering for mbedTLS: -Ithird_party/mbedtls-wbl-config BEFORE
# -Ithird_party/mbedtls/include. Our wbl-config dir contains an override
# for <mbedtls/mbedtls_config.h> (symlinked to our minimal config) so
# #include <mbedtls/mbedtls_config.h> picks up our file first.
WBL_THIRD_PARTY_INC := \
    -Ithird_party/curl/include \
    -Ithird_party/mbedtls-wbl-config \
    -Ithird_party/mbedtls/include \
    -Ithird_party/quickjs

CFLAGS   := $(WBL_THIRD_PARTY_INC) $(WBL_CFLAGS) $(CFLAGS)
CXXFLAGS := $(WBL_THIRD_PARTY_INC) $(WBL_CFLAGS) $(CXXFLAGS)

#---------------------------------------------------------------------------------
# Source Files
#---------------------------------------------------------------------------------
CFILES := $(filter-out $(EXCLUDE_CFILES), \
    $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c))))
CPPFILES := $(filter-out $(EXCLUDE_CPPFILES), \
    $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp))))
sFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
TTFFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ttf)))
LANGFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.lang)))
PNGFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.png)))
JPGFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.jpg)))
GIFFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.gif)))
OGGFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.pcm)))
PEMFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.pem)))
OFILES       := $(CFILES:.c=.o) $(CPPFILES:.cpp=.o) \
                $(sFILES:.s=.o) $(SFILES:.S=.o) \
                $(TTFFILES:.ttf=.ttf.o) $(LANGFILES:.lang=.lang.o) \
                $(PNGFILES:.png=.png.o) \
                $(OGGFILES:.ogg=.ogg.o) $(PCMFILES:.pcm=.pcm.o) \
                $(JPGFILES:.jpg=.jpg.o) \
                $(GIFFILES:.gif=.gif.o) \
                $(PEMFILES:.pem=.pem.o)
OFILES := $(addprefix $(BUILD)/,$(OFILES))

#---------------------------------------------------------------------------------
# Third-party libraries (all built statically, conditionally per platform)
#---------------------------------------------------------------------------------
include third_party/mbedtls.mk
include third_party/curl.mk
ifeq ($(WBL_HAS_JAVASCRIPT),1)
include third_party/quickjs.mk
WBL_THIRD_PARTY_LIBS += build/libquickjs.a
WBL_THIRD_PARTY_DEPS += build/libquickjs.a
endif

ifeq ($(WBL_HAS_HTTPS),1)
WBL_THIRD_PARTY_LIBS += $(CURL_LIB) $(MBEDTLS_LIB) $(MBEDX509_LIB) $(MBEDCRYPTO_LIB)
# MBEDTLS_PATCH_STAMP makes our app's compilation depend on submodule
# patches having been applied, so app source files see the patched
# mbedtls headers (e.g. our __has_include override in mbedtls_config.h).
WBL_THIRD_PARTY_DEPS += $(CURL_LIB) $(MBEDTLS_LIB) $(MBEDTLS_PATCH_STAMP)
endif

#---------------------------------------------------------------------------------
# Rules
#---------------------------------------------------------------------------------
all: $(TARGET_OUT)

$(BUILD):
	@mkdir -p $(BUILD)

# Compile C source. Order-only-deps on $(WBL_THIRD_PARTY_DEPS) ensure
# submodule patches (e.g. our mbedtls config-override patch) are
# applied before any of our app code that #includes the patched
# headers gets compiled.
$(BUILD)/%.o: %.c | $(BUILD) $(WBL_THIRD_PARTY_DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source (same rationale as above)
$(BUILD)/%.o: %.cpp | $(BUILD) $(WBL_THIRD_PARTY_DEPS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Binary data conversion.
#
# devkitPro's bin2o macro in base_tools outputs to $(<F).o in the
# CURRENT directory, not to our $(BUILD) output path. Define our own
# version that writes directly to $@ so the link step can find the
# resulting objects.
#
# Modern bin2s (devkitPro) emits `NAME[]` and `NAME_end[]` as linker
# symbols but defines `NAME_size` as a C++ constexpr in the generated
# .h file - NOT as a linker symbol. Our app's src/filelist.h declares
# `extern const u32 NAME_size;` expecting a real linker symbol.
#
# To bridge that, we append a small assembly stanza to each generated
# .s that exports `NAME_size` as a 4-byte .int equal to
# (NAME_end - NAME). This keeps both the legacy extern-symbol usage
# and modern bin2s's constexpr header working, with no app-side churn.
define wbl_bin2o
	@echo $(notdir $<)
	$(eval BIN2S_NAME := $(shell echo $(<F) | tr . _))
	$(SILENTCMD)bin2s -a 32 -H $(BUILD)/$(BIN2S_NAME).h $< > $(BUILD)/$(<F).s
	$(SILENTCMD)printf '\n\t.section .rodata.%s_size, "a"\n\t.balign 4\n\t.global %s_size\n%s_size:\n\t.int %s_end - %s\n' \
		$(BIN2S_NAME) $(BIN2S_NAME) $(BIN2S_NAME) $(BIN2S_NAME) $(BIN2S_NAME) \
		>> $(BUILD)/$(<F).s
	$(SILENTCMD)$(CC) -x assembler-with-cpp $(CPPFLAGS) $(ASFLAGS) -c $(BUILD)/$(<F).s -o $@
	@rm $(BUILD)/$(<F).s
endef

$(BUILD)/%.ttf.o:  %.ttf  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.lang.o: %.lang | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.png.o:  %.png  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.jpg.o:  %.jpg  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.gif.o:  %.gif  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.ogg.o:  %.ogg  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.pcm.o:  %.pcm  | $(BUILD) ; $(wbl_bin2o)
$(BUILD)/%.pem.o:  %.pem  | $(BUILD) ; $(wbl_bin2o)

# Link ELF
$(TARGET).elf: $(OFILES) $(WBL_THIRD_PARTY_DEPS)
	$(CC) $(LDFLAGS) -o $@ $(OFILES) $(WBL_THIRD_PARTY_LIBS) $(LIBS)

# Convert ELF to DOL (Wii-specific; other platforms override TARGET_OUT rule)
$(TARGET).dol: $(TARGET).elf
	elf2dol $< $@

# Clean
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol $(TARGET).map

# Deep clean - also wipes third-party builds
distclean: clean quickjs-clean mbedtls-clean curl-clean

.PHONY: all clean distclean
