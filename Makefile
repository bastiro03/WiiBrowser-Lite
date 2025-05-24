#---------------------------------------------------------------------------------
# Wii Homebrew Makefile Template
#
# A general-purpose Makefile for Wii homebrew development using devkitPPC.
#
# Features:
# - Automatic detection of .c, .cpp, .S source files.
# - Automatic detection and embedding of .png, .pcm, .ttf, .bin asset files.
# - Separate build and output directories.
# - ELF to DOL conversion.
# - wiiload integration for easy testing.
# - Configurable project settings.
# - Well-commented for clarity.
#---------------------------------------------------------------------------------

#---------------------------------------------------------------------------------
# Project Configuration
#---------------------------------------------------------------------------------
# TARGET_ELF: Name of the intermediate ELF file.
# TARGET_DOL: Name of the final DOL file (this is what the Wii runs).
# Default names are based on the current directory name.
# Uncomment and set these for specific names if desired.
# TARGET_ELF   := my_awesome_game.elf
# TARGET_DOL   := my_awesome_game.dol
TARGET_ELF   ?= $(notdir $(CURDIR)).elf
TARGET_DOL   ?= boot.dol # Often homebrew is expected to be boot.dol in an apps/app_name/ folder

# PROGRAM_TITLE: Used by some tools or for display (optional).
# PROGRAM_TITLE := "My Awesome Game"

# OUTPUT_DIR: Directory for the final DOL and other distributable files (e.g., meta.xml, icon.png).
OUTPUT_DIR   := output

# BUILD_DIR: Directory for intermediate object files and the ELF file.
BUILD_DIR    := build

#---------------------------------------------------------------------------------
# Source and Asset Files
#---------------------------------------------------------------------------------
# SOURCE_DIRS: List of directories containing your C/C++/Assembly source code.
# Default is 'source'. Add more directories if your project is structured differently.
# Example: SOURCE_DIRS := src frontend backend
SOURCE_DIRS  := source

# ASSET_DIRS: List of directories containing your binary assets (images, sounds, fonts, etc.).
# Default is 'assets'. Add more directories if needed.
# Example: ASSET_DIRS := assets/images assets/sounds
ASSET_DIRS   := assets

