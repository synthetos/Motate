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

BOARD  ?= NONE

MOTATE_PATH ?= $(dir "$(CURDIR)"/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
export MOTATE_PATH

TOOLS_PATH ?= ${MOTATE_PATH}/../../Tools

SOURCE_DIRS ?= .
SOURCE_DIRS += ${MOTATE_PATH}

FIRST_LINK_SOURCES +=

# List all user directories here
USER_INCLUDE_DIRS += $(SOURCE_DIRS)

# List the user directory to look for the libraries here
USER_LIB_DIRS +=

# List all user libraries here
USER_LIBS +=

# Define optimisation level here
#OPTIMIZATION ?= 0
OPTIMIZATION ?= s
#OPTIMIZATION ?= 3

CFLAGS   +=
CPPFLAGS +=

# The project MUST specify the tools version, so we can assure a match bewteen motate and the project.
TOOLS_VERSION ?= unspecified

#
# End of configuration section
##############################################################################################

##############################################################################################
# Start of configuration section
#

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
CPPFLAGS += -Wredundant-decls -Wlong-long
#CPPFLAGS += -Winline
#CPPFLAGS += -Wmissing-noreturn -Wpacked
#CPPFLAGS += -Wconversion

# Turn off printf() format strings. We use late bound FLASH and RAM format strings and this causes warnings
CPPFLAGS += -Wno-format-nonliteral -Wno-format-security

#
# End of configuration section
##############################################################################################

##############################################################################################
# Start of build support section
#

# Default is to compile more quielty.
# To show actual commands: make [options] VERBOSE=1
VERBOSE ?= 0
COLOR ?= 1
RECORD ?= 0

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
endif


ifeq ($(VERBOSE),2)
define NEWLINE_ONLY


endef
define NEWLINE_TAB


endef
endif

ifeq ($(RECORD),0)
REC := true #
else
REC := recordCompilation() { $(realpath ${MOTATE_PATH}/compile_recorder.js) "$${@}"; }; \
	useNVMorDirectNode() { \
		if [[ -f ${HOME}/.nvm/nvm.sh ]]; then \
			. ${HOME}/.nvm/nvm.sh && nvm use --lts && recordCompilation "$${@}"; \
		else \
			recordCompilation "$${@}"; \
		fi;\
	}; useNVMorDirectNode
endif

#
# End of build support section
##############################################################################################

##############################################################################################
# Output directories (must be before BOARDs section)
#

ifeq ($(strip $(CONFIG)),)
	BIN=bin/$(BOARD)
	OBJ=build/$(BOARD)
else
	BIN=bin/$(CONFIG)-$(BOARD)
	OBJ=build/$(CONFIG)-$(BOARD)
endif
DEPDIR=$(OBJ)/dep

#
# End of Output directories
##############################################################################################

include ${MOTATE_PATH}/MotateUtilities.mk

##############################################################################################
# Start of BOARDs section
#

# Available BOARD-specific defines:
# MOTATE_CONFIG_HAS_USBSERIAL
#

# Common location for the CMSIS directory (might not be used)
CMSIS_ROOT = ${MOTATE_PATH}/cmsis

# STAR is a stupid hack to make XCode stopthinking the REST OF THE FILE
# is a comment.
STAR:=*
-include ./boards.mk
include $(wildcard ${MOTATE_PATH}/board/$(STAR).mk)

ifneq ("$(_BOARD_FOUND)", "1")

ifeq ('$(BOARD)','NONE')
# errors cannot be indented
$(error BOARD not defined - please provide a project name)
endif

# errors cannot be indented
$(error Unknown board "$(BOARD)")
endif

#
# End of BOARDs section
##############################################################################################

##############################################################################################
# Start of setup tools
#

# Output file basename
OUTPUT_BIN := $(BIN)/$(PROJECT)

# Some basical (external) utility paths
RM      = rm
CP      = cp
# NOTE: Atmel Studio will default to the wrong mkdir that doesn't understand -p.
#       So, in windows we set MKDIR=gmkdir below.
MKDIR   = mkdir
GIT     = git

SHELL = bash

# Unix/Linux section:
UNAME := $(shell uname -s)

ifeq (Darwin,${UNAME})
$(info "Found that we're in OS X")
OS = OSX
TOOLS_SUBPATH := osx/gcc-$(CROSS_COMPILE)-$(TOOLS_VERSION)
else
ifeq (Linux,${UNAME})
$(info "Found that we're in Linux")
OS = LINUX
TOOLS_SUBPATH := linux/gcc-$(CROSS_COMPILE)-$(TOOLS_VERSION)
endif #LINUX
endif #Darwin


PATH := $(TOOLS_PATH)/$(TOOLS_SUBPATH)/bin:$(PATH)

# end Unix/linux section


export PATH
export MOTATE_PATH

CWD = $(realpath .)

# Compilation tools - hardcode the full path to the ones we provide
TOOLS_FULLPATH = "$(TOOLS_PATH)/$(TOOLS_SUBPATH)/bin"
CC      = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-gcc
CXX     = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-g++
LD      = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-ld
AR      = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-ar
SIZE    = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-size
STRIP   = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-strip
OBJCOPY = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-objcopy
GDB     = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-gdb
GDB_PY  = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-gdb-py
NM      = $(TOOLS_FULLPATH)/$(CROSS_COMPILE)-nm


ifneq ($(NOT_IN_GIT),1)
	GIT_LOCATED := $(GIT)
	GIT_VERSION := $(shell $(GIT) describe --abbrev=4 --dirty --always --tags)
	GIT_EXACT_VERSION := $(shell $(GIT) describe --abbrev=0 --tags)

	# Push the git version into the code -- the ""s must be escaped and make it into the code intact
	DEVICE_DEFINES += GIT_VERSION=\"$(GIT_VERSION)\" GIT_EXACT_VERSION=$(GIT_EXACT_VERSION)
else
   # Push the git version into the code -- the ""s must be escaped and make it into the code intact
   DEVICE_DEFINES += GIT_VERSION=\"UNKNOWN-not-from-git\" GIT_EXACT_VERSION=0.0
endif


# ---------------------------------------------------------------------------------------
# LTO generic flags

ifeq ($(USE_LTO),1)
LTO = -flto
else
LTO =
endif


# ---------------------------------------------------------------------------------------
# C Flags (NOT CPP flags)

#CFLAGS +=  $(DEVICE_CFLAGS)
CFLAGS +=  $(DEVICE_CFLAGS) -DBOARD=$(BOARD) $(LTO)

# ---------------------------------------------------------------------------------------
# CPP Flags

#CPPFLAGS +=  $(DEVICE_CPPFLAGS)
CPPFLAGS +=  $(DEVICE_CPPFLAGS) -DBOARD=$(BOARD)  $(LTO)

# ---------------------------------------------------------------------------------------
# ASM Flags

ASFLAGS = $(DEVICE_ASFLAGS)  $(LTO)

# ---------------------------------------------------------------------------------------
# Linker Flags

DEBUG_SYMBOLS = -g3

LDFLAGS += $(LIBS) $(USER_LIBS) $(DEBUG_SYMBOLS) -O$(OPTIMIZATION) -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -ffunction-sections -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align $(DEVICE_LDFLAGS)  $(LTO)
# To allow unresolved symbols, uncomment
#LDFLAGS += -Wl,--warn-unresolved-symbols

#LDFLAGS += -Wl,--relax

#
# End of setup tools
##############################################################################################

##############################################################################################
# Start of file paths setup
#

# Directories where source files can be found

C_SOURCES   = $(foreach dir,$(SOURCE_DIRS),$(sort $(wildcard $(dir)/$(STAR).c)) )
CXX_SOURCES = $(foreach dir,$(SOURCE_DIRS),$(sort $(wildcard $(dir)/$(STAR).cpp)) )
ASM_SOURCES = $(foreach dir,$(SOURCE_DIRS),$(sort $(wildcard $(dir)/$(STAR).s $(dir)/$(STAR).S)) )

C_OBJECTS   := $(addsuffix .o,$(basename $(C_SOURCES)))
CXX_OBJECTS := $(addsuffix .o,$(basename $(CXX_SOURCES)))
ASM_OBJECTS := $(addsuffix .o,$(basename $(ASM_SOURCES)))
FIRST_LINK_OBJECTS = $(addsuffix .o,$(basename $(FIRST_LINK_SOURCES)))

INCLUDES = $(patsubst %,-I%,$(DEVICE_INCLUDE_DIRS) $(USER_INCLUDE_DIRS))
LIBS     = $(patsubst %,-l%,$(DEVICE_LIBS) $(USER_LIBS))
LIBDIR   = $(patsubst %,-L%,$(DEVICE_LIB_DIRS) $(USER_LIB_DIRS))

#
# End of file paths setup
##############################################################################################

##############################################################################################
# Start of RULES
#

ifneq ("$(DEVICE_NEEDS_HEX)","")
NEEDS_HEX = $(OUTPUT_BIN).hex
endif

all: $(OUTPUT_BIN).elf $(OUTPUT_BIN).bin $(OUTPUT_BIN).elf.txt $(NEEDS_HEX) $(SPECIAL_ATMEL_STUDIO_DEFAULT_TARGETS) $(ALL_ADDITIONS)
	@echo $(START_BOLD)Build ${GIT_EXACT_VERSION} "${GIT_VERSION}"$(END_BOLD)

ALL_OTHER_C_SOURCES :=
ALL_OTHER_CXX_SOURCES :=

$(eval $(DEVICE_RULES))

ifeq ($(VERBOSE),2)
$(info DEVICE_INCLUDE_DIRS+USER_INCLUDE_DIRS:$(patsubst %,$(NEWLINE_TAB)%,$(DEVICE_INCLUDE_DIRS) $(USER_INCLUDE_DIRS)))
$(info $(NEWLINE_ONLY)DEVICE_LIB_DIRS+USER_LIB_DIRS: $(patsubst %,$(NEWLINE_TAB)%,$(DEVICE_LIB_DIRS) $(USER_LIB_DIRS)))
$(info $(NEWLINE_ONLY)SOURCE_DIRS: $(patsubst %,$(NEWLINE_TAB)%,$(SOURCE_DIRS)))
$(info $(NEWLINE_ONLY)C_SOURCES full: $(patsubst %,$(NEWLINE_TAB)%,$(ALL_OTHER_C_SOURCES) $(C_SOURCES)))
$(info $(NEWLINE_ONLY)CXX_SOURCES full: $(patsubst %,$(NEWLINE_TAB)%,$(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES)))
$(info $(NEWLINE_ONLY)FIRST_LINK_SOURCES: $(patsubst %,$(NEWLINE_TAB)%,$(FIRST_LINK_SOURCES)))
endif

# We use tools as a "macro" for dependencies later
tools: | $(TOOLS_PATH)/$(TOOLS_SUBPATH)/bin

# and make it depend on the tools makefile to catch updates
$(TOOLS_PATH)/$(TOOLS_SUBPATH)/bin: $(TOOLS_PATH)/Makefile
	@echo Installing the necessary tools...
	cd ${TOOLS_PATH} && make "ARCH=gcc-${CROSS_COMPILE}" "TOOLS_VERSION=$(TOOLS_VERSION)"

OUTDIR = $(OBJ)

FIRST_LINK_OBJECTS_PATHS := $(addprefix $(OUTDIR)/,$(subst $(MOTATE_PATH),motate,$(FIRST_LINK_OBJECTS)))

MOTATE_C_OBJECTS   := $(addprefix $(OUTDIR)/,$(subst $(MOTATE_PATH),motate,$(filter $(MOTATE_PATH)/%,$(C_OBJECTS))))
MOTATE_CXX_OBJECTS := $(addprefix $(OUTDIR)/,$(subst $(MOTATE_PATH),motate,$(filter $(MOTATE_PATH)/%,$(CXX_OBJECTS))))
MOTATE_ASM_OBJECTS := $(addprefix $(OUTDIR)/,$(subst $(MOTATE_PATH),motate,$(filter $(MOTATE_PATH)/%,$(ASM_OBJECTS))))

ALL_OTHER_C_OBJECTS   := $(addprefix $(OUTDIR)/,$(filter-out $(MOTATE_PATH)/%,$(C_OBJECTS)))
ALL_OTHER_CXX_OBJECTS := $(addprefix $(OUTDIR)/,$(filter-out $(MOTATE_PATH)/%,$(CXX_OBJECTS)))
ALL_OTHER_ASM_OBJECTS := $(addprefix $(OUTDIR)/,$(filter-out $(MOTATE_PATH)/%,$(ASM_OBJECTS)))

ALL_C_OBJECTS   := $(MOTATE_C_OBJECTS) $(ALL_OTHER_C_OBJECTS)
ALL_CXX_OBJECTS := $(MOTATE_CXX_OBJECTS) $(ALL_OTHER_CXX_OBJECTS)
ALL_ASM_OBJECTS := $(MOTATE_ASM_OBJECTS) $(ALL_OTHER_ASM_OBJECTS)


#
CFLAGS   += $(DEBUG_SYMBOLS) -O$(OPTIMIZATION) $(INCLUDES) $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))
CPPFLAGS += $(DEBUG_SYMBOLS) -O$(OPTIMIZATION) $(INCLUDES) $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))
ASFLAGS  += $(DEBUG_SYMBOLS) -O$(OPTIMIZATION) $(INCLUDES) -D__ASSEMBLY__ $(patsubst %,-D%,$(DEVICE_DEFINES) $(USER_DEFINES))

