# Nintendo DS platform build configuration
#
# Target: Nintendo DS (ARM9 @ 67 MHz + ARM7 @ 33 MHz, 4 MB RAM)
# Toolchain: devkitARM + libnds + dswifi
# Precedent: Bunjalloo browser already runs mbedTLS on DS successfully.
#
# Status: PORT IN PROGRESS - build system wired, but the browser's
# platform abstraction for DS (video, input, filesystem) is not yet
# implemented in src/. This Makefile exists so third_party/ builds
# can be exercised cross-platform.

DEVKITARM ?= $(DEVKITPRO)/devkitARM

ifeq ($(strip $(DEVKITARM)),)
$(error "Please set DEVKITARM in your environment. export DEVKITARM=<path to>devkitARM")
endif

include $(DEVKITARM)/ds_rules

# Toolchain - gcc-ar / gcc-ranlib for LTO across static archives
PREFIX  := $(DEVKITARM)/bin/arm-none-eabi-
CC      := $(PREFIX)gcc
CXX     := $(PREFIX)g++
AS      := $(PREFIX)as
AR      := $(PREFIX)gcc-ar
LD      := $(PREFIX)ld
RANLIB  := $(PREFIX)gcc-ranlib
OBJCOPY := $(PREFIX)objcopy

LIBNDS_INC  ?= $(DEVKITPRO)/libnds/include
LIBNDS_LIB  ?= $(DEVKITPRO)/libnds/lib
PORTLIBS_INC ?= $(DEVKITPRO)/portlibs/nds/include
PORTLIBS_LIB ?= $(DEVKITPRO)/portlibs/nds/lib

CURL_HOST_TRIPLET := arm-none-eabi

# Feature matrix for vanilla DS - matches Bunjalloo's footprint:
#   HTTPS yes (mbedTLS, tight cipher set, 4KB records)
#   JS    no  (1MB+ QuickJS heap won't fit in 4MB)
WBL_HAS_JAVASCRIPT := 0
WBL_HAS_HTTPS      := 1

# -mthumb picks the 16-bit instruction set for density. The ARM9 is fine
# with ARM-mode code too, but Thumb cuts .text roughly in half for a
# small perf hit on compute-light code (which most UI/browser code is).
#
# -mcpu=arm9tdmi because libnds targets the ARM9 core of the NDS.
CFLAGS := -D_NDS -DARM9 -Os -Wall \
                -mcpu=arm946e-s+nofp -mthumb -mthumb-interwork -ffast-math \
                -flto -ffunction-sections -fdata-sections \
                -Wno-implicit-function-declaration \
                -Wno-incompatible-pointer-types \
                -Wno-int-conversion \
                -Wno-builtin-declaration-mismatch \
                -I$(LIBNDS_INC) \
                -I$(PORTLIBS_INC) \
                $(foreach dir,$(SOURCES),-I$(dir))

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti

LDFLAGS := -specs=ds_arm9.specs -mthumb -mthumb-interwork \
                -flto -fuse-linker-plugin -flto-partition=one \
                -Wl,-Map,$(TARGET).map -Wl,--gc-sections \
                -L$(LIBNDS_LIB) -L$(PORTLIBS_LIB)

LIBS := -lfat -lnds9 -lm

TARGET_OUT := $(TARGET).nds

export WBL_HAS_JAVASCRIPT WBL_HAS_HTTPS
