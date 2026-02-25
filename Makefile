##############################################################################
# Build global options
# NOTE: Can be overridden externally.
#

# Compiler options here.
ifeq ($(USE_OPT),)
  USE_OPT = -O2 -ggdb -fomit-frame-pointer -falign-functions=16
endif

# C specific options here (added to USE_OPT).
ifeq ($(USE_COPT),)
  USE_COPT = 
endif

# C++ specific options here (added to USE_OPT).
ifeq ($(USE_CPPOPT),)
  USE_CPPOPT = -fno-rtti -std=c++20 -fno-rtti -fno-exceptions
endif

# Enable this if you want the linker to remove unused code and data.
ifeq ($(USE_LINK_GC),)
  USE_LINK_GC = yes
endif

# Linker extra options here.
ifeq ($(USE_LDOPT),)
  USE_LDOPT = -lm
endif

# Enable this if you want link time optimizations (LTO).
ifeq ($(USE_LTO),)
  USE_LTO = yes
endif

# Enable this if you want to see the full log while compiling.
ifeq ($(USE_VERBOSE_COMPILE),)
  USE_VERBOSE_COMPILE = no
endif

# If enabled, this option makes the build process faster by not compiling
# modules not used in the current configuration.
ifeq ($(USE_SMART_BUILD),)
  USE_SMART_BUILD = yes
endif


# protobuf
PROTOC = protoc
PROTO_DIR = proto
PROTO_GEN_DIR = build/generated/proto
PROTO_GEN_DIR_PYTHON = python/generated
EMBEDDED_PROTO_DIR = $(shell pwd)/EmbeddedProto

