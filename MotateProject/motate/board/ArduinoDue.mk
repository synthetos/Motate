# ----------------------------------------------------------------------------
# This file is part of the Motate project.

ifeq ("$(BOARD)","Due")
    BASE_BOARD = due
    DEVICE_DEFINES += MOTATE_BOARD="Due"
    DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=1
endif

ifeq ("$(BASE_BOARD)","due")
    _BOARD_FOUND = 1

    FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Atmel_sam3x/*.cpp)

    CHIP = SAM3X8E
    export CHIP
    CHIP_LOWERCASE = sam3x8e

    BOARD_PATH = ${MOTATE_PATH}/board/ArduinoDue
    DEVICE_INCLUDE_DIRS += ./board/ArduinoDue $(BOARD_PATH)

    PLATFORM_BASE = ${MOTATE_PATH}/platform/atmel_sam
    include $(PLATFORM_BASE).mk
endif
