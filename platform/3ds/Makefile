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
WITH_GT2_BOOT ?= 0
ifeq ($(WITH_GT2_BOOT),1)
override WITH_GT2_PROBE := 1
endif
WITH_GT2_PROBE ?= 0
ifeq ($(WITH_GT2_PROBE),1)
TARGET := opengtps1-old3ds-gt2probe
BUILD := build-gt2probe
endif
export APP_TITLE := OpenGTPS1 MIPS tests
export APP_DESCRIPTION := Native guest tests (no game data)
ifeq ($(WITH_GT2_PROBE),1)
export APP_TITLE := OpenGTPS1 GT2 hash probe
export APP_DESCRIPTION := Synthetic tests and exact GT2 hash
endif
ifeq ($(WITH_GT2_BOOT),1)
TARGET := opengtps1-old3ds-bootprobe
BUILD := build-bootprobe
export APP_TITLE := OpenGTPS1 GT2 GPU probe
export APP_DESCRIPTION := Native GP1 and original GT2 CdInit entry
endif
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
                $(CURDIR)/../../native/runtime \
                $(CURDIR)/../../native/tests/recompiled \
                $(CURDIR)/../../native/tests \
                $(CURDIR)/../../generated/old3ds \
                $(CURDIR)/../../native/src
export DEPSDIR := $(CURDIR)/$(BUILD)
export OFILES := main.o host.o stdio_files.o geometry_stitcher.o guest.o ps1_interrupts.o ps1_dma.o ps1_gpu.o startup_services.o mips_suite.o boot_runtime_suite.o interrupt_tests.o bios_tests.o dma_tests.o gpu_tests.o
ifeq ($(WITH_GT2_PROBE),1)
ifeq ($(wildcard ../../generated/old3ds/gt2_probe.cpp),)
$(error Run python tools/prepare_native_gt2_probe.py with your supported disc first)
endif
export OFILES += gt2_probe.o
else
export OFILES += gt2_probe_stub.o
endif
ifeq ($(WITH_GT2_BOOT),1)
ifeq ($(wildcard ../../generated/old3ds/gt2_boot.cpp),)
$(error Run python tools/prepare_native_gt2_boot.py with your supported disc first)
endif
export OFILES += gt2_boot.o
else
export OFILES += boot_probe_stub.o
endif
export INCLUDE := -I$(CURDIR)/../../native/include \
                  -I$(CURDIR)/../../native/platform/3ds \
                  $(foreach dir,$(LIBDIRS),-isystem $(dir)/include)
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
