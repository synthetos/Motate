# ----------------------------------------------------------------------------
# This file is part of the Motate project. It is heavily modified from a file
# that contained this original copyright:

# ----------------------------------------------------------------------------
#         ATMEL Microcontroller Software Support
# ----------------------------------------------------------------------------
# Copyright (c) 2010, Atmel Corporation
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following condition is met:
#
# - Redistributions of source code must retain the above copyright notice,
# this list of conditions and the disclaimer below.
#
# Atmel's name may not be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
# DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

#-------------------------------------------------------------------------------
#        User-modifiable options
#-------------------------------------------------------------------------------

ifeq ('$(CHIP)','')
$(error CHIP not defined)
endif

include $(MOTATE_PATH)/platform/atmel_sam/atmel_sam_series.mk

# fill the needed variables
ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3N)))

#BOARD:=SAM3N_EK
CHIP_SERIES:=sam3n

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3S)))

#BOARD:=SAM3S_EK
CHIP_SERIES:=sam3s

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3SD8)))

#BOARD:=SAM3S_EK2
CHIP_SERIES:=sam3sd8

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3U)))

#BOARD:=SAM3U_EK
CHIP_SERIES:=sam3u

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3XA)))

#BOARD:=SAM3X_EK
CHIP_SERIES:=sam3x

    ifeq ($(CHIP),$(findstring $(CHIP), $(SAM3X8)))
        CHIP_SUBSERIES:=sam3x8
    else
        CHIP_SUBSERIES:=sam3x4
    endif

CPU_DEV = cortex-m3

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM4S)))

#BOARD:=SAM4S_EK
CHIP_SERIES:=sam4s

CPU_DEV = cortex-m4
#FLOAT_OPTIONS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAM4E)))

    CHIP_SERIES:=sam4e

    ifeq ($(CHIP),$(findstring $(CHIP), $(SAM4E8)))
        CHIP_SUBSERIES:=sam4e8
    else
        CHIP_SUBSERIES:=sam4e16
    endif

    CPU_DEV = cortex-m4
    #FLOAT_OPTIONS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

else ifeq ($(CHIP),$(findstring $(CHIP), $(SAMS70)))

    CHIP_SERIES:=sams70

    ifeq ($(CHIP),$(findstring $(CHIP), SAMS70N19))
        CHIP_SUBSERIES:=sams70n19
    else ifeq ($(CHIP),$(findstring $(CHIP), SAMS70N20)
        CHIP_SUBSERIES:=sams70n20
    else ifeq ($(CHIP),$(findstring $(CHIP), SAMS70N21)
        CHIP_SUBSERIES:=sams70n21
    else
        $(error $(CHIP) is not in a known Atmel subseries (incomplete makefile, most likely).)
    endif

    CPU_DEV = cortex-m7
    #FLOAT_OPTIONS = -mfloat-abi=hard -mfpu=fpv4-sp-d16

else

$(error $(CHIP) is not a known Atmel processor.)

endif



# GCC toolchain provider
GCC_TOOLCHAIN = gcc

# Toolchain prefix when cross-compiling
CROSS_COMPILE = arm-none-eabi

SAM_PATH    = $(CMSIS_ROOT)/TARGET_Atmel
DEVICE_PATH = $(SAM_PATH)/$(CHIP_SERIES)

SAM_SOURCE_DIRS += $(MOTATE_PATH)/Atmel_sam_common
SAM_SOURCE_DIRS += $(MOTATE_PATH)/Atmel_$(CHIP_SERIES)
SAM_SOURCE_DIRS += $(DEVICE_PATH)/source/templates
SAM_SOURCE_DIRS += $(DEVICE_PATH)/source/templates/$(GCC_TOOLCHAIN)
SAM_SOURCE_DIRS += $(MOTATE_PATH)/platform/atmel_sam
FIRST_LINK_SOURCES += $(MOTATE_PATH)/platform/atmel_sam/syscalls.cpp

DEVICE_RULES = $(call CREATE_DEVICE_LIBRARY,SAM,cmsis_sam)

# Flags
DEVICE_INCLUDE_DIRS += $(CMSIS_ROOT)
DEVICE_INCLUDE_DIRS += $(DEVICE_PATH)/include
DEVICE_INCLUDE_DIRS += $(SAM_PATH)
DEVICE_INCLUDE_DIRS += $(SAM_PATH)/preprocessor
DEVICE_INCLUDE_DIRS += $(SAM_PATH)/$(CHIP_SERIES)/include
DEVICE_INCLUDE_DIRS += $(SAM_PATH)/$(CHIP_SERIES)/source/templates
DEVICE_INCLUDE_DIRS += $(MOTATE_PATH)/Atmel_sam_common
DEVICE_INCLUDE_DIRS += $(MOTATE_PATH)/Atmel_$(CHIP_SERIES)
DEVICE_INCLUDE_DIRS += $(MOTATE_PATH)/platform/atmel_sam

# Add this to search for the additional .ld script
DEVICE_LIB_DIRS     += $(MOTATE_PATH)/platform/atmel_sam


DEVICE_LINKER_SCRIPT = $(SAM_PATH)/linker_scripts/$(CHIP_SERIES)/$(CHIP_SUBSERIES)/$(GCC_TOOLCHAIN)/flash.ld
DEVICE_LINKER_SCRIPT_PATH = $(DEVICE_PATH)/source/$(GCC_TOOLCHAIN)/


include $(MOTATE_PATH)/arch/arm.mk
