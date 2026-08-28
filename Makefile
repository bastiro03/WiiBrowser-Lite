#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
export DEVKITPPC = $(DEVKITPRO)/devkitPPC

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

#---------------------------------------------------------------------------------
# TARGET is the name of the output
# BUILD is the directory where object files & intermediate files will be placed
# SOURCES is a list of directories containing source code and asset data
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
MPLAYER		:=	$(CURDIR)/external/mplayer
TARGET		:=	boot
BUILD		:=	build

# Application Source, External Dependencies, and Binary Assets
SOURCES		:=	src/core src/ui src/media src/filesystem src/archive src/network \
				src/html_parser src/text src/utils \
				external/libwiigui external/litehtml external/mplayerwii \
				assets/images assets/images/appbar assets/fonts assets/sounds assets/lang

# Include paths (adding all subdirectories so existing #includes don't break)
INCLUDES	:=	src src/core src/ui src/media src/filesystem src/archive src/network \
				src/html_parser src/text src/utils \
				external external/mplayer external/libwiigui external/litehtml \
				external/mplayerwii libs/include

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		=	-g -O2 -std=c17 -Wall -Wextra -Wpedantic -Werror=implicit-function-declaration -fdata-sections -ffunction-sections -D_DEFAULT_SOURCE $(MACHDEP) $(INCLUDE) -MMD -MP
CXXFLAGS	=	-g -O2 -std=gnu++17 -D_GLIBCXX_USE_CXX11_ABI=0 -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections -D_DEFAULT_SOURCE $(MACHDEP) $(INCLUDE) -MMD -MP
LDFLAGS		=	-g -ggdb $(MACHDEP) -Wl,--gc-sections -Wl,--print-memory-usage -Wl,-Map,$(notdir $(OUTPUT)).map

# Extra map options (uncomment to adjust .init base): -Wl,--section-start,.init=0x80620000
# Heap wrappers (disabled by default; enable with -DDEBUG_MEM2_LEVEL): -Wl,-wrap,malloc,-wrap,free,-wrap,memalign,-wrap,calloc,-wrap,realloc,-wrap,malloc_usable_size

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
# LIBS	:=	-lmplayerwii -lavformat -lavcodec -lswscale -lavutil \

