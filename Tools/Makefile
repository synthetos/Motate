MKDIR = mkdir -p

ARCH ?= gcc-arm-none-eabi

ifneq (,$(findstring /cygdrive/,$(PATH)))
OS := WIN32
else
ifneq (,$(findstring WINDOWS,$(PATH)))
OS := WIN32
else
ifneq (,$(findstring Atmel Studio,$(PATH)))
OS := WIN32
else
UNAME := $(shell uname -s)

ifeq (Darwin,${UNAME})
OS = OSX
else
ifeq (Linux,${UNAME})
OS = LINUX
endif #LINUX
endif #Darwin

endif #Atmel Studio
endif #WINDOWS
endif #cygdrive


ifeq ($(ARCH), gcc-arm-none-eabi)

TOOLS_EASYPATH = gcc-arm-none-eabi

TOOLS_VERSION_SHORT = 5.0
TOOLS_VERSION_LONG = 5-2016-q2-update
TOOLS_VERSION_FILENAME = 5_4-2016q2-20160622
TOOLS_EXPANDED_DIRNAME = gcc-arm-none-eabi-5_4-2016q2

# Example:        https://launchpad.net/gcc-arm-embedded/5.0/5-2016-q2-update/+download/gcc-arm-none-eabi-5_4-2016q2-20160622-win32.exe
# Expands to: gcc-arm-none-eabi-5_3-2016q1

TOOLS_URL_PREFIX = https://launchpad.net/gcc-arm-embedded/${TOOLS_VERSION_SHORT}/${TOOLS_VERSION_LONG}/+download/
TOOLS_OSX_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-mac.tar.bz2
TOOLS_LINUX_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-linux.tar.bz2
TOOLS_WIN32_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-win32.zip

TEST_COMMAND_BASE=arm-none-eabi-gcc

endif #eq ($(ARCH), 'gcc-arm-none-eabi')



ifeq ($(ARCH), gcc-avr)

TOOLS_EASYPATH = gcc-avr

TOOLS_VERSION = 4.8.1-arduino3
TOOLS_EXPANDED_DIRNAME = avr

#http://arduino.cc/download.php?f=/tools/avr-gcc-4.8.1-arduino3-i386-apple-darwin11.tar.bz2
TOOLS_URL_PREFIX = http://arduino.cc/download.php?f=/tools/

TOOLS_OSX_ARCHIVE_FILENAME = avr-gcc-${TOOLS_VERSION}-i386-apple-darwin11.tar.bz2
TOOLS_LINUX_ARCHIVE_FILENAME = avr-${TOOLS_VERSION}-x86_64-pc-linux-gnu.tar.bz2
TOOLS_WIN32_ARCHIVE_FILENAME = avr-${TOOLS_VERSION}-i686-mingw32.zip

TEST_COMMAND_BASE=avr-gcc

endif #eq ($(ARCH), 'gcc-avr')



EXPAND_COMMAND_OSX = tar xjf
TOOLS_SUBPATH_OSX = osx
TEST_COMMAND_OSX = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_OSX = curl -OL

EXPAND_COMMAND_LINUX = tar xjf
TOOLS_SUBPATH_LINUX = linux
TEST_COMMAND_LINUX = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_LINUX = wget

EXPAND_COMMAND_WIN32 = 7za/7za x
TOOLS_SUBPATH_WIN32 = win32
TEST_COMMAND_WIN32 = ${TEST_COMMAND_BASE}.exe
DOWNLOAD_TOOL_WIN32 = curl -OL

EXPAND_COMMAND = ${EXPAND_COMMAND_${OS}}
TOOLS_SUBPATH = ${TOOLS_SUBPATH_${OS}}
TEST_COMMAND = ${TEST_COMMAND_${OS}}
DOWNLOAD_TOOL = ${DOWNLOAD_TOOL_${OS}}

# NOTE: We set the PATH in the Motate.mk makefile that calls this!
# If it's missing stuff, then you're path is wrong!
#$(info PATH:$(PATH))

# Note the | -- order-only means we don't care about the modification date
TOOLS_FULL_EASYPATH_IF_NEEDED = | $(TOOLS_SUBPATH)/$(TOOLS_EASYPATH)

all: TOOLS_INSTALLED

.PHONY:
TOOLS_INSTALLED: ${TOOLS_FULL_EASYPATH_IF_NEEDED}


ifneq ($(OS), WIN32)

.ONESHELL:
$(TOOLS_SUBPATH)/$(TOOLS_EASYPATH): $(TOOLS_SUBPATH)/${TOOLS_EXPANDED_DIRNAME}
	cd $(TOOLS_SUBPATH) && \
	ln -s "${TOOLS_EXPANDED_DIRNAME}" "${TOOLS_EASYPATH}"

${TOOLS_SUBPATH}/${TOOLS_EXPANDED_DIRNAME}: ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}
	cd $(TOOLS_SUBPATH) && \
	rm -f "${TOOLS_EASYPATH}" && \
	${EXPAND_COMMAND} ${TOOLS_${OS}_ARCHIVE_FILENAME}

else

## Additional test added to make sure we have curl.
## Otherwise, windows will NOT return non-zero for these commands and some wierd
## failure will happen later.

ifeq ($(shell command -v curl >/dev/null 2>&1 || echo "missing"), missing)
${info On windows git must be installed for the curl command. Please download and install from http://git-scm.com/downloads }
${info Current PATH: $(PATH) }
${error If you called this makefile directly (instead of from compiling Motate, you will get this error. Got back tot he Motate project you want to compile and `make tools`.) }
endif

# end additional curl test



.ONESHELL:
${TOOLS_SUBPATH}/$(TOOLS_EASYPATH): ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}
	mkdir -p $(TOOLS_SUBPATH) && \
	cd $(TOOLS_SUBPATH) && \
	../${EXPAND_COMMAND} -o$(TOOLS_EASYPATH) "${TOOLS_${OS}_ARCHIVE_FILENAME}"

endif # if not windows/else


${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}:
	$(info The $(ARCH) tools need installed. Downloading ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME} and installing now.)
	mkdir -p $(TOOLS_SUBPATH) && \
	cd $(TOOLS_SUBPATH) && \
	${DOWNLOAD_TOOL} "${TOOLS_URL_PREFIX}${TOOLS_${OS}_ARCHIVE_FILENAME}"

.PHONY: clean

clean:
	rm -rf ${TOOLS_SUBPATH}
