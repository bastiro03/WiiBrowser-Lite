# Wii platform build configuration
#
# Targets: Nintendo Wii (PowerPC 750CL @ 729 MHz, 88MB RAM)
# Toolchain: devkitPPC + libogc

DEVKITPPC ?= $(DEVKITPRO)/devkitPPC

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

# Toolchain - use the GCC wrappers for ar/ranlib so LTO bitcode in
# static archives stays indexed across the final link. Without
# gcc-ar/gcc-ranlib, the linker plugin can't find LTO symbols in
# archive members and silently drops to non-LTO for them.
PREFIX  := $(DEVKITPPC)/bin/powerpc-eabi-
CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)gcc-ar
LD      := $(PREFIX)ld
RANLIB  := $(PREFIX)gcc-ranlib
OBJCOPY := $(PREFIX)objcopy

# Curl --host triplet
CURL_HOST_TRIPLET := powerpc-eabi

# libogc / portlibs paths
LIBOGC_INC ?= $(DEVKITPRO)/libogc/include
LIBOGC_LIB ?= $(DEVKITPRO)/libogc/lib/wii
PORTLIBS_INC ?= $(DEVKITPRO)/portlibs/ppc/include
PORTLIBS_LIB ?= $(DEVKITPRO)/portlibs/ppc/lib

# Feature capability matrix for Wii
WBL_HAS_JAVASCRIPT := 1
WBL_HAS_HTTPS      := 1

# Compiler flags - aggressive size + LTO + dead code elimination
CFLAGS := -DGEKKO -Os -Wall -mrvl -mcpu=750 -meabi -mhard-float -flto \
                -ffunction-sections -fdata-sections \
                -Wno-implicit-function-declaration \
                -Wno-incompatible-pointer-types \
                -Wno-int-conversion \
                -Wno-builtin-declaration-mismatch \
                -I$(LIBOGC_INC) \
                -I$(PORTLIBS_INC) \
                $(foreach dir,$(SOURCES),-I$(dir))

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti

# Final-link flags. Key LTO bits:
#   -flto                  turn on the gcc driver's LTO pass
#   -fuse-linker-plugin    enable the linker plugin (gold/lld/bfd+plugin)
#                          so the linker can read LTO IR from static libs
#   -flto-partition=one    single-partition whole-program optimization
#                          (default is 'balanced' which parallelizes).
#                          With WPO the compiler can inline/dead-strip
#                          across every translation unit in one go.
#   -Wl,--gc-sections      drop unreferenced .text/.data (matches the
#                          -ffunction-sections/-fdata-sections we used
#                          during compile)
LDFLAGS := -DGEKKO -mrvl -mcpu=750 -meabi -mhard-float \
                -flto -fuse-linker-plugin -flto-partition=one \
                -Wl,-Map,$(TARGET).map -Wl,--gc-sections \
                -L$(CURDIR)/libs/wii -L$(LIBOGC_LIB) -L$(PORTLIBS_LIB)

# Wii-specific libraries (libcurl/cyassl removed - now built from third_party)
LIBS := -lmplayerwii -lavformat -lavcodec -lswscale -lavutil \
        -lfribidi -ljpeg -liconv -ldi -lpng -lunrar -lzip -lsevenzip -lz \
        -lnetport -lasnd -lvorbisidec \
        -lmxml -lm -lfat -lwiiuse -lwiikeyboard -lbte -logc -lfreetype -lexif

# Output: .dol file for Wii
TARGET_OUT := $(TARGET).dol

export WBL_HAS_JAVASCRIPT WBL_HAS_HTTPS