ifneq ("$(CUSTOM_LINKER_SCRIPT)", "")
$(info Using custom linker script!)
LINKER_SCRIPT := $(CUSTOM_LINKER_SCRIPT)
LINKER_SCRIPT_TEXT = $(LINKER_SCRIPT)
LINKER_SCRIPT_OPTION = -T"$(LINKER_SCRIPT)"
else
ifneq ("$(DEVICE_LINKER_SCRIPT)", "")
LINKER_SCRIPT := $(DEVICE_LINKER_SCRIPT)
LINKER_SCRIPT_TEXT = $(LINKER_SCRIPT)
LINKER_SCRIPT_OPTION = -T"$(LINKER_SCRIPT)"
else
ABS_LINKER_SCRIPT =
ABS_LINKER_SCRIPT_TEXT = "[built in]"
LINKER_SCRIPT_OPTION =
endif
endif

# Generate dependency information
DEPFLAGS = -MMD -MF $(OBJ)/dep/$(@F).d -MT $(subst $(OUTDIR),$(OBJ),$@)


UNITY_BUILD ?= 0
ifeq ($(UNITY_BUILD),1)

$(info Single Translation Unit Build (STUB, AKA Unity Build))

# This little dance is to make the sources land in this order:
# 1- Motate
#    1.1- first-link then non-first-link
# 2- Non-Motate
#    2.1- first-link then non-first-link

