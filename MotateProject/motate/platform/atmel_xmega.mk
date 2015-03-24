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


CHIP_SERIES=XMEGA

# Toolchain prefix when cross-compiling
CROSS_COMPILE = avr

DEVICE_PATH = $(MOTATE_PATH)/platform/atmel_xmega

XMEGA_SOURCE_DIRS += $(MOTATE_PATH)/Atmel_XMega
XMEGA_SOURCE_DIRS += $(DEVICE_PATH)
FIRST_LINK_SOURCES += $(MOTATE_PATH)/platform/atmel_xmega/syscalls.c
FIRST_LINK_SOURCES += $(MOTATE_PATH)/platform/atmel_xmega/xmega.c

DEVICE_RULES = $(call CREATE_DEVICE_LIBRARY,XMEGA,device_xmega)

# Flags
DEVICE_INCLUDE_DIRS += $(DEVICE_PATH)
DEVICE_INCLUDE_DIRS += $(MOTATE_PATH)/Atmel_XMega


DEVICE_LINKER_SCRIPT =

CPU_DEV = $(CHIP)

FLASH_REQUIRES = $(OUTPUT_BIN).hex
AVRDUDE_PORT ?= /dev/tty.usbserial-*
DEVICE_FLASH_CMD = avrdude -C $(TOOLS_PATH)/gcc-avr/etc/avrdude.conf -D  -p x192a3 -c avr109 -b 115200 -P $(AVRDUDE_PORT) -U flash:w:$(OUTPUT_BIN).hex

include $(MOTATE_PATH)/arch/avr.mk