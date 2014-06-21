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

##############################################################################################
# Start of default section
#

PROJECT  = MotateProject

MOTATE_PATH ?= motate

SOURCE_DIRS = . "${MOTATE_PATH}"
# arduino arduino/USB variants

FIRST_LINK_SOURCES :=

# List all user directories here
USER_INCLUDE_DIRS := $(SOURCE_DIRS)

# List the user directory to look for the libraries here
USER_LIB_DIRS =

# List all user libraries here
USER_LIBS =

# Define optimisation level here
OPTIMIZATION ?= 0
#OPTIMIZATION ?= s
#OPTIMIZATION ?= 3

CFLAGS   :=
CPPFLAGS :=

# List ASM source files here
ASRC =

DEFAULT_MEMORY = flash


# ---------------------------------------------------------------------------------------
# C Flags (NOT CPP Flags)

# Configure warnings:
CFLAGS += -Wall -Wchar-subscripts -Wcomment -Wformat=2 -Wimplicit-int
CFLAGS += -Werror-implicit-function-declaration -Wmain -Wparentheses
CFLAGS += -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
CFLAGS += -Wuninitialized -Wno-unknown-pragmas -Wfloat-equal -Wundef
CFLAGS += -Wshadow -Wpointer-arith -Wbad-function-cast -Wwrite-strings
CFLAGS += -Wsign-compare -Waggregate-return
#CFLAGS += -Wstrict-prototypes
#CFLAGS += -Wmissing-prototypes -Wmissing-declarations
CFLAGS += -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations
CFLAGS += -Wredundant-decls -Wnested-externs -Winline -Wlong-long
CFLAGS += -Wunreachable-code
#CFLAGS += -Wcast-align -Wpacked 
#CFLAGS += -Wmissing-noreturn
#CFLAGS += -Wconversion

# Turn off printf() format strings. We use late bound FLASH and RAM format strings and this causes warnings
CFLAGS += -Wno-format-nonliteral -Wno-format-security

# To reduce application size use only integer printf function.
#CFLAGS += -Dprintf=iprintf

# ---------------------------------------------------------------------------------------
# CPP Flags

CPPFLAGS += -Wall -Wchar-subscripts -Wcomment -Wformat=2
CPPFLAGS += -Wmain -Wparentheses -Wunreachable-code
#CPPFLAGS += -Wcast-align
CPPFLAGS += -Wsequence-point -Wreturn-type -Wswitch -Wtrigraphs -Wunused
CPPFLAGS += -Wuninitialized -Wno-unknown-pragmas -Wfloat-equal -Wundef
CPPFLAGS += -Wshadow -Wpointer-arith -Wwrite-strings
#CPPFLAGS += -Wsign-compare -Waggregate-return -Wmissing-declarations
CPPFLAGS += -Wformat -Wmissing-format-attribute -Wno-deprecated-declarations
CPPFLAGS += -Wredundant-decls -Winline -Wlong-long
#CPPFLAGS += -Wmissing-noreturn -Wpacked 
#CPPFLAGS += -Wconversion

# Turn off printf() format strings. We use late bound FLASH and RAM format strings and this causes warnings
CPPFLAGS += -Wno-format-nonliteral -Wno-format-security

# To reduce application size use only integer printf function.
#CPPFLAGS += -Dprintf=iprintf

# Default is to compile more quielty.
# To show actual commands: make [options] VERBOSE=1
VERBOSE ?= 0
COLOR ?= 1

ifeq ($(VERBOSE),0)
QUIET := @
else
QUIET :=
endif

ifeq ($(COLOR),1)
START_BOLD=`tput bold`
END_BOLD=`tput sgr0`
NO_COLOR=`tput setaf 0`
OK_COLOR=`tput setaf 2`
ERROR_COLOR=`tput setaf 1`
WARN_COLOR=`tput setaf 3`
else
QUIET :=
endif


#
# End of default section
##############################################################################################

##############################################################################################
# Output directories (must be before platforms section)
#

BIN = bin/$(PLATFORM)
OBJ = build/$(PLATFORM)
DEPDIR = $(OBJ)/dep

#
# End of Output directories
##############################################################################################

##############################################################################################
# Start of platforms section
#

# Available platform-specific defines:
# MOTATE_CONFIG_HAS_USBSERIAL
#

# Common location for the CMSIS directory (might not be used)
CMSIS_ROOT = cmsis

# Default platform
PLATFORM ?= Due

ifeq ("$(PLATFORM)","Due")

	BASE_PLATFORM=due
	DEVICE_DEFINES += MOTATE_BOARD="Due"
	DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=1