# The C sources have to be wrapped in extern "C" { ... } and can safely always go LAST, so we handle that as well.

MOTATE_C_SOURCES := $(filter $(MOTATE_PATH)/%,$(filter-out $(FIRST_LINK_SOURCES), $(ALL_OTHER_C_SOURCES) $(C_SOURCES)) $(filter $(FIRST_LINK_SOURCES), $(ALL_OTHER_C_SOURCES) $(C_SOURCES)))
NON_MOTATE_C_SOURCES := $(filter-out $(MOTATE_PATH)/%,$(filter-out $(FIRST_LINK_SOURCES), $(ALL_OTHER_C_SOURCES) $(C_SOURCES)) $(filter $(FIRST_LINK_SOURCES), $(ALL_OTHER_C_SOURCES) $(C_SOURCES)))
MOTATE_CXX_SOURCES := $(filter $(MOTATE_PATH)/%,$(filter-out $(FIRST_LINK_SOURCES), $(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES)) $(filter $(FIRST_LINK_SOURCES), $(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES)))
NON_MOTATE_CXX_SOURCES := $(filter-out $(MOTATE_PATH)/%,$(filter-out $(FIRST_LINK_SOURCES), $(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES)) $(filter $(FIRST_LINK_SOURCES), $(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES)))

