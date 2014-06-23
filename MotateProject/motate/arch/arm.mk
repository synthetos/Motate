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


#######
#
# These values must be defined before including this file:
# $(CHIP)
# $(CHIP_SERIES)
# $(CPU_DEV)


DEVICE_LIBS          = gcc c

# ---------------------------------------------------------------------------------------
# C Flags (NOT CPP flags)

DEVICE_CFLAGS := -D__$(CHIP)__ -D__$(CHIP_SERIES)__ --param max-inline-insns-single=500 -mcpu=$(CPU_DEV) -mthumb -mlong-calls -ffunction-sections -fdata-sections -nostdlib -std=gnu99 -u _printf_float


# ---------------------------------------------------------------------------------------
# CPP Flags

DEVICE_CPPFLAGS := -D__$(CHIP)__ -D__$(CHIP_SERIES)__ --param max-inline-insns-single=500 -mcpu=$(CPU_DEV) -mthumb -mlong-calls -ffunction-sections -fdata-sections -nostdlib -std=gnu++11 -fno-rtti -fno-exceptions -u _printf_float

# ---------------------------------------------------------------------------------------
# Assembly Flags

DEVICE_ASFLAGS  := -D__$(CHIP)__ -D__$(CHIP_SERIES)__ -mcpu=$(CPU_DEV) -mthumb

# ---------------------------------------------------------------------------------------
# Linker Flags

DEVICE_LDFLAGS := -nostartfiles -mcpu=$(CPU_DEV) --specs=nano.specs -u _printf_float -mthumb -L$(DEVICE_LINKER_SCRIPT_PATH)

