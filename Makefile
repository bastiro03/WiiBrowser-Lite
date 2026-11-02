.DEFAULT_GOAL := all
.SUFFIXES:

BUILD ?= build
TARGET := boot
PYTHON ?= python3
FLEX ?= flex
BISON ?= bison
HOST_GOALS := test check clean forwarder
WII_GOALS := $(filter-out $(HOST_GOALS),$(if $(MAKECMDGOALS),$(MAKECMDGOALS),all))

ifneq ($(strip $(WII_GOALS)),)
ifeq ($(strip $(DEVKITPRO)),)
$(error Set DEVKITPRO to an installed devkitPro SDK; host checks use 'make check' or 'make test')
endif
export DEVKITPPC := $(DEVKITPRO)/devkitPPC
include $(DEVKITPPC)/wii_rules

# Only enabled application code is built. Quarantined archive/GIF/BMP/editor/
# media-player implementations remain in git but are not release dependencies.
SOURCE_DIRS := src/core src/ui src/media src/filesystem src/network src/html_parser src/utils external/libwiigui external/mplayerwii
INCLUDE_DIRS := src src/core src/ui src/media src/filesystem src/network src/html_parser src/text src/utils external/libwiigui external/mplayerwii external/mplayer
CPPFILES := $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.cpp)) src/text/gettext.cpp src/text/TextPointer.cpp
CPPFILES := $(filter-out src/utils/easybmp.cpp src/utils/newlibcompat.cpp,$(CPPFILES))
CFILES := src/media/oggplayer.c src/network/mime.c src/network/urlcode.c src/html_parser/parser.c src/html_parser/entities.c src/utils/mem2_manager.c src/utils/pngu.c src/utils/timer.c
GENERATED_C := $(BUILD)/generated/css_syntax.c $(BUILD)/generated/css_lex.c
ASSETS := $(sort $(wildcard assets/fonts/*.ttf assets/images/*.png assets/images/appbar/*.png assets/sounds/*.pcm assets/lang/*.lang))
OBJECTS := $(addprefix $(BUILD)/,$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) $(addsuffix .o,$(ASSETS))) $(GENERATED_C:.c=.o)
DEPS := $(OBJECTS:.o=.d)
LIBDIRS := $(if $(PORTLIBS),$(PORTLIBS),$(DEVKITPRO)/portlibs/wii $(DEVKITPRO)/portlibs/ppc)
ELF2DOL ?= $(DEVKITPRO)/tools/bin/elf2dol

# First-party headers are quote-only. In particular <network.h> MUST resolve to
# the installed libogc header, not the repository's historical copy.
INCLUDE := $(foreach dir,$(INCLUDE_DIRS),-iquote $(CURDIR)/$(dir)) -I$(CURDIR)/$(BUILD) -I$(CURDIR)/$(BUILD)/generated \
           $(foreach dir,$(LIBDIRS),-I$(dir)/include -I$(dir)/include/freetype2) -I$(LIBOGC_INC)
COMMON_FLAGS := -O2 -g -Wall -Wextra -Wformat=2 -Werror=format-security -Werror=return-type -fdata-sections -ffunction-sections -D_DEFAULT_SOURCE $(MACHDEP) $(INCLUDE) -MMD -MP
CFLAGS := $(COMMON_FLAGS) -std=c17 -Werror=implicit-function-declaration
CXXFLAGS := $(COMMON_FLAGS) -std=gnu++17
LDFLAGS := $(MACHDEP) -Wl,--gc-sections -Wl,-Map,$(BUILD)/boot.map
LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib) -L$(LIBOGC_LIB)

# Resolve curl's *actual* TLS/compression dependencies from the target SDK.
# No fallback to mismatched legacy archives or an arbitrarily selected TLS ABI.
empty :=
space := $(empty) $(empty)
PKG_DIRS := $(subst $(space),:,$(strip $(foreach dir,$(LIBDIRS),$(dir)/lib/pkgconfig)))
CURL_LIBS ?= $(shell PKG_CONFIG_LIBDIR='$(PKG_DIRS)' pkg-config --libs --static libcurl 2>/dev/null)
FREETYPE_LIBS ?= $(shell PKG_CONFIG_LIBDIR='$(PKG_DIRS)' pkg-config --libs --static freetype2 2>/dev/null)
ifeq ($(strip $(CURL_LIBS)),)
$(error Cannot resolve target libcurl.pc; install no packages automatically. Set CURL_LIBS to the reviewed target curl/TLS link flags)
endif
ifeq ($(strip $(FREETYPE_LIBS)),)
$(error Cannot resolve target freetype2.pc. Set FREETYPE_LIBS to the target FreeType link flags including its dependencies)
endif
LIBS := $(CURL_LIBS) $(FREETYPE_LIBS) -lmxml -lpng -ljpeg -lfribidi -lvorbisidec -logg -lasnd -lfat -lwiiuse -lwiikeyboard -lbte -logc -lz -lm

.PHONY: all package
all: $(TARGET).dol

$(TARGET).elf: $(OBJECTS)
	$(CXX) $(LDFLAGS) $(OBJECTS) $(LIBPATHS) -Wl,--start-group $(LIBS) -Wl,--end-group -o $@

$(TARGET).dol: $(TARGET).elf
	$(ELF2DOL) $< $@

$(BUILD)/embedded_assets.h: $(ASSETS) scripts/build_assets.py
	@mkdir -p $(@D)
	$(PYTHON) scripts/build_assets.py headers $@ $(ASSETS)

$(BUILD)/generated/css_syntax.c $(BUILD)/generated/css_syntax.h &: src/html_parser/css_syntax.y
	@mkdir -p $(BUILD)/generated
	$(BISON) --defines=$(BUILD)/generated/css_syntax.h -o $(BUILD)/generated/css_syntax.c $<

$(BUILD)/generated/css_lex.c: src/html_parser/css_lex.l $(BUILD)/generated/css_syntax.h src/html_parser/css_lex.h
	$(FLEX) -o $@ $<

$(BUILD)/generated/%.o: $(BUILD)/generated/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.cpp $(BUILD)/embedded_assets.h $(BUILD)/generated/css_syntax.h
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.c $(BUILD)/embedded_assets.h $(BUILD)/generated/css_syntax.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/assets/%.o: assets/% scripts/build_assets.py
	@mkdir -p $(@D)
	$(PYTHON) scripts/build_assets.py embed $< $(@:.o=.S)
	$(CC) $(MACHDEP) -x assembler-with-cpp -c $(@:.o=.S) -o $@

# This creates a DEVELOPMENT bundle only, never publishes or uploads it.
package: $(TARGET).dol check
	$(PYTHON) scripts/package.py --binary $(TARGET).dol --output $(BUILD)/WiiBrowser-Lite-HBC-dev.zip

-include $(DEPS)
endif

.PHONY: test check clean forwarder
check:
	$(PYTHON) tests/check_source.py

test: check
	$(MAKE) -C tests

forwarder:
	@echo "Forwarder quarantined: fixed-RAM/USB bounds and loader provenance require separate approval." >&2
	@exit 1

clean:
	$(PYTHON) scripts/clean.py
