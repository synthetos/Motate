#
# Makefile
# 
# Copyright (c) 2012 - 2014 Robert Giseburt
# Copyright (c) 2013 - 2014 Alden S. Hart Jr.
# 
#	This file is part of the Motate Library.
#
#	This file ("the software") is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License, version 2 as published by the
#	Free Software Foundation. You should have received a copy of the GNU General Public
#	License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
#
#	As a special exception, you may use this file as part of a software library without
#	restriction. Specifically, if other files instantiate templates or use macros or
#	inline functions from this file, or you compile this file and link it with  other
#	files to produce an executable, this file does not by itself cause the resulting
#	executable to be covered by the GNU General Public License. This exception does not
#	however invalidate any other reasons why the executable file might be covered by the
#	GNU General Public License.
#
#	THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
#	WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
#	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
#	SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
#	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

ifeq ('$(CHIP)','')
$(error CHIP not defined)
endif

KL05Z8 = MKL05Z8VFK4 MKL05Z8VLC4 MKL05Z8VFM4

KL05Z16 = MKL05Z16VFK4 MKL05Z16VLC4 MKL05Z16VFM4 MKL05Z16VLF4

KL05Z32 = MKL05Z32VFK4 MKL05Z32VLC4 MKL05Z32VFM4 MKL05Z32VLF4

KL05Z_UNLINKABLE = $(KL05Z8) $(KL05Z16)

# fill the needed variables
ifeq ($(CHIP),$(findstring $(CHIP), $(KL05Z_UNLINKABLE)))

$(error $(CHIP) is not yet supported, simply because there's not a linker script prepared for it.)

else ifeq ($(CHIP),$(findstring $(CHIP), $(KL05Z32)))

LD_SCRIPT_BASENAME:=MKL05Z4
CHIP_SERIES=KL05Z

else

$(error $(CHIP) is not a known Atmel processor.)

endif



# GCC toolchain provider
GCC_TOOLCHAIN = TOOLCHAIN_GCC_ARM

# Toolchain prefix when cross-compiling
CROSS_COMPILE = arm-none-eabi

KLXX_PATH   = $(CMSIS_ROOT)/TARGET_Freescale/TARGET_KLXX/TARGET_KL05Z
DEVICE_PATH = $(KLXX_PATH)

KLXX_SOURCE_DIRS += $(MOTATE_PATH)/Freescale_klxx
KLXX_SOURCE_DIRS += $(DEVICE_PATH)
KLXX_SOURCE_DIRS += $(DEVICE_PATH)/$(GCC_TOOLCHAIN)
KLXX_SOURCE_DIRS += $(MOTATE_PATH)/platform/freescale_klxx
FIRST_LINK_SOURCES += $(MOTATE_PATH)/platform/freescale_klxx/syscalls.c

DEVICE_RULES = $(call CREATE_DEVICE_LIBRARY,KLXX,cmsis_klxx)

# Flags
DEVICE_INCLUDE_DIRS += $(CMSIS_ROOT)
DEVICE_INCLUDE_DIRS += $(KLXX_PATH)
DEVICE_INCLUDE_DIRS += $(MOTATE_PATH)/Freescale_klxx


CPU_DEV = cortex-m0plus

DEVICE_LINKER_SCRIPT = $(DEVICE_PATH)/$(GCC_TOOLCHAIN)/$(LD_SCRIPT_BASENAME).ld

include $(MOTATE_PATH)/arch/arm.mk