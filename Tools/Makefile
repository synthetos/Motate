MKDIR = mkdir -p

ARCH ?= gcc-arm-none-eabi

UNAME := $(shell uname -s)
${info Current UNAME: $(UNAME) }

ifeq (Darwin,${UNAME})
ifeq (x86_64,$(shell uname -m))
OS = OSX_INTEL
else
OS = OSX_APPLE
endif
else

ifeq (Linux,${UNAME})
ifeq (armv7l,$(shell uname -m))
# TODO this needs verified!
OS = LINUX_ARMV7
else
OS = LINUX
endif #LINUX ARM
endif #LINUX

endif #Darwin


# ARM GCC ------

ifeq ($(ARCH), gcc-arm-none-eabi)

ifeq ("$(TOOLS_VERSION)", "12.2r1")

TOOLS_VERSION_FILENAME = 12.2.rel1

TOOLS_OSX_INTEL_URL_PREFIX = https://github.com/synthetos/gcc-arm-none-eabi-archive/releases/download/12.2.Rel1/arm-gnu-toolchain-12.2.rel1-darwin-x86_64-arm-none-eabi.tar.xz
TOOLS_OSX_APPLE_URL_PREFIX = https://github.com/synthetos/gcc-arm-none-eabi-archive/releases/download/12.2.Rel1/arm-gnu-toolchain-12.2.rel1-darwin-arm64-arm-none-eabi.tar.xz
TOOLS_LINUX_URL_PREFIX = https://github.com/synthetos/gcc-arm-none-eabi-archive/releases/download/12.2.Rel1/arm-gnu-toolchain-12.2.rel1-linux-x86_64-arm-none-eabi.tar.xz
TOOLS_LINUX_ARMV7_URL_PREFIX = https://github.com/synthetos/gcc-arm-none-eabi-archive/releases/download/12.2.Rel1/arm-gnu-toolchain-12.2.rel1-linux-aarch64-arm-none-eabi.tar.xz
TOOLS_URL = ${TOOLS_${OS}_URL_PREFIX}

TOOLS_OSX_INTEL_ARCHIVE_FILENAME   = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-mac.tar.xz
TOOLS_OSX_APPLE_ARCHIVE_FILENAME   = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-mac.tar.xz
TOOLS_LINUX_ARCHIVE_FILENAME       = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-linux.tar.xz
TOOLS_LINUX_ARMV7_ARCHIVE_FILENAME = gcc-arm-none-eabi-${TOOLS_VERSION_FILENAME}-linux.tar.xz

TOOLS_OSX_INTEL_EXPANDED_DIRNAME    = arm-gnu-toolchain-${TOOLS_VERSION_FILENAME}-darwin-x86_64-arm-none-eabi
TOOLS_OSX_APPLE_EXPANDED_DIRNAME    = arm-gnu-toolchain-${TOOLS_VERSION_FILENAME}-darwin-arm64-arm-none-eabi
TOOLS_LINUX_EXPANDED_DIRNAME        = arm-gnu-toolchain-${TOOLS_VERSION_FILENAME}-x86_64-arm-none-eabi
TOOLS_LINUX_ARMV7_EXPANDED_DIRNAME  = arm-gnu-toolchain-${TOOLS_VERSION_FILENAME}-arm-none-eabi
TOOLS_EXPANDED_DIRNAME = ${TOOLS_${OS}_EXPANDED_DIRNAME}

TEST_COMMAND_BASE=arm-none-eabi-gcc

else

$(error The TOOLS_VERSION ($(TOOLS_VERSION)) requested is not a valid option. You project and Motate are mismatched. (?))

endif #$(TOOLS_VERSION)

TOOLS_EASYPATH = gcc-arm-none-eabi-$(TOOLS_VERSION)

endif #eq ($(ARCH), 'gcc-arm-none-eabi')

# all platforms from here on -----


EXPAND_COMMAND_OSX_INTEL = tar xf
TOOLS_SUBPATH_OSX_INTEL = osx
TEST_COMMAND_OSX_INTEL = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_OSX_INTEL = curl -L
DOWNLOAD_TOOL_SET_NAME_OSX_INTEL = -o

EXPAND_COMMAND_OSX_APPLE = tar xf
TOOLS_SUBPATH_OSX_APPLE = osx
TEST_COMMAND_OSX_APPLE = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_OSX_APPLE = curl -L
DOWNLOAD_TOOL_SET_NAME_OSX_APPLE = -o

EXPAND_COMMAND_LINUX = tar xf
EXPAND_COMMAND_LINUX = tar xf
TOOLS_SUBPATH_LINUX = linux
TEST_COMMAND_LINUX = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_LINUX = wget
DOWNLOAD_TOOL_SET_NAME_LINUX = -O

EXPAND_COMMAND_LINUX_ARMV7 = tar xf
EXPAND_COMMAND_LINUX_ARMV7 = tar xf
TOOLS_SUBPATH_LINUX_ARMV7 = linux
TEST_COMMAND_LINUX_ARMV7 = ${TEST_COMMAND_BASE}
DOWNLOAD_TOOL_LINUX_ARMV7 = wget
DOWNLOAD_TOOL_SET_NAME_LINUX_ARMV7 = -O

EXPAND_COMMAND = ${EXPAND_COMMAND_${OS}}
TOOLS_SUBPATH = ${TOOLS_SUBPATH_${OS}}
TEST_COMMAND = ${TEST_COMMAND_${OS}}
DOWNLOAD_TOOL = ${DOWNLOAD_TOOL_${OS}}
DOWNLOAD_TOOL_SET_NAME = ${DOWNLOAD_TOOL_SET_NAME_${OS}}


# NOTE: We set the PATH in the Motate.mk makefile that calls this!
# If it's missing stuff, then you're path is wrong!
#$(info PATH:$(PATH))

# Note the | -- order-only means we don't care about the modification date
TOOLS_FULL_EASYPATH_IF_NEEDED = | $(TOOLS_SUBPATH)/$(TOOLS_EASYPATH)

all: TOOLS_INSTALLED

.PHONY:
TOOLS_INSTALLED: ${TOOLS_FULL_EASYPATH_IF_NEEDED}

.ONESHELL:
$(TOOLS_SUBPATH)/$(TOOLS_EASYPATH): $(TOOLS_SUBPATH)/${TOOLS_EXPANDED_DIRNAME}
	cd $(TOOLS_SUBPATH) && \
	ln -s "${TOOLS_EXPANDED_DIRNAME}" "${TOOLS_EASYPATH}"

${TOOLS_SUBPATH}/${TOOLS_EXPANDED_DIRNAME}: ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}
	cd $(TOOLS_SUBPATH) && \
	rm -f "${TOOLS_EASYPATH}" && \
	${EXPAND_COMMAND} ${TOOLS_${OS}_ARCHIVE_FILENAME} && \
	rm -f "${TOOLS_${OS}_ARCHIVE_FILENAME}"


.INTERMEDIATE: ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}
${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME}:
	$(info The $(ARCH) tools need installed. Downloading ${TOOLS_SUBPATH}/${TOOLS_${OS}_ARCHIVE_FILENAME} and installing now.)
	mkdir -p $(TOOLS_SUBPATH) && \
	cd $(TOOLS_SUBPATH) && \
	${DOWNLOAD_TOOL} "${TOOLS_URL}" ${DOWNLOAD_TOOL_SET_NAME} "${TOOLS_${OS}_ARCHIVE_FILENAME}"

.PHONY: clean

clean:
	rm -rf ${TOOLS_SUBPATH}