endif

ifeq ("$(PLATFORM)","FRDM-KL05Z")

BASE_PLATFORM=frdm-kl05z
DEVICE_DEFINES += MOTATE_BOARD="FRDM-KL05Z"
endif


ifeq ("$(BASE_PLATFORM)","")
	BASE_PLATFORM = $(PLATFORM)
endif

ifeq ("$(BASE_PLATFORM)","due")
	_PLATFORM_FOUND = 1
	#FIRST_LINK_SOURCES += ${MOTATE_PATH}/SamTimers.cpp ${MOTATE_PATH}/SamUSB.cpp ${MOTATE_PATH}/SamPins.cpp
	FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Atmel_sam3xa/*.cpp)

	CHIP = SAM3X8E
	CHIP_LOWERCASE = sam3x8e
    PLATFORM_BASE = platform/atmel_sam

	DEVICE_INCLUDE_DIRS += $(PLATFORM_BASE)/board/due

	include $(PLATFORM_BASE).mk
endif

ifeq ("$(BASE_PLATFORM)","frdm-kl05z")
	_PLATFORM_FOUND = 1
	FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Freescale_klxx/*.cpp)

	CHIP = MKL05Z32VFM4
	CHIP_LOWERCASE = mkl05z32vfm4
    PLATFORM_BASE = platform/freescale_klxx

	DEVICE_INCLUDE_DIRS += $(PLATFORM_BASE)/board/frdm

	include $(PLATFORM_BASE).mk
endif

ifeq ("$(_PLATFORM_FOUND)", "0")
# errors cannot be indented
$(error Unknown platform "$(PLATFORM)")
endif

#
# End of platforms section
##############################################################################################

##############################################################################################
# Actual compilation
#


# Output file basename
OUTPUT_BIN = $(BIN)/$(PLATFORM)

#-------------------------------------------------------------------------------
#		Tools
#-------------------------------------------------------------------------------

# Compilation tools
CC      = $(CROSS_COMPILE)-gcc
CXX     = $(CROSS_COMPILE)-g++
LD      = $(CROSS_COMPILE)-ld
AR      = $(CROSS_COMPILE)-ar
SIZE    = $(CROSS_COMPILE)-size
STRIP   = $(CROSS_COMPILE)-strip
OBJCOPY = $(CROSS_COMPILE)-objcopy
GDB     = $(CROSS_COMPILE)-gdb
NM      = $(CROSS_COMPILE)-nm
RM      = rm
CP      = cp
# NOTE: Atmel Studio 6.1 will default to the wrong mkdir that doesn't understand -p.
#       So, in AS6.1, we have to pass MKDIR=gmkdir in the command line.
MKDIR   ?= mkdir

SHELL = bash
export PATH := $(PATH):../Tools/gcc-$(CROSS_COMPILE)/bin


# ---------------------------------------------------------------------------------------
# C Flags (NOT CPP flags)

#CFLAGS +=  $(DEVICE_CFLAGS)
CFLAGS +=  $(DEVICE_CFLAGS) -DPLATFORM=$(PLATFORM)

# ---------------------------------------------------------------------------------------
# CPP Flags

#CPPFLAGS +=  $(DEVICE_CPPFLAGS)
CPPFLAGS +=  $(DEVICE_CPPFLAGS) -DPLATFORM=$(PLATFORM)

# ---------------------------------------------------------------------------------------
# ASM Flags

ASFLAGS = $(DEVICE_ASFLAGS)


# ---------------------------------------------------------------------------------------
# Linker Flags

LDFLAGS += $(LIBS) $(USER_LIBS) -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--warn-unresolved-symbols $(DEVICE_LDFLAGS)


#-------------------------------------------------------------------------------
#		Files
#-------------------------------------------------------------------------------

# Directories where source files can be found

C_SOURCES   = $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.c) )
CXX_SOURCES = $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.cpp) )
ASM_SOURCES = $(foreach dir, $(SOURCE_DIRS), $(wildcard $(dir)/*.s) )

C_OBJECTS   := $(addsuffix .o,$(basename $(C_SOURCES)))
CXX_OBJECTS := $(addsuffix .o,$(basename $(CXX_SOURCES)))
ASM_OBJECTS := $(addsuffix .o,$(basename $(ASM_SOURCES)))
FIRST_LINK_OBJECTS = $(addsuffix .o,$(basename $(FIRST_LINK_SOURCES)))

# Append OBJ and BIN directories to output filename
OUTPUT := $(BIN)/$(OUTPUT_BIN)

INCLUDES = $(patsubst %,-I%,$(DEVICE_INCLUDE_DIRS) $(USER_INCLUDE_DIRS))
LIBS     = $(patsubst %,-l%,$(DEVICE_LIBS) $(USER_LIBS))
LIBDIR   = $(patsubst %,-L%,$(DEVICE_LIB_DIRS) $(USER_LIB_DIRS))

CFLAGS   += -g3 -O$(OPTIMIZATION) $(INCLUDES) $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))
CPPFLAGS += -g3 -O$(OPTIMIZATION) $(INCLUDES) $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))
ASFLAGS  += -g3 -O$(OPTIMIZATION) $(INCLUDES) -D__ASSEMBLY__ $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))

ifeq ($(VERBOSE),2)
define NEWLINE_ONLY


endef
define NEWLINE_TAB

	
endef

$(info DEVICE_INCLUDE_DIRS+USER_INCLUDE_DIRS:$(patsubst %,$(NEWLINE_TAB)%,$(DEVICE_INCLUDE_DIRS) $(USER_INCLUDE_DIRS)))
$(info $(NEWLINE_ONLY)DEVICE_LIB_DIRS+USER_LIB_DIRS: $(patsubst %,$(NEWLINE_TAB)%,$(DEVICE_LIB_DIRS) $(USER_LIB_DIRS)))
$(info $(NEWLINE_ONLY)SOURCE_DIRS: $(patsubst %,$(NEWLINE_TAB)%,$(SOURCE_DIRS)))
$(info $(NEWLINE_ONLY)C_SOURCES: $(patsubst %,$(NEWLINE_TAB)%,$(C_SOURCES)))
$(info $(NEWLINE_ONLY)CXX_SOURCES: $(patsubst %,$(NEWLINE_TAB)%,$(CXX_SOURCES)))
$(info $(NEWLINE_ONLY)FIRST_LINK_SOURCES: $(patsubst %,$(NEWLINE_TAB)%,$(FIRST_LINK_SOURCES)))
endif


#-------------------------------------------------------------------------------
#		Rules
#-------------------------------------------------------------------------------


all: TOOLS $(DEFAULT_MEMORY)

REQUIRED_DIRS := $(BIN) $(OBJ) $(DEPDIR)

MK_DIRS =   $(shell                              \
              for d in $(REQUIRED_DIRS);         \
              do                                 \
				echo "Checking for $${d}"        \
                [[ -d $$d ]] || mkdir -p $$d;    \
              done)

$(eval $(DEVICE_RULES))

TOOLS:
if test \! `command -v ${CC}`; then cd ../Tools && make "ARCH=${CROSS_COMPILE}"; fi

#-------------------------------------------------------------------------------
#		Per-Memory Rules Block
#-------------------------------------------------------------------------------

define RULES
OUTDIR = $(OBJ)/$(1)
REQUIRED_DIRS += $(OUTDIR)
C_OBJECTS_$(1) := $(addprefix $$(OUTDIR)/,$(C_OBJECTS))
CXX_OBJECTS_$(1) := $(addprefix $$(OUTDIR)/,$(CXX_OBJECTS))
ASM_OBJECTS_$(1) := $(addprefix $$(OUTDIR)/,$(ASM_OBJECTS))
#LINKER_SCRIPT_$(1) ?= $(PLATFORM_BASE)/gcc_$(1).ld
MEMORY_TYPE := $(1)
LINKER_SCRIPT_$(1) := $(DEVICE_LINKER_SCRIPT)
ABS_LINKER_SCRIPT_$(1) = $(abspath $$(LINKER_SCRIPT_$(1)))
FIRST_LINK_OBJECTS_$(1) := $(addprefix $$(OUTDIR)/,$(FIRST_LINK_OBJECTS))

# Generate dependency information
DEPFLAGS_$(1) = -MMD -MF $(OBJ)/dep/$$(@F)_$(1).d -MT $(subst $(OUTDIR),$(OBJ)/$(1),$$@)

$$(OUTDIR)/core.a: $$(OUTDIR)/core.a( $$(ASM_OBJECTS_$(1)) $$(CXX_OBJECTS_$(1)) $$(C_OBJECTS_$(1)) ) | $(BIN) $(OBJ)

ALL_C_OBJECTS_$(1)   += $$(C_OBJECTS_$(1))
ALL_CXX_OBJECTS_$(1) += $$(CXX_OBJECTS_$(1))
ALL_ASM_OBJECTS_$(1) += $$(ASM_OBJECTS_$(1))

.PHONY: $(1)

$(1): $(OUTPUT_BIN)_$(1).elf

$(OUTPUT_BIN)_$(1).elf: $$(OUTDIR)/core.a $$(DEVICE_OBJECTS_$(1))
	@echo $$(START_BOLD)"Linking $(OUTPUT_BIN)_$(1).elf" $$(END_BOLD)
	@echo $(START_BOLD)"Using linker script: $$(ABS_LINKER_SCRIPT_$(1))" $(END_BOLD)
	$(QUIET)$(CXX) $(LIB_PATH) -T"$$(ABS_LINKER_SCRIPT_$(1))" -Wl,-Map,"$(OUTPUT_BIN)_$(1).map" -o $$@ $(LDFLAGS) $(LD_OPTIONAL) $(LIBS) -Wl,--start-group $$(FIRST_LINK_OBJECTS_$(1)) $$+ -Wl,--end-group
	@echo $(START_BOLD)"Exporting symbols $(OUTPUT_BIN)_$(1).elf.txt" $(END_BOLD)
	$(QUIET)$(NM) "$(OUTPUT_BIN)_$(1).elf" >"$(OUTPUT_BIN)_$(1).elf.txt"
	@echo $(START_BOLD)"Making binary $(OUTPUT_BIN)_$(1).bin" $(END_BOLD)
	$(QUIET)$(OBJCOPY) -O binary "$(OUTPUT_BIN)_$(1).elf" "$(OUTPUT_BIN)_$(1).bin"
	@echo "--- SIZE INFO ---"
	$(QUIET)$(SIZE) "$(OUTPUT_BIN)_$(1).elf"

$$(ALL_CXX_OBJECTS_$(1)): $$(OUTDIR)/%.o: %.cpp
	$(QUIET)$(MKDIR) -p "$$(@D)" "$$(DEPDIR)" "$$(BIN)"
	@echo $(START_BOLD)"Compiling cpp ($(1)) $$<"; echo "    -> $$@" $(END_BOLD)
	$(QUIET)$(CXX) $(CPPFLAGS) $$(DEPFLAGS_$(1)) -D$(1) -xc++ -c -o $$@ $$<

$$(ALL_C_OBJECTS_$(1)): $$(OUTDIR)/%.o: %.c
	$(QUIET)$(MKDIR) -p "$$(@D)" "$$(DEPDIR)" "$$(BIN)"
	@echo $(START_BOLD)"Compiling c ($(1)) $$<"; echo "    -> $$@" $(END_BOLD)
	$(QUIET)$(CC) $(CFLAGS) $$(DEPFLAGS_$(1)) -D$(1) -c -o $$@ $$<

$$(ALL_ASM_OBJECTS_$(1)): $$(OUTDIR)/%.o: %.S
	$(QUIET)$(MKDIR) -p "$$(@D)" "$$(DEPDIR)" "$$(BIN)"
	@echo $(START_BOLD)"Compiling ($(1)) $$<"; echo "    -> $$@"  $(END_BOLD)
	$(QUIET)$(CC) $(ASFLAGS) $$(DEPFLAGS_$(1)) -D$(1) -c -o $$@ $$<

debug_$(1): $(1)
	$(GDB) -x "$(CHIP)_$(1).gdb" -ex "reset" -readnow -se "$(OUTPUT_BIN)_$(1).elf"

# 
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir $(OBJ)/dep 2>/dev/null) $(wildcard $(OBJ)/dep/*.d)

endef

#$(foreach MEMORY, $(MEMORIES), $(warning $(call RULES,$(MEMORY))))
$(foreach MEMORY, $(MEMORIES), $(eval $(call RULES,$(MEMORY))))


#-------------------------------------------------------------------------------
#		Default $(PROJECT).elf, and clean
#-------------------------------------------------------------------------------

$(PROJECT).elf: $(OUTPUT_BIN)_$(DEFAULT_MEMORY).elf
	$(CP) $< $@

$(PROJECT).map: $(OUTPUT_BIN)_$(DEFAULT_MEMORY).map
	$(CP) $< $@

$(PROJECT).hex: $(OUTPUT_BIN)_$(DEFAULT_MEMORY).elf
	$(QUIET)$(OBJCOPY) -O ihex "$(OUTPUT_BIN)_$(DEFAULT_MEMORY).elf" "$(PROJECT)".hex

clean:
	-$(RM) -fR $(OBJ) $(BIN) $(PROJECT).elf

# *** EOF ***