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
# Exclusions:
# - src/archiveoperations/: legacy libunrar+libsevenzip wrappers that
#   don't compile against current unrarlib headers (TODO: migrate to
#   submodules). Not needed for Wikipedia browsing. Filter so the
#   directory's .cpp files don't appear in CPPFILES.
SOURCES := $(filter-out src/archiveoperations, $(shell find src -type d))

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
DATA := images images/appbar fonts sounds certs
INCLUDES := include

VPATH := $(SOURCES) $(DATA)

#---------------------------------------------------------------------------------
# Common cross-platform feature flags
#---------------------------------------------------------------------------------
WBL_CFLAGS := \
    -DWBL_PLATFORM_$(shell echo $(WBL_PLATFORM) | tr a-z A-Z) \
    -Iinclude

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
CFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
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

# Binary data conversion (Wii uses bin2o macro from wii_rules)
$(BUILD)/%.ttf.o: %.ttf | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.lang.o: %.lang | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.png.o: %.png | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.jpg.o: %.jpg | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.gif.o: %.gif | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.ogg.o: %.ogg | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.pcm.o: %.pcm | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

$(BUILD)/%.pem.o: %.pem | $(BUILD)
	@echo $(notdir $<)
	$(bin2o)

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
