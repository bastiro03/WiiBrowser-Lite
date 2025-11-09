#---------------------------------------------------------------------------------
# Clear the implicit built in rules
#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment. export DEVKITPPC=<path to>devkitPPC")
endif

include $(DEVKITPPC)/wii_rules

# Project Name
TARGET := wiibrowserlite

# Directories
BUILD := build
SOURCES := $(shell find src -type d)
DATA := images fonts sounds certs
INCLUDES := include
VPATH := $(SOURCES) $(DATA)

#---------------------------------------------------------------------------------
# devkitPPC toolchain setup
#---------------------------------------------------------------------------------
DEVKITPPC ?= $(DEVKITPRO)/devkitPPC
PREFIX := $(DEVKITPPC)/bin/powerpc-eabi-
CC := $(PREFIX)gcc
CXX := $(PREFIX)g++
AS := $(PREFIX)as
AR := $(PREFIX)ar
LD := $(PREFIX)ld
OBJCOPY := $(PREFIX)objcopy

#---------------------------------------------------------------------------------
# Flags
#---------------------------------------------------------------------------------
# Optimization flags: -Os for size, -flto for link-time optimization
# Function/data sections for dead code elimination
CFLAGS := -Os -Wall -mrvl -mcpu=750 -meabi -mhard-float -flto \
                -ffunction-sections -fdata-sections \
                $(foreach dir,$(INCLUDES),-I$(dir))

CXXFLAGS := $(CFLAGS) -fno-exceptions -fno-rtti
LDFLAGS := -flto -Wl,-Map,$(TARGET).map -Wl,--gc-sections

LIBS := -lmplayerwii -lavformat -lavcodec -lswscale -lavutil \
				-lfribidi -ljpeg -liconv -ldi -lpng -lunrar -lzip -lsevenzip -lz \
				-lcurl -lcyassl -lnetport -lasnd -lvorbisidec \
				-lmxml -llua -lm -lfat -lwiiuse -lwiikeyboard -lbte -logc -lfreetype

# Add library search path for local libs
LDFLAGS += -L$(CURDIR)/libs/wii

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
# Rules
#---------------------------------------------------------------------------------
all: $(TARGET).dol

$(BUILD):
	@mkdir -p $(BUILD)

# Compile C source
$(BUILD)/%.o: %.c | $(BUILD)
	$(CC) $(CFLAGS) -c $< -o $@

# Compile C++ source
$(BUILD)/%.o: %.cpp | $(BUILD)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Binary data conversion
$(BUILD)/%.ttf.o: %.ttf | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.lang.o: %.lang | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.png.o: %.png | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.jpg.o: %.jpg | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.gif.o: %.gif | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.ogg.o: %.ogg | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.pcm.o: %.pcm | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

$(BUILD)/%.pem.o: %.pem | $(BUILD)
	@echo $(notdir $<)
	$(bin2o) $< $@

# Link ELF
$(TARGET).elf: $(OFILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Convert ELF to DOL
$(TARGET).dol: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

# Clean
clean:
	@echo "Cleaning..."
	@rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol $(TARGET).map

.PHONY: all clean