SORTED_C_SOURCES := $(filter-out $(LAST_C_SOURCES), $(MOTATE_C_SOURCES) $(NON_MOTATE_C_SOURCES)) $(LAST_C_SOURCES)
SORTED_CXX_SOURCES := $(MOTATE_CXX_SOURCES) $(NON_MOTATE_CXX_SOURCES)

# Also of note are the calls to $(REC) which are to populate the compile_commands.json file, for intelligent IDE navigation

$(OUTDIR)/all.cpp: $(ALL_OTHER_C_SOURCES) $(ALL_OTHER_CXX_SOURCES) $(C_SOURCES) $(CXX_SOURCES) | tools $(DEPDIR) $(BIN)
	$(shell echo "#define SINGLE_TRANSLATION_BUILD" > $(OUTDIR)/all.cpp)
	$(foreach f,$(SORTED_CXX_SOURCES),$(shell echo "#include \"../../$(f)\"" >> $(OUTDIR)/all.cpp ))
	$(foreach f,$(SORTED_CXX_SOURCES),$(shell $(REC) $(realpath $(CWD)) $(realpath $(CWD)) $(f) $(CXX) "$(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $(f).o $(f)" ))
	$(shell echo "extern \"C\" {" >> $(OUTDIR)/all.cpp)
	$(foreach f,$(SORTED_C_SOURCES),$(shell echo "#include \"../../$(f)\"" >> $(OUTDIR)/all.cpp ))
	$(foreach f,$(SORTED_C_SOURCES),$(shell $(REC) $(realpath $(CWD)) $(realpath $(CWD)) $(f) $(CC) "$(CFLAGS) $(DEPFLAGS) -c -o $(f).o $(f)" ))
	$(shell echo "}" >> $(OUTDIR)/all.cpp)

