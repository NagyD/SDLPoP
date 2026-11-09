#---------------------------------------------------------------------------------
# SDLPoP for Game Boy Advance (devkitARM)
#---------------------------------------------------------------------------------
ifeq ($(strip $(DEVKITARM)),)
$(error "DEVKITARM is not set. Install devkitPro/devkitARM and source devkitarm.")
endif

include $(DEVKITARM)/gba_rules

#---------------------------------------------------------------------------------
TARGET      := SDLPoP_gba
BUILD       := build
SOURCES     := src ../src
INCLUDES    := include ../src
DATA        := data
ASSETS      := assets

GAME_TITLE  := SDLPOP
GAME_CODE   := SLPP
MAKER_CODE  := PP
GAME_VER    := 0

#---------------------------------------------------------------------------------
# options for code generation
#---------------------------------------------------------------------------------
ARCH        := -mthumb -mthumb-interwork
CFLAGS      := -g -Wall -O2 -mcpu=arm7tdmi -mtune=arm7tdmi -fomit-frame-pointer \
               -ffast-math -fno-strict-aliasing \
               $(ARCH)
CFLAGS      += $(INCLUDE) -DARM9 -D__GBA__ -DGBA -DPOP_GBA

CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS     := -g $(ARCH)
LDFLAGS     := -g $(ARCH) -Wl,-Map,$(notdir $*.map) \
               -L$(CURDIR) -B$(CURDIR) \
               -Wl,--wrap=free \
               -Wl,--wrap=malloc -Wl,--wrap=calloc -Wl,--wrap=realloc \
               -Wl,--wrap=snprintf -Wl,--wrap=vsnprintf \
               -Wl,--wrap=sprintf  -Wl,--wrap=vsprintf

LIBS        := -lmm -lgba -lm
LIBDIRS     := $(LIBGBA)

# gba_rules invokes $(LD) with gcc-style flags + -specs=gba.specs, so LD must
# be the gcc driver, not bare ld.
LD := $(CC)

#---------------------------------------------------------------------------------
ifneq ($(BUILD),$(notdir $(CURDIR)))
#---------------------------------------------------------------------------------
export OUTPUT   := $(CURDIR)/$(TARGET)
export VPATH    := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(DATA),$(CURDIR)/$(dir)) \
                   $(foreach dir,$(ASSETS),$(CURDIR)/$(dir))
export DEPSDIR  := $(CURDIR)/$(BUILD)

CFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
SFILES   := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.s))) \
            $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.s)))
BINFILES := $(foreach dir,$(DATA),$(notdir $(wildcard $(dir)/*.bin)))

# Exclude SDLPoP source files that we replace or that pull in dependencies
# we can't satisfy on GBA. The rest of the upstream code compiles unchanged.
EXCLUDE_CFILES := main.c midi.c opl3.c lighting.c screenshot.c \
                  replay.c stb_vorbis.c
CFILES := $(filter-out $(EXCLUDE_CFILES),$(CFILES))

export OFILES_BIN := $(addsuffix .o,$(BINFILES))
export OFILES_SRC := $(CFILES:.c=.o) $(SFILES:.s=.o)
export OFILES     := $(OFILES_BIN) $(OFILES_SRC)
export HFILES     := $(addsuffix .h,$(subst .,_,$(BINFILES)))

export INCLUDE  := $(foreach dir,$(INCLUDES),-I$(CURDIR)/$(dir)) \
                   $(foreach dir,$(LIBDIRS),-I$(dir)/include) \
                   -I$(CURDIR)/$(BUILD)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)

.PHONY: $(BUILD) clean all

#---------------------------------------------------------------------------------
all: $(BUILD)

$(BUILD):
	@[ -d $@ ] || mkdir -p $@
	@$(MAKE) --no-print-directory -C $(BUILD) -f $(CURDIR)/Makefile

clean:
	@echo Cleaning...
	@rm -fr $(BUILD) $(TARGET).elf $(TARGET).gba $(TARGET).sav

#---------------------------------------------------------------------------------
else

DEPENDS := $(OFILES:.o=.d)

$(OUTPUT).gba   : $(OUTPUT).elf
	$(SILENTCMD)$(OBJCOPY) -O binary $< $@
	$(SILENTCMD)gbafix $@ -t$(GAME_TITLE) -c$(GAME_CODE) -m$(MAKER_CODE) -r$(GAME_VER)
	@echo built ... $(notdir $@)

# Override the default link rule: we use our own linker script that puts
# .data / .bss in EWRAM, and our own specs so the stock gba_cart.ld is not
# picked up (which would conflict and force .data into IWRAM).
$(OUTPUT).elf   : $(OFILES)
	$(SILENTMSG) linking cartridge
	$(SILENTCMD)$(LD) $(LDFLAGS) -B$(CURDIR)/.. -specs=$(CURDIR)/../gba_ewram.specs \
	    $(OFILES) $(LIBPATHS) $(LIBS) -o $@

$(OFILES_SRC)   : $(HFILES)

%_bin.h %.bin.o : %.bin
	@echo $(notdir $<)
	$(SILENTCMD)$(bin2o)

-include $(DEPENDS)

endif
#---------------------------------------------------------------------------------
