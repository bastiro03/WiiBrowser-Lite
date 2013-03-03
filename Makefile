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
# SOURCES is a list of directories containing source code
# INCLUDES is a list of directories containing extra header files
#---------------------------------------------------------------------------------
MPLAYER		:=	$(CURDIR)/source/mplayer
TARGET		:=	wiibrowser
BUILD		:=	build
SOURCES		:=	source source/html source/css source/libwiigui source/images source/fonts source/sounds \
				source/lang source/utils source/images/appbar
INCLUDES	:=	source source/mplayer

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------

CFLAGS		=	-g -O3 -Wall $(MACHDEP) $(INCLUDE)  \
				-D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -Wframe-larger-than=8192
CXXFLAGS	=	-std=gnu++0x $(CFLAGS)
LDFLAGS		=	-g $(MACHDEP) -specs=wiimc.spec -Wl

#---------------------------------------------------------------------------------
# any extra libraries we wish to link with the project
#---------------------------------------------------------------------------------
LIBS    := -lmplayerwii -lavformat -lavcodec -lswscale -lavutil \
                        -lfribidi -ljpeg -liconv -ldi -lpng -lz \
						-lcurl -lcyassl -lnetport -lasnd -lvorbisidec \
                        -lfat -lwiiuse -lwiikeyboard -lbte -logc -lfreetype \

#---------------------------------------------------------------------------------
# list of directories containing libraries, this must be the top level containing
# include and lib
#---------------------------------------------------------------------------------
LIBDIRS	:= $(PORTLIBS)


#---------------------------------------------------------------------------------
# no real need to edit anything past this point unless you need to add additional
# rules for different file extensions
#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------

export OUTPUT	:=	$(CURDIR)/$(TARGETDIR)/$(TARGET)
export VPATH	:=	$(foreach dir,$(SOURCES),$(CURDIR)/$(dir))
export DEPSDIR	:=	$(CURDIR)/$(BUILD)

#---------------------------------------------------------------------------------
# automatically build a list of object files for our project
#---------------------------------------------------------------------------------
CFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
sFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s)))
SFILES		:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.S)))
TTFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ttf)))
LANGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.lang)))
PNGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.png)))
JPGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.jpg)))
GIFFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.gif)))
OGGFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.ogg)))
PCMFILES	:=	$(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.pcm)))

#---------------------------------------------------------------------------------
# use CXX for linking C++ projects, CC for standard C
#---------------------------------------------------------------------------------
ifeq ($(strip $(CPPFILES)),)
	export LD	:=	$(CC)
else
	export LD	:=	$(CXX)
endif

export OFILES	:=	$(CPPFILES:.cpp=.o) $(CFILES:.c=.o) \
					$(sFILES:.s=.o) $(SFILES:.S=.o) \
					$(TTFFILES:.ttf=.ttf.o) $(LANGFILES:.lang=.lang.o) \
					$(PNGFILES:.png=.png.o) \
					$(OGGFILES:.ogg=.ogg.o) $(PCMFILES:.pcm=.pcm.o) \
					$(JPGFILES:.jpg=.jpg.o) \
					$(GIFFILES:.gif=.gif.o)
					
#---------------------------------------------------------------------------------
# build a list of include paths
#---------------------------------------------------------------------------------
export INCLUDE	:=	$(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
					$(foreach dir,$(LIBDIRS),-I$(dir)/include) \
					-I$(CURDIR)/$(BUILD) \
					-I$(LIBOGC_INC) -I$(PORTLIBS)/include/freetype2

#---------------------------------------------------------------------------------
# build a list of library paths
#---------------------------------------------------------------------------------
 
export LIBPATHS	:=	$(foreach dir,$(LIBDIRS),-L$(dir)/lib) \
					-L$(LIBOGC_LIB) \
				-L$(MPLAYER)/ \
				-L$(MPLAYER)/ffmpeg/libavcodec \
				-L$(MPLAYER)/ffmpeg/libavformat \
				-L$(MPLAYER)/ffmpeg/libavutil \
				-L$(MPLAYER)/ffmpeg/libswscale 

export OUTPUT	:=	$(CURDIR)/$(TARGET)
.PHONY: $(BUILD) clean

#---------------------------------------------------------------------------------
$(BUILD):
#	cd source/mplayer; $(MAKE) -f Makefile; cd ../..
	@[ -d $@ ] || mkdir -p $@
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

#---------------------------------------------------------------------------------
test:
	@make --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile


clean:
	@echo clean ...
	rm -f $(BUILD)/*.d $(BUILD)/*.h $(BUILD)/*.ii $(BUILD)/*.lst $(BUILD)/*.map \
	$(BUILD)/*.o $(BUILD)/*.s
	@rm -fr $(OUTPUT).elf $(OUTPUT).dol
#	cd source/mplayer; $(MAKE) -f Makefile clean

#---------------------------------------------------------------------------------
run:
	wiiload $(OUTPUT).dol

#---------------------------------------------------------------------------------
reload:
	wiiload -r $(OUTPUT).dol

#---------------------------------------------------------------------------------
else

DEPENDS	:=	$(OFILES:.o=.d)

#---------------------------------------------------------------------------------
# main targets
#---------------------------------------------------------------------------------
$(OUTPUT).dol: $(OUTPUT).elf
$(OUTPUT).elf: $(OFILES)

#---------------------------------------------------------------------------------
# This rule links in binary data with .ttf, .png, and .mp3 extensions
#---------------------------------------------------------------------------------
%.ttf.o : %.ttf
	@echo $(notdir $<)
	$(bin2o)

%.lang.o : %.lang
	@echo $(notdir $<)
	$(bin2o)

%.png.o : %.png
	@echo $(notdir $<)
	$(bin2o)

%.jpg.o : %.jpg
	@echo $(notdir $<)
	$(bin2o)
	
%.gif.o : %.gif
	@echo $(notdir $<)
	$(bin2o)
	
%.ogg.o : %.ogg
	@echo $(notdir $<)
	$(bin2o)

%.pcm.o : %.pcm
	@echo $(notdir $<)
	$(bin2o)

-include $(DEPENDS)

#---------------------------------------------------------------------------------
endif
#---------------------------------------------------------------------------------
