MKDIR = mkdir -p

ARCH ?= gcc-arm-none-eabi

ifneq (,$(findstring /cygdrive/,$(PATH)))
OS := WIN32
else
ifneq (,$(findstring WINDOWS,$(PATH)))
OS := WIN32
else
UNAME := $(shell uname -s)

ifeq (Darwin,${UNAME})
OS = OSX
DOWNLOAD_TOOL = curl -OL
else
ifeq (Linux,${UNAME})
OS = LINUX
DOWNLOAD_TOOL = wget
endif #LINUX
endif #Darwin

endif #cygdrive
endif #WINDOWS


ifeq ($(ARCH), gcc-arm-none-eabi)
TOOLS_SUBPATH = gcc-arm-none-eabi

TOOLS_VERSION_SHORT = 4.9
TOOLS_VERSION_LONG = 4.9-2015-q2-update
TOOLS_VERSION_FILENAME = 4_9-2015q2-20150609
TOOLS_EXPANDED_DIRNAME = gcc-arm-none-eabi-4_9-2015q2

TOOLS_URL_PREFIX = https://launchpad.net/gcc-arm-embedded/${TOOLS_VERSION_SHORT}/${TOOLS_VERSION_LONG}/+download/
TOOLS_OSX_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-mac.tar.bz2
TOOLS_LINUX_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-linux.tar.bz2
TOOLS_WIN32_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-win32.zip

EXPAND_COMMAND = tar xjf

TEST_COMMAND=arm-none-eabi-gcc

endif #eq ($(ARCH), 'gcc-arm-none-eabi')



ifeq ($(ARCH), gcc-avr)
TOOLS_SUBPATH = gcc-avr

TOOLS_VERSION = 4.8.1-arduino3
TOOLS_EXPANDED_DIRNAME = avr

#http://arduino.cc/download.php?f=/tools/avr-gcc-4.8.1-arduino3-i386-apple-darwin11.tar.bz2
TOOLS_URL_PREFIX = http://arduino.cc/download.php?f=/tools/

TOOLS_OSX_ARCHIVE_FILENAME = avr-gcc-${TOOLS_VERSION}-i386-apple-darwin11.tar.bz2
EXPAND_COMMAND_OSX = tar xzvf

TOOLS_LINUX_ARCHIVE_FILENAME = avr-${TOOLS_VERSION}-x86_64-pc-linux-gnu.tar.bz2
EXPAND_COMMAND_LINUX = tar xzvf

TOOLS_WIN32_ARCHIVE_FILENAME = avr-${TOOLS_VERSION}-i686-mingw32.zip
EXPAND_COMMAND_WIN32 = unzip -d ${TOOLS_EXPANDED_DIRNAME}

EXPAND_COMMAND = ${EXPAND_COMMAND_${OS}}

TEST_COMMAND=avr-gcc

endif #eq ($(ARCH), 'gcc-arm-none-eabi')



ifeq ($(shell PATH=$(TOOLS_SUBPATH)/bin/:${PATH} command -v $(TEST_COMMAND) >/dev/null 2>&1 || echo "yes"), yes)
TOOLS_SUBPATH_IF_NEEDED = $(TOOLS_SUBPATH)
endif


all: TOOLS_INSTALLED

.PHONY:
TOOLS_INSTALLED: ${TOOLS_SUBPATH_IF_NEEDED}


.ONESHELL:
${TOOLS_SUBPATH}: ${TOOLS_EXPANDED_DIRNAME}
	ln -s "${TOOLS_EXPANDED_DIRNAME}" "${TOOLS_SUBPATH}"

${TOOLS_EXPANDED_DIRNAME}: ${TOOLS_${OS}_ARCHIVE_FILENAME}
	${EXPAND_COMMAND} "${TOOLS_${OS}_ARCHIVE_FILENAME}"

${TOOLS_${OS}_ARCHIVE_FILENAME}:
	$(info The $(ARCH) tools need installed. Downloading and installing now.)
	${DOWNLOAD_TOOL} "${TOOLS_URL_PREFIX}${TOOLS_${OS}_ARCHIVE_FILENAME}"

.PHONY: clean

clean:
	rm -rf ${TOOLS_EXPANDED_DIRNAME}
	rm ${TOOLS_${OS}_ARCHIVE_FILENAME}
	rm ${TOOLS_SUBPATH}