PROTO_FILES_PREFIXED = $(wildcard $(PROTO_DIR)/*.proto)
PROTO_FILES = $(PROTO_FILES_PREFIXED:proto/%=%)
PROTO_HDR := $(PROTO_FILES:%.proto=$(PROTO_GEN_DIR)/%.h) 

PROTO_PY := $(PROTO_FILES:%.proto=$(PROTO_GEN_DIR_PYTHON)/%_pb2.py) 
EMBEDDED_PROTO_SRC := $(wildcard ./EmbeddedProto/src/*.cpp)
EMBEDDED_PROTO_OBJS := $(EMBEDDED_PROTO_SRC:%.cpp=$(OBJECT_DIR)/%.o)



#
# Build global options
##############################################################################

##############################################################################
# Architecture or project specific options
#

GCC_DIAG =  -Werror -Wno-error=unused-variable -Wno-error=format \
            -Wno-error=unused-function \
            -Wunused -Wpointer-arith \
            -Werror=sign-compare \
            -Wshadow -Wparentheses -fmax-errors=5 \
            -ftrack-macro-expansion=2 -Wstrict-overflow=2

# Stack size to be allocated to the Cortex-M process stack. This stack is
# the stack used by the main() thread.
ifeq ($(USE_PROCESS_STACKSIZE),)
  USE_PROCESS_STACKSIZE = 0x400
endif

# Stack size to the allocated to the Cortex-M main/exceptions stack. This
# stack is used for processing interrupts and exceptions.
ifeq ($(USE_EXCEPTIONS_STACKSIZE),)
  USE_EXCEPTIONS_STACKSIZE = 0x400
endif

# Enables the use of FPU (no, softfp, hard).
ifeq ($(USE_FPU),)
  USE_FPU = no
endif

# FPU-related options.
ifeq ($(USE_FPU_OPT),)
  USE_FPU_OPT = -mfloat-abi=$(USE_FPU) -mfpu=fpv4-sp-d16
endif

#
# Architecture or project specific options
##############################################################################

##############################################################################
# Project, target, sources and paths
#

# Define project name here
PROJECT = PompeSAP

# Target settings.
MCU  = cortex-m4

# Imported source files and paths.
CHIBIOS  := ./ChibiOS
CONFDIR  := ./cfg
BUILDDIR := ./build
DEPDIR   := ./.dep
VARIOUS = ./chibios_enac_various_common
TOOLDIR  := $(VARIOUS)/TOOLS


# Licensing files.
include $(CHIBIOS)/os/license/license.mk
# Startup files.
include $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk/startup_stm32l4xx.mk
# HAL-OSAL files (optional).
include $(CHIBIOS)/os/hal/hal.mk
include $(CHIBIOS)/os/hal/ports/STM32/STM32L4xx/platform_l432.mk
include $(CONFDIR)/board.mk
include $(CHIBIOS)/os/hal/osal/rt-nil/osal.mk
# RTOS files (optional).
include $(CHIBIOS)/os/rt/rt.mk
include $(CHIBIOS)/os/common/ports/ARMv7-M-ALT/compilers/GCC/mk/port.mk
#include $(CHIBIOS)/os/common/ports/ARMv7-M/compilers/GCC/mk/port.mk
# Auto-build files in ./source recursively.
include $(CHIBIOS)/tools/mk/autobuild.mk
# Other files (optional).
#include $(CHIBIOS)/os/test/test.mk
#include $(CHIBIOS)/test/rt/rt_test.mk
#include $(CHIBIOS)/test/oslib/oslib_test.mk
include $(CHIBIOS)/os/hal/lib/complex/mfs/hal_mfs.mk

# Define linker script file here
LDSCRIPT= $(STARTUPLD)/STM32L432xC.ld

# C sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CSRC = $(ALLCSRC) \
       $(VARIOUS)/microrl/microrl.c \
       $(VARIOUS)/microrl/microrlShell.c \
       $(VARIOUS)/stdutil.c \
       $(VARIOUS)/printf.c

# C++ sources that can be compiled in ARM or THUMB mode depending on the global
# setting.
CPPSRC = $(ALLCPPSRC) $(VARIOUS)/embedeedStub.cpp

# List ASM source files here.
ASMSRC = $(ALLASMSRC)

# List ASM with preprocessor source files here.
ASMXSRC = $(ALLXASMSRC)

# Inclusion directories.
INCDIR = $(CONFDIR) $(ALLINC) $(TESTINC) src $(VARIOUS)

# Define C warning options here.
CWARN = -Wall -Wextra -Wundef -Wstrict-prototypes -Wcast-align=strict

# Define C++ warning options here.
CPPWARN = -Wall -Wextra -Wundef

#
# Project, target, sources and paths
##############################################################################

##############################################################################
# Start of user section
#

# List all user C define here, like -D_DEBUG=1
UDEFS = #-DTRACE

# Define ASM defines here
UADEFS =

# List all user directories here
UINCDIR = $(VARIOUS)/microrl

# List the user directory to look for the libraries here
ULIBDIR =

# List all user libraries here
ULIBS =

#
# End of user section
##############################################################################

##############################################################################
# Common rules
#

RULESPATH = $(CHIBIOS)/os/common/startup/ARMCMx/compilers/GCC/mk
include $(RULESPATH)/arm-none-eabi.mk
include $(RULESPATH)/rules.mk


#
# Common rules
##############################################################################

##############################################################################
# Custom rules
#
$(OBJS): $(CONFDIR)/board.h

$(CONFDIR)/board.h: $(CONFDIR)/board.cfg
	$(shell mkdir -p $(dir $@))
	$(TOOLDIR)/boardGen.pl -opd=STM32_open_pin_data --no-pp-line $<  $@

flash: build/PompeSAP.elf
	$(TOOLDIR)/bmpflash build/PompeSAP.elf

dfu_flash: build/ch.bin
	dfu-util -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000:leave -D build/ch.bin

board_clean:
	$(info Removing $(CONFDIR)/board.h)
	@rm -f $(CONFDIR)/board.h

clean: board_clean


#
# Custom rules
##############################################################################