$(OUTPUT_BIN).elf: $(OUTDIR)/all.o $(ALL_ASM_OBJECTS) $(LINKER_SCRIPT) | $(ALL_OTHER_C_SOURCES) $(ALL_OTHER_CXX_SOURCES) $(C_SOURCES) $(CXX_SOURCES)
	@echo $(START_BOLD)"Linking $(OUTPUT_BIN).elf" $(END_BOLD)
	@echo $(START_BOLD)"Using linker script: $(LINKER_SCRIPT)" $(END_BOLD)
	$(QUIET)$(CXX) $(LIB_PATH) $(LINKER_SCRIPT_OPTION) $(LIBDIR) -Wl,-Map,"$(OUTPUT_BIN).map" -o $@ $(LDFLAGS) $(LD_OPTIONAL) -Wl,--start-group $(OUTDIR)/all.o $(LIBS) -Wl,--end-group
	$(QUIET)$(OBJCOPY) -O binary "$(OUTPUT_BIN).elf" "$(OUTPUT_BIN).bin"
	@echo "--- SIZE INFO ---"
	$(QUIET)$(SIZE) "$(OUTPUT_BIN).elf"

else

compile_commands.json: $(ALL_OTHER_C_SOURCES) $(ALL_OTHER_CXX_SOURCES) $(C_SOURCES) $(CXX_SOURCES) | tools $(DEPDIR) $(BIN)
	$(foreach f,$(ALL_OTHER_CXX_SOURCES) $(CXX_SOURCES),$(shell $(REC) $(realpath $(CWD)) $(realpath $(CWD)) $(f) $(CXX) "$(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $(f).o $(f)" ))
	$(foreach f,$(ALL_OTHER_C_SOURCES) $(C_SOURCES),$(shell $(REC) $(realpath $(CWD)) $(realpath $(CWD)) $(f) $(CC) "$(CFLAGS) $(DEPFLAGS) -c -o $(f).o $(f)" ))

$(OUTPUT_BIN).elf: compile_commands.json $(ALL_C_OBJECTS) $(ALL_CXX_OBJECTS) $(ALL_ASM_OBJECTS) $(LINKER_SCRIPT)
	@echo $(START_BOLD)"Linking $(OUTPUT_BIN).elf" $(END_BOLD)
	@echo $(START_BOLD)"Using linker script: $(LINKER_SCRIPT)" $(END_BOLD)
	$(QUIET)$(CXX) $(LIB_PATH) $(LINKER_SCRIPT_OPTION) $(LIBDIR) -Wl,-Map,"$(OUTPUT_BIN).map" -o ${filter-out tools,$@} $(LDFLAGS) $(LD_OPTIONAL) -Wl,--start-group $(FIRST_LINK_OBJECTS_PATHS) $(filter-out $(FIRST_LINK_OBJECTS_PATHS) $(LINKER_SCRIPT) tools compile_commands.json,$+) $(LIBS) -Wl,--end-group
	@echo "--- SIZE INFO ---"
	$(QUIET)$(SIZE) "$(OUTPUT_BIN).elf"

endif

$(OUTPUT_BIN).elf.txt: $(OUTPUT_BIN).elf
	@echo $(START_BOLD)"Exporting symbols $(OUTPUT_BIN).elf.txt" $(END_BOLD)
	$(QUIET)$(NM) "$(OUTPUT_BIN).elf" >"$(OUTPUT_BIN).elf.txt"

$(OUTPUT_BIN).bin: $(OUTPUT_BIN).elf
	@echo $(START_BOLD)"Making binary $(OUTPUT_BIN).bin" $(END_BOLD)
	$(QUIET)$(OBJCOPY) -O binary "$(OUTPUT_BIN).elf" "$(OUTPUT_BIN).bin"