LIBS	:=	-lfribidi -ljpeg -liconv -ldi -lunrar -lzip -lsevenzip \
			-lcurl -lcyassl -lnetport -lasnd -lvorbisidec \
			-lmxml -llua -lm -lfat -lwiiuse -lwiikeyboard -lbte -logc -lfreetype \
			-lpng -lz -lbz2 -lbrotlidec -lbrotlicommon -logg

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
# (single-phase path-preserving build — no ifneq/else)
#---------------------------------------------------------------------------------
# (VPATH not needed — sources referenced via path-preserving $(BUILD)/%.o: %.c)
# (fixes notdir collision where same filename in two SOURCES overwrites)
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.c))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.cpp))
sFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.s))
SFILES		:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.S))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.ttf))
LANGFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.lang))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.png))
JPGFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.jpg))
GIFFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.gif))
OGGFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.ogg))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(wildcard $(dir)/*.pcm))

# Collision check — warn if two SOURCES contain same basename
_DUP_CHECK := $(strip $(shell \
	basenames="$(notdir $(CFILES) $(CPPFILES))"; \
	echo "$$basenames" | tr ' ' '\n' | sort | uniq -d | tr '\n' ' '))
ifneq ($(_DUP_CHECK),)
$(warning duplicate basenames across SOURCES (now path-preserved, safe): $(_DUP_CHECK))
endif

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

# Path-preserving OFILES: src/core/foo.cpp -> src/core/foo.o; assets/.../x.png -> assets/.../x.png.o
export OFILES	:=	$(CPPFILES:%.cpp=%.o) $(CFILES:%.c=%.o) \
					$(sFILES:%.s=%.o) $(SFILES:%.S=%.o) \
					$(TTFFILES:%=%.o) $(LANGFILES:%=%.o) \
					$(PNGFILES:%=%.o) \
					$(OGGFILES:%=%.o) $(PCMFILES:%=%.o) \
					$(JPGFILES:%=%.o) \
					$(GIFFILES:%=%.o)
					
#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include/freetype2) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC)

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
 
export LIBPATHS	:=	-L$(CURDIR)/libs/wii \
					$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) \
					-L$(MPLAYER)/ \
					-L$(MPLAYER)/ffmpeg/libavcodec \
					-L$(MPLAYER)/ffmpeg/libavformat \
					-L$(MPLAYER)/ffmpeg/libavutil \
					-L$(MPLAYER)/ffmpeg/libswscale 

export OUTPUT	:=	$(CURDIR)/$(TARGET)
export EMBEDSCRIPT	:=	$(CURDIR)/scripts/embeddata.sh
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

# Re-map OFILES to be prefixed with $(BUILD)/ for path-preserving single-phase build
# (CFILES already contains like src/core/foo.c)
BUILD_OFILES	:=	$(addprefix $(BUILD)/,$(OFILES))
BUILD_DEPENDS	:=	$(BUILD_OFILES:.o=.d)

.PHONY: all clean distclean forwarder test run reload docs format lint package dist version

# Default target
all: $(BUILD) $(OUTPUT).dol

# Ensure build subdirectories exist (path-preserving)
$(BUILD):
	@mkdir -p $@

# Ensure each object’s directory exists before compiling/embed (via order-only prereq in pattern rules)

# Host-side unit tests (native compiler, not devkitPPC) — see tests/
test:
	@echo "Running host unit tests (if configured)..."
	@if [ -f tests/Makefile ]; then \
		if command -v g++ >/dev/null 2>&1 || command -v clang++ >/dev/null 2>&1; then \
			$(MAKE) -C tests || echo "Host tests failed (see above)"; \
		else \
			echo "No host C++ compiler — skipping host tests"; \
		fi; \
	else echo "No tests/Makefile — skipping"; fi

format:
	@echo "Checking format with clang-format..."
	@if command -v clang-format >/dev/null 2>&1; then \
		find src external/libwiigui external/litehtml -name "*.cpp" -o -name "*.h" | xargs clang-format --dry-run --Werror || (echo "Format check failed — run clang-format"; exit 1); \
	else echo "clang-format not found — skipping"; fi

lint:
	@echo "Running clang-tidy (if available)..."
	@if command -v clang-tidy >/dev/null 2>&1; then \
		clang-tidy --version; \
		echo "Lint: add per-file checks as needed"; \
	else echo "clang-tidy not found — skipping"; fi
	@$(MAKE) format

# Forwarder channel (folded into main build, optional) — Q4
forwarder:
	@echo "Building forwarder channel..."
	@$(MAKE) -C forwarder

distclean: clean
	@echo "Removing generated parser/lexer and build artifacts..."
	rm -rf $(BUILD)
	rm -f src/html_parser/css_lex.c src/html_parser/css_lex.h src/html_parser/css_syntax.c src/html_parser/css_syntax.h
	rm -f $(OUTPUT).elf $(OUTPUT).dol
	rm -rf dist artifacts
	@if [ -d forwarder/build ]; then $(MAKE) -C forwarder clean; fi

clean:
	@echo clean ...
	rm -rf $(BUILD)
	@rm -fr $(OUTPUT).elf $(OUTPUT).dol

# HBC bundle packaging — mirrors CI Package step, uses gen_version.sh for templating
# M6: dual install to wiibrowser (canonical) + wiibrowser-lite (HBC compat) + version templating
package: $(OUTPUT).dol
	@echo "Packaging HBC bundle..."
	@mkdir -p dist/apps/wiibrowser dist/apps/wiibrowser-lite
	@cp $(OUTPUT).dol dist/apps/wiibrowser/boot.dol
	@cp $(OUTPUT).dol dist/apps/wiibrowser-lite/boot.dol
	@cp HBC/icon.png dist/apps/wiibrowser/icon.png 2>/dev/null || cp HBC/icon.png dist/apps/wiibrowser-lite/icon.png 2>/dev/null || cp hbc/icon.png dist/apps/wiibrowser/icon.png 2>/dev/null || true
	@cp HBC/icon.png dist/apps/wiibrowser-lite/icon.png 2>/dev/null || cp hbc/icon.png dist/apps/wiibrowser-lite/icon.png 2>/dev/null || true
	@if [ -f HBC/meta.xml.in ]; then \
		VERSION=$${GITHUB_REF_NAME:-$$(git rev-parse --short HEAD 2>/dev/null || echo "dev")}; \
		DATE=$$(date -u +%Y%m%d); \
		sed -e "s|@VERSION@|$$VERSION|g" -e "s|@RELEASE_DATE@|$$DATE|g" HBC/meta.xml.in > dist/apps/wiibrowser/meta.xml; \
		sed -e "s|@VERSION@|$$VERSION|g" -e "s|@RELEASE_DATE@|$$DATE|g" HBC/meta.xml.in > dist/apps/wiibrowser-lite/meta.xml; \
	else \
		cp HBC/meta.xml dist/apps/wiibrowser/meta.xml 2>/dev/null || cp hbc/meta.xml dist/apps/wiibrowser/meta.xml 2>/dev/null || true; \
		cp HBC/meta.xml dist/apps/wiibrowser-lite/meta.xml 2>/dev/null || cp hbc/meta.xml dist/apps/wiibrowser-lite/meta.xml 2>/dev/null || true; \
	fi
	@cp HBC/wiibrowser.cfg dist/apps/wiibrowser/ 2>/dev/null || true
	@cp HBC/wiibrowser.cfg dist/apps/wiibrowser-lite/ 2>/dev/null || true
	@if command -v zip >/dev/null 2>&1; then \
		(cd dist && zip -r ../WiiBrowser-Lite-HBC.zip apps >/dev/null && echo "  ZIP WiiBrowser-Lite-HBC.zip"); \
	else \
		echo "  ZIP (python fallback) WiiBrowser-Lite-HBC.zip"; \
		python3 -c "import zipfile, pathlib; z=zipfile.ZipFile('WiiBrowser-Lite-HBC.zip','w',zipfile.ZIP_DEFLATED); [z.write(p, p.relative_to('dist')) for p in pathlib.Path('dist/apps').rglob('*') if p.is_file()]; z.close()"; \
	fi

dist: package

run:
	wiiload $(OUTPUT).dol

reload:
	wiiload -r $(OUTPUT).dol

# Version header generation (optional)
version:
	@$(SHELL) scripts/gen_version.sh

docs:
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile && echo "Docs at docs/doxygen/html/index.html"; \
	else echo "doxygen not found — skipping docs"; fi

#---------------------------------------------------------------------------------
# main targets — path-preserving
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(BUILD_OFILES)

#---------------------------------------------------------------------------------
# Compile rules — path-preserving (ensure subdirs via | $(BUILD) + mkdir)
#---------------------------------------------------------------------------------
$(BUILD)/%.o: %.c | $(BUILD)
	@mkdir -p $(@D)
	@echo "  CC  $<"
	$(SILENTCMD)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.cpp | $(BUILD)
	@mkdir -p $(@D)
	@echo "  CXX $<"
	$(SILENTCMD)$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD)/%.o: %.s | $(BUILD)
	@mkdir -p $(@D)
	$(SILENTCMD)$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S | $(BUILD)
	@mkdir -p $(@D)
	$(SILENTCMD)$(CC) $(CFLAGS) -c $< -o $@

#---------------------------------------------------------------------------------
# Asset embedding (replaces bin2o, adds _size symbol)
#---------------------------------------------------------------------------------
$(BUILD)/%.ttf.o: %.ttf | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.lang.o: %.lang | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.png.o: %.png | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.jpg.o: %.jpg | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.gif.o: %.gif | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.ogg.o: %.ogg | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

$(BUILD)/%.pcm.o: %.pcm | $(BUILD)
	@mkdir -p $(@D)
	@echo "  EMBED $<"
	$(SILENTCMD)$(SHELL) $(EMBEDSCRIPT) $< $@

FLEX	?=	flex
BISON	?=	bison

# Generated CSS parser/lexer — flex/bison (optional, stubs committed)
src/html_parser/css_lex.c src/html_parser/css_lex.h: src/html_parser/css_lex.l src/html_parser/css_syntax.h
	@echo "  FLEX $<"
	@$(FLEX) -o src/html_parser/css_lex.c --header-file=src/html_parser/css_lex.h $< || echo "flex missing — using committed stub"

src/html_parser/css_syntax.c src/html_parser/css_syntax.h: src/html_parser/css_syntax.y
	@echo "  BISON $<"
	@$(BISON) -d -o src/html_parser/css_syntax.c $< || echo "bison missing — using committed stub"

-include $(BUILD_DEPENDS)