# ----------------------------------------------------------------------------
# This file is part of the Motate project.

ifeq ("$(BOARD)","g2Ref")
    BASE_BOARD = g2ref
    DEVICE_DEFINES += MOTATE_BOARD="g2Ref"
    DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=1
endif

ifeq ("$(BASE_BOARD)","g2ref")
    _BOARD_FOUND = 1

    FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Atmel_sam3xa/*.cpp)

    CHIP = SAM3X8C
    export CHIP
    CHIP_LOWERCASE = sam3x8c

    BOARD_PATH = ${MOTATE_PATH}/board/g2Ref
    DEVICE_INCLUDE_DIRS += ./board/g2Ref $(BOARD_PATH)

    PLATFORM_BASE = ${MOTATE_PATH}/platform/atmel_sam
    include $(PLATFORM_BASE).mk
endif