$(OUTPUT_BIN).hex: $(OUTPUT_BIN).elf
	$(QUIET)$(OBJCOPY) -O ihex $(DEVICE_HEX_FLAGS) $< $@

## Note: The motate paths are seperated due to MOTATE_PATH having multple ../ in it.

$(MOTATE_CXX_OBJECTS): | tools $(sort $(dir $(MOTATE_CXX_OBJECTS))) $(DEPDIR) $(BIN)
$(MOTATE_CXX_OBJECTS): $(OUTDIR)/motate/%.o: $(MOTATE_PATH)/%.cpp
	@echo $(START_BOLD)"Compiling cpp $<"; echo "    -> $@" $(END_BOLD)
	$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CXX) "$(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $@ $<"
	$(QUIET)$(CXX) $(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $@ $<

$(ALL_OTHER_CXX_OBJECTS): | tools $(sort $(dir $(ALL_OTHER_CXX_OBJECTS))) $(DEPDIR) $(BIN)
$(ALL_OTHER_CXX_OBJECTS): $(OUTDIR)/%.o: %.cpp
	@echo $(START_BOLD)"Compiling cpp $<"; echo "    -> $@" $(END_BOLD)
	$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CXX) "$(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $@ $<"
	$(QUIET)$(CXX) $(CPPFLAGS) $(DEPFLAGS) -xc++ -c -o $@ $<

$(MOTATE_C_OBJECTS): | tools $(sort $(dir $(MOTATE_C_OBJECTS))) $(DEPDIR) $(BIN)
$(MOTATE_C_OBJECTS): $(OUTDIR)/motate/%.o: $(MOTATE_PATH)/%.c
	@echo $(START_BOLD)"Compiling c $<"; echo "    -> $@" $(END_BOLD)
	@$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CC) "$(CFLAGS) $(DEPFLAGS) -c -o $@ $<"
	$(QUIET)$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(ALL_OTHER_C_OBJECTS): | tools $(sort $(dir $(ALL_OTHER_C_OBJECTS))) $(DEPDIR) $(BIN)
$(ALL_OTHER_C_OBJECTS): $(OUTDIR)/%.o: %.c
	@echo $(START_BOLD)"Compiling c $<"; echo "    -> $@" $(END_BOLD)
	@$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CC) "$(CFLAGS) $(DEPFLAGS) -c -o $@ $<"
	$(QUIET)$(CC) $(CFLAGS) $(DEPFLAGS) -c -o $@ $<

$(MOTATE_ASM_OBJECTS): | tools $(sort $(dir $(MOTATE_ASM_OBJECTS))) $(DEPDIR) $(BIN)
$(MOTATE_ASM_OBJECTS): $(OUTDIR)/motate/%.o: $(MOTATE_PATH)/%.s
	@echo $(START_BOLD)"Compiling $<"; echo "    -> $@"  $(END_BOLD)
	@$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CC) "$(ASFLAGS) $(DEPFLAGS) -c -o $@ $<"
	$(QUIET)$(CC) $(ASFLAGS) $(DEPFLAGS) -c -o $@ $<

$(ALL_OTHER_ASM_OBJECTS): | tools $(sort $(dir $(ALL_OTHER_ASM_OBJECTS))) $(DEPDIR) $(BIN)
$(ALL_OTHER_ASM_OBJECTS): $(OUTDIR)/%.o: %.s
	@echo $(START_BOLD)"Compiling $<"; echo "    -> $@"  $(END_BOLD)
	@$(REC) $(realpath $(CWD)) $(realpath $(CWD)) $< $(CC) "$(ASFLAGS) $(DEPFLAGS) -c -o $@ $<"
	$(QUIET)$(CC) $(ASFLAGS) $(DEPFLAGS) -c -o $@ $<


# Special rule that can be added by ./board/*.mk files
$(eval $(ADDITIONAL_RECIPES))

# Rules to make the directories:
$(sort $(dir $(MOTATE_CXX_OBJECTS) $(ALL_OTHER_CXX_OBJECTS) $(MOTATE_C_OBJECTS) $(ALL_OTHER_C_OBJECTS) $(MOTATE_ASM_OBJECTS) $(ALL_OTHER_ASM_OBJECTS))):
	$(QUIET)$(MKDIR) -p "$@"