# Automatically find all .c, .cpp, and .S (Assembly) files in SOURCE_DIRS.
C_FILES      := $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.c))
CPP_FILES    := $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.cpp))
S_FILES      := $(foreach dir,$(SOURCE_DIRS),$(wildcard $(dir)/*.S))

# Automatically find all asset files in ASSET_DIRS.
PNG_ASSETS   := $(foreach dir,$(ASSET_DIRS),$(wildcard $(dir)/*.png))
PCM_ASSETS   := $(foreach dir,$(ASSET_DIRS),$(wildcard $(dir)/*.pcm))
TTF_ASSETS   := $(foreach dir,$(ASSET_DIRS),$(wildcard $(dir)/*.ttf))
BIN_ASSETS   := $(foreach dir,$(ASSET_DIRS),$(wildcard $(dir)/*.bin)) # For generic binary data

# Generate object file names, placing them in the BUILD_DIR.
# $(notdir ...) gets the filename without the path.
# $(patsubst ...) changes the extension and prepends BUILD_DIR.
OBJS_C       := $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(C_FILES)))
OBJS_CPP     := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(CPP_FILES)))
OBJS_S       := $(patsubst %.S,$(BUILD_DIR)/%.o,$(notdir $(S_FILES)))

# Generate object file names for assets, placing them in BUILD_DIR.
# e.g., an asset 'assets/image.png' becomes 'build/image.png.o'
OBJS_PNG_ASSETS := $(patsubst %.png,$(BUILD_DIR)/%.png.o,$(notdir $(PNG_ASSETS)))
OBJS_PCM_ASSETS := $(patsubst %.pcm,$(BUILD_DIR)/%.pcm.o,$(notdir $(PCM_ASSETS)))
OBJS_TTF_ASSETS := $(patsubst %.ttf,$(BUILD_DIR)/%.ttf.o,$(notdir $(TTF_ASSETS)))
OBJS_BIN_ASSETS := $(patsubst %.bin,$(BUILD_DIR)/%.bin.o,$(notdir $(BIN_ASSETS)))

OBJECTS      := $(OBJS_C) $(OBJS_CPP) $(OBJS_S) \
                $(OBJS_PNG_ASSETS) $(OBJS_PCM_ASSETS) $(OBJS_TTF_ASSETS) $(OBJS_BIN_ASSETS)

# VPATH: Tells make where to find the source and asset files based on their basenames.
VPATH        := $(SOURCE_DIRS) $(ASSET_DIRS)

#---------------------------------------------------------------------------------
# DevkitPPC Toolchain Configuration
#---------------------------------------------------------------------------------
# Ensure DEVKITPPC environment variable is set. This points to your devkitPPC installation.
ifeq ($(strip $(DEVKITPPC)),)
$(error "DEVKITPPC environment variable is not set. Please set it to your devkitPPC directory (e.g., /opt/devkitpro/devkitPPC or c:/devkitPro/devkitPPC).")
endif

include $(DEVKITPPC)/wii_rules

# Toolchain prefix and executables.
PREFIX       := $(DEVKITPPC)/bin/powerpc-eabi-
CC           := $(PREFIX)gcc
CXX          := $(PREFIX)g++
AS           := $(PREFIX)as
LD           := $(CC) # Use GCC as the linker driver; it calls ld with correct system libs.
OBJCOPY      := $(PREFIX)objcopy
STRIP        := $(PREFIX)strip
NM           := $(PREFIX)nm # Useful for debugging symbols
BIN2O        := $(DEVKITPPC)/bin/bin2o # Tool to convert binary files to object files

# WIILOAD: Utility to send DOL files to the Wii over network.
# Assumes wiiload is in your PATH. If not, specify the full path.
# Example: WIILOAD := $(DEVKITPRO)/tools/bin/wiiload
WIILOAD      := wiiload

# WII_IP: IP address of your Wii. Set this here or pass it as a command-line argument.
# Example: make run WII_IP=192.168.1.100
WII_IP       :=

#---------------------------------------------------------------------------------
# Build Flags
#---------------------------------------------------------------------------------
# ARCH_FLAGS: Flags specific to the Wii's PowerPC architecture (Broadway processor).
# -mcpu=750: Broadway is a 750CL.
# -meabi: Use Embedded Application Binary Interface.
# -mhard-float: Use hardware floating-point unit.
# -DHW_RVL: Define HW_RVL (Revolution, Wii's codename) for libogc.
ARCH_FLAGS   := -mcpu=750 -meabi -mhard-float -DHW_RVL

# INCLUDE_DIRS: Directories to search for header files.
# $(DEVKITPRO)/libogc/include is essential for libogc.
# $(DEVKITPRO)/portlibs/ppc/include is for ported libraries.
INCLUDE_DIRS := -Iinclude \
                -I$(DEVKITPRO)/libogc/include \
                -I$(DEVKITPRO)/portlibs/ppc/include
# Add your project's include directories here, e.g., -I$(SOURCE_DIRS)/my_module

# LIB_DIRS: Directories to search for libraries.
# $(DEVKITPRO)/libogc/lib/wii for libogc.
# $(DEVKITPRO)/portlibs/ppc/lib for ported libraries.
LIB_DIRS     := -L$(DEVKITPRO)/libogc/lib/wii \
                -L$(DEVKITPRO)/portlibs/ppc/lib
# Add your project's library directories here.

# LIBS: Libraries to link against.
# -logc: Core OGC library.
# -lm: Math library.
# Common optional libraries: -lwiiuse (Wiimote), -lbte (Bluetooth), -lfat (FAT FS)
# -lsysbase (low level access, usually included by -logc)
LIBS         := -logc -lm
# Add other libraries: e.g., LIBS += -lwiiuse -lbte -lfat

# COMMON_FLAGS: Flags used by both C and C++ compilers.
# -g: Include debugging information.
# -Wall: Enable all warnings.
# -O2: Optimization level 2 (good balance of speed and size). Use -Os for size.
# -ffunction-sections -fdata-sections: Useful for linker garbage collection.
COMMON_FLAGS := $(ARCH_FLAGS) -g -Wall -O2 -ffunction-sections -fdata-sections

# CFLAGS: Flags for C compilation.
CFLAGS       := $(COMMON_FLAGS) $(INCLUDE_DIRS)

# CXXFLAGS: Flags for C++ compilation.
# -fno-rtti: Disable Run-Time Type Information (saves space).
# -fno-exceptions: Disable C++ exceptions (saves space, common in embedded).
# -std=c++11: Set C++ standard. Adjust as needed (e.g., c++14, c++17).
CXXFLAGS     := $(COMMON_FLAGS) $(INCLUDE_DIRS) -fno-rtti -fno-exceptions -std=c++11

# ASFLAGS: Flags for Assembly compilation.
ASFLAGS      := $(ARCH_FLAGS) -g

# LDFLAGS: Flags for the linker.
# -Wl,-Map,...: Generate a map file (useful for debugging sizes and layout).
# -Wl,--gc-sections: Linker garbage collection (removes unused sections if -ffunction-sections/-fdata-sections used).
LDFLAGS      := -g $(MACHDEP) $(ARCH_FLAGS) $(LIB_DIRS) $(LIBS) -Wl,-Map,$(BUILD_DIR)/$(TARGET_ELF:.elf=.map) -Wl,--gc-sections

#---------------------------------------------------------------------------------
# Build Targets
#---------------------------------------------------------------------------------
# Phony targets are not actual files.
.PHONY: all clean run dol elf dirs help

# Default target: Build everything.
all: dirs $(OUTPUT_DIR)/$(TARGET_DOL)

# Target to build only the DOL file (implies ELF).
dol: dirs $(OUTPUT_DIR)/$(TARGET_DOL)

# Target to build only the ELF file.
elf: dirs $(BUILD_DIR)/$(TARGET_ELF)

# Create output and build directories if they don't exist.
# The '-' before mkdir suppresses errors if directories already exist.
# @ suppresses echoing of the command.
dirs:
	@echo "Creating directories..."
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(BUILD_DIR)

# Link object files to create the ELF file.
$(BUILD_DIR)/$(TARGET_ELF): $(OBJECTS)
	@echo "Linking ELF: $@"
	$(LD) $(OBJECTS) $(LDFLAGS) -o $@

# Convert the ELF file to DOL format.
$(OUTPUT_DIR)/$(TARGET_DOL): $(BUILD_DIR)/$(TARGET_ELF)
	@echo "Converting ELF to DOL: $@"
	$(OBJCOPY) -O binary $< $@
	@echo "Stripping ELF (optional, for symbols in GDB): $(BUILD_DIR)/$(TARGET_ELF).stripped"
	@$(STRIP) --strip-unneeded -R .comment -R .GCC.command.line -R .note.GNU-stack $(BUILD_DIR)/$(TARGET_ELF) -o $(BUILD_DIR)/$(TARGET_ELF).stripped
	@echo "Build complete: $(OUTPUT_DIR)/$(TARGET_DOL)"

#---------------------------------------------------------------------------------
# Compilation Rules for Source Code
#---------------------------------------------------------------------------------
# These are pattern rules for compiling source files into object files in BUILD_DIR.

# Compile C files (.c -> .o)
# $< is the first prerequisite (the .c file).
# $@ is the target (the .o file).
$(BUILD_DIR)/%.o: %.c
	@echo "Compiling C: $<"
	@$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ files (.cpp -> .o)
$(BUILD_DIR)/%.o: %.cpp
	@echo "Compiling C++: $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

# Compile Assembly files (.S -> .o)
$(BUILD_DIR)/%.o: %.S
	@echo "Assembling: $<"
	@$(AS) $(ASFLAGS) $< -o $@

#---------------------------------------------------------------------------------
# Compilation Rules for Assets (Embedding into executable)
#---------------------------------------------------------------------------------
# These rules use bin2o to convert binary assets into object files,
# which are then linked into the executable.
#
# Accessing embedded data in C/C++:
# If you have an asset, e.g., 'assets/my_image.png', after compilation,
# you can access its data in C/C++ code like this:
#
# extern const unsigned char _binary_my_image_png_start[]; // Note: no size in array
# extern const unsigned char _binary_my_image_png_end[];
# extern const int _binary_my_image_png_size; // This is an int, not a pointer
#
# const void *my_image_data = _binary_my_image_png_start;
# unsigned int my_image_size = (unsigned int)&_binary_my_image_png_size;
# // Or, size can be calculated as: (_binary_my_image_png_end - _binary_my_image_png_start)
#
# The symbol name is derived from the asset's filename (dots and slashes replaced by underscores).
# For 'assets/textures/player.png', symbols would be like _binary_player_png_start etc.
# (bin2o uses the basename of the input file for symbol generation).
#---------------------------------------------------------------------------------

# Embed PNG files (.png -> .png.o)
$(BUILD_DIR)/%.png.o: %.png
	@echo "Embedding PNG: $<"
	@$(BIN2O) $< $@

# Embed PCM audio files (.pcm -> .pcm.o)
$(BUILD_DIR)/%.pcm.o: %.pcm
	@echo "Embedding PCM: $<"
	@$(BIN2O) $< $@

# Embed TTF font files (.ttf -> .ttf.o)
$(BUILD_DIR)/%.ttf.o: %.ttf
	@echo "Embedding TTF: $<"
	@$(BIN2O) $< $@

# Embed generic binary files (.bin -> .bin.o)
$(BUILD_DIR)/%.bin.o: %.bin
	@echo "Embedding BIN: $<"
	@$(BIN2O) $< $@

#---------------------------------------------------------------------------------
# Cleaning Target
#---------------------------------------------------------------------------------
clean:
	@echo "Cleaning build files..."
	@rm -rf $(BUILD_DIR)
	@rm -rf $(OUTPUT_DIR)
	# Optional: remove specific files if they are not in BUILD_DIR or OUTPUT_DIR
	# @rm -f $(TARGET_ELF) $(TARGET_DOL) *.o *.elf *.dol *.map
	@echo "Clean complete."

#---------------------------------------------------------------------------------
# Run/Deploy Target
#---------------------------------------------------------------------------------
# Sends the DOL file to the Wii using wiiload.
run: $(OUTPUT_DIR)/$(TARGET_DOL)
ifeq ($(strip $(WII_IP)),)
	@echo "Sending $(TARGET_DOL) via wiiload (autodiscovery or USBGecko)..."
	@$(WIILOAD) $(OUTPUT_DIR)/$(TARGET_DOL)
	@echo "If wiiload fails, ensure your Wii is ready and accessible,"
	@echo "or set WII_IP in Makefile, or pass as argument: make run WII_IP=your_wii_ip"
else
	@echo "Sending $(TARGET_DOL) to Wii at $(WII_IP)..."
	@$(WIILOAD) -u $(WII_IP) $(OUTPUT_DIR)/$(TARGET_DOL)
endif
	@echo "DOL sent."

#---------------------------------------------------------------------------------
# Optional: Assets for Homebrew Channel (icon.png, meta.xml)
#---------------------------------------------------------------------------------
# This section is for copying specific files like icon.png and meta.xml
# to the output directory, typically for Homebrew Channel presentation.
# This is different from embedding game assets directly into the DOL.
#
# HBC_ASSETS_DIR   := . # or data, or wherever your icon.png/meta.xml are
# ICON_FILE    := icon.png
# META_FILE    := meta.xml
#
# # Add these to the 'all' target dependencies if you use them:
# # all: dirs $(OUTPUT_DIR)/$(TARGET_DOL) $(OUTPUT_DIR)/$(ICON_FILE) $(OUTPUT_DIR)/$(META_FILE)
#
# $(OUTPUT_DIR)/$(ICON_FILE): $(HBC_ASSETS_DIR)/$(ICON_FILE)
# 	@echo "Copying $< to $(OUTPUT_DIR)/"
# 	@cp $< $(OUTPUT_DIR)/
#
# $(OUTPUT_DIR)/$(META_FILE): $(HBC_ASSETS_DIR)/$(META_FILE)
# 	@echo "Copying $< to $(OUTPUT_DIR)/"
# 	@cp $< $(OUTPUT_DIR)/
#
# # And ensure they are cleaned if OUTPUT_DIR is not entirely removed:
# # (Handled if OUTPUT_DIR is removed by clean target)
# # @rm -f $(OUTPUT_DIR)/$(ICON_FILE) $(OUTPUT_DIR)/$(META_FILE)

#---------------------------------------------------------------------------------
# Help Target
#---------------------------------------------------------------------------------
help:
	@echo "Wii Homebrew Makefile Targets:"
	@echo "  all         - Build the project and create $(OUTPUT_DIR)/$(TARGET_DOL) (default)."
	@echo "  dol         - Build only the DOL file: $(OUTPUT_DIR)/$(TARGET_DOL)."
	@echo "  elf         - Build only the ELF file: $(BUILD_DIR)/$(TARGET_ELF)."
	@echo "  clean       - Remove all generated files from $(BUILD_DIR) and $(OUTPUT_DIR)."
	@echo "  run         - Build and send $(TARGET_DOL) to a Wii via wiiload."
	@echo "                Set WII_IP in Makefile or pass as argument:"
	@echo "                e.g., make run WII_IP=192.168.1.100"
	@echo "  dirs        - Create $(BUILD_DIR) and $(OUTPUT_DIR) if they don't exist."
	@echo "  help        - Show this help message."
	@echo ""
	@echo "Key Variables (edit in Makefile as needed):"
	@echo "  TARGET_DOL    - Final DOL filename (default: $(TARGET_DOL))."
	@echo "  TARGET_ELF    - Intermediate ELF filename (default: $(TARGET_ELF))."
	@echo "  SOURCE_DIRS   - Directories containing C/C++/S source files (default: '$(SOURCE_DIRS)')."
	@echo "  ASSET_DIRS    - Directories containing .png, .pcm, .ttf, .bin assets for embedding (default: '$(ASSET_DIRS)')."
	@echo "  INCLUDE_DIRS  - Additional directories for header files."
	@echo "  LIB_DIRS      - Additional directories for libraries."
	@echo "  LIBS          - Additional libraries to link (e.g., -lwiiuse)."
	@echo "  WII_IP        - IP address of the Wii for 'make run'."
	@echo ""
	@echo "Prerequisites:"
	@echo "  - devkitPPC installed (with DEVKITPRO environment variable set)."
	@echo "  - DEVKITPPC environment variable pointing to devkitPPC installation."
	@echo "  - wiiload in PATH or WIILOAD variable set for 'make run'."
	@echo ""
	@echo "Using Embedded Assets:"
	@echo "  Assets from ASSET_DIRS (e.g., 'assets/image.png') are converted to object files."
	@echo "  Access them in C/C++ code using symbols like:"
	@echo "    extern const unsigned char _binary_image_png_start[];"
	@echo "    extern const unsigned char _binary_image_png_end[];"
	@echo "    extern const int _binary_image_png_size;"
	@echo "  (Replace 'image_png' with your asset's filename, dots replaced by underscores)."


# Set the default goal for 'make' without arguments.
.DEFAULT_GOAL := all