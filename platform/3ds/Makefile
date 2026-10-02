# Run from a devkitPro shell: make -C platform/3ds
.SUFFIXES:
.DEFAULT_GOAL := all
ifeq ($(strip $(DEVKITARM)),)
$(error DEVKITARM is unset; install devkitPro 3ds-dev and use its shell)
endif
ifeq ($(strip $(DEVKITPRO)),)
$(error DEVKITPRO is unset; install devkitPro 3ds-dev and use its shell)
endif
include $(DEVKITARM)/3ds_rules

TARGET := opengtps1-old3ds
BUILD := build
export APP_TITLE := OpenGTPS1 Old 3DS
export APP_DESCRIPTION := Native platform bootstrap (no game data)
export APP_AUTHOR := OpenGTPS1 contributors

# ARM11 baseline, shared by original 3DS/2DS. No New 3DS switches.
ARCH := -march=armv6k -mtune=mpcore -mfloat-abi=hard -mtp=soft
CFLAGS := -g -O2 -Wall -Wextra -Wpedantic -mword-relocations \
          -ffunction-sections -fdata-sections $(ARCH)
CFLAGS += $(INCLUDE) -D__3DS__
CXXFLAGS := $(CFLAGS) -std=gnu++17 -fno-rtti -fno-exceptions
LDFLAGS = -specs=3dsx.specs -g $(ARCH) -Wl,--gc-sections,-Map,$(OUTPUT).map
LIBS := -lcitro3d -lctru -lm
LIBDIRS := $(CTRULIB)

# Explicit source selection: never glob native/src (which contains D3D11).
# geometry_stitcher is existing portable code, compiled unchanged for ARM.
ifneq ($(notdir $(CURDIR)),$(BUILD))
export TOPDIR := $(CURDIR)
export OUTPUT := $(CURDIR)/$(TARGET)
export VPATH := $(CURDIR)/../../native/platform/3ds \
                $(CURDIR)/../../native/platform/common \
                $(CURDIR)/../../native/src
export DEPSDIR := $(CURDIR)/$(BUILD)
export OFILES := main.o host.o stdio_files.o geometry_stitcher.o
export INCLUDE := -I$(CURDIR)/../../native/include \
                  -I$(CURDIR)/../../native/platform/3ds \
                  $(foreach dir,$(LIBDIRS),-I$(dir)/include)
export LIBPATHS := $(foreach dir,$(LIBDIRS),-L$(dir)/lib)
export LD := $(CXX)
export _3DSXDEPS := $(OUTPUT).smdh
export _3DSXFLAGS := --smdh=$(OUTPUT).smdh
export APP_ICON := $(CTRULIB)/default_icon.png

.PHONY: all
all: $(BUILD)
	$(MAKE) --no-print-directory -C $(BUILD) -f $(TOPDIR)/Makefile

$(BUILD):
	mkdir -p $@
else
.PHONY: all
all: $(OUTPUT).3dsx
$(OUTPUT).3dsx: $(OUTPUT).elf $(_3DSXDEPS)
$(OUTPUT).elf: $(OFILES)
-include $(DEPSDIR)/*.d
endif