$(DEPDIR) $(BIN):
	$(QUIET)$(MKDIR) -p "$@"


# Rule for debugging with the J-Link
# Best to start the server with make ... debugjs first, then run make ... debugj
.PHONY: debugj debuggyj debugjs debugjs2 debugj-kill debugj-murder
debugjs2:
	JLinkGDBServer -device "${JLINK_DEVICE}" -log jlinkgdbserver.log -silent -halt -vd -if SWD -excdbg 4 -nogui -speed 2000 -ir

debugjs:
	$(MAKE) debugjs2 &

debugj: $(OUTPUT_BIN).elf
	$(GDB) -ex "dir '${MOTATE_PATH}/arch/'" -x "${BOARD_PATH}.gdb" -x "${MOTATE_PATH}/platform/jlink.gdb" -readnow -se "$(OUTPUT_BIN).elf"

# use the GDB with python enabled
debuggyj: $(OUTPUT_BIN).elf
	$(GDB_PY) -ex "dir '${MOTATE_PATH}/arch/'" -x "${BOARD_PATH}.gdb" -x "${MOTATE_PATH}/platform/jlink.gdb" -readnow -se "$(OUTPUT_BIN).elf"

# when done debugging, run this
debugj-kill:
	killall JLinkGDBServer

# if the debugger gets "jammed", this'll kill it
debugj-murder:
	killall -9 JLinkGDBServer


# Rules for debugging with the Atmel-ICE
.PHONY: debugi debuggyi debugis debugis2 debugi-kill debugi-murder
debugis2:
	openocd -c "set CHIPNAME ${CHIP}" -f ${MOTATE_PATH}/openocd.cfg -f ${OPENOCD_CFG} -c "gdb_port 2331; log_output openocd.log"

debugis:
	$(MAKE) debugis2 &

debugi: $(OUTPUT_BIN).elf
	$(GDB) -ex "dir '${MOTATE_PATH}/arch/'" -x "${BOARD_PATH}.gdb" -x "${MOTATE_PATH}/platform/atmel_ice.gdb" -readnow -se "$(OUTPUT_BIN).elf"

# Rule for debugging with the Atmel-ICE
debuggyi: $(OUTPUT_BIN).elf
	$(GDB_PY) -ex "dir '${MOTATE_PATH}/arch/'" -x "${BOARD_PATH}.gdb" -x "${MOTATE_PATH}/platform/atmel_ice.gdb" -readnow -se "$(OUTPUT_BIN).elf"

# when done debugging, run this
debugi-kill:
	killall openocd

# if the debugger gets "jammed", this'll kill it
debugi-murder:
	killall -9 openocd

flash: $(FLASH_REQUIRES)
	$(DEVICE_FLASH_CMD)

#-------------------------------------------------------------------------------
#		Default $(PROJECT).elf, and clean
#-------------------------------------------------------------------------------

$(PROJECT).elf: $(OUTPUT_BIN).elf
	$(CP) $< $@

$(PROJECT).map: $(OUTPUT_BIN).map
	$(CP) $< $@

$(PROJECT).hex: $(OUTPUT_BIN).elf
	$(QUIET)$(OBJCOPY) -O ihex $(DEVICE_HEX_FLAGS) $< $@

$(PROJECT).bin: $(OUTPUT_BIN).elf
	$(QUIET)$(OBJCOPY) -O binary $< $@

ifneq (WIN32,${UNAME})

clean:
	-$(RM) -fR $(OBJ)
	-$(RM) -fR $(BIN)
	-$(RM) -fR $(BOARD).elf $(BOARD).map $(BOARD).hex $(BOARD).bin
	-$(RM) -fR compile_commands.json $(MOTATE_PATH)/compile_commands.json

else

clean:
	-find  $(OBJ)/ -d -print0 -name \* | xargs -0 rm -rf
	-find  $(BIN)/ -d -print0 -name \* | xargs -0 rm -rf
	-$(RM) -fR $(BOARD).elf $(BOARD).map $(BOARD).hex $(BOARD).bin
	-$(RM) -fR compile_commands.json $(MOTATE_PATH)/compile_commands.json

endif

#
# Include the dependency files, should be the last of the makefile
#
-include $(shell mkdir $(OBJ)/dep 2>/dev/null) $(wildcard $(OBJ)/dep/*.d)

#
# End of RULES
##############################################################################################

# *** EOF ***
