# ----------------------------------------------------------------------------
# This file is part of the Motate project.
#
# Board definition for Adafruit Metro M4 AirLift Lite
# https://www.adafruit.com/product/4000
#
# MCU: ATSAMD51J19A
#   - ARM Cortex-M4F @ 120 MHz
#   - 512 KB Flash
#   - 192 KB SRAM
#   - Hardware FPU
#
# Features:
#   - Arduino-compatible pinout
#   - ESP32 WiFi co-processor (AirLift)
#   - USB-C connector
#   - SWD debug port

# Only process this file if the board wasn't already found by g2core's board files
# g2core has its own MetroM4AirLift.mk in g2core/board/ that sets _BOARD_FOUND = 1
ifneq ("$(_BOARD_FOUND)","1")

ifeq ("$(BOARD)","MetroM4AirLift")
    BASE_BOARD = metrom4airlift
    DEVICE_DEFINES += MOTATE_BOARD="MetroM4AirLift"
    DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=1
endif

ifeq ("$(BASE_BOARD)","metrom4airlift")
    _BOARD_FOUND = 1

    FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Atmel_samd51/*.cpp)

    CHIP = SAMD51J19A
    export CHIP
    CHIP_LOWERCASE = samd51j19a

    BOARD_PATH = ${MOTATE_PATH}/board/MetroM4AirLift
    DEVICE_INCLUDE_DIRS += ./board/MetroM4AirLift $(BOARD_PATH)

    PLATFORM_BASE = ${MOTATE_PATH}/platform/atmel_sam
    include $(PLATFORM_BASE).mk
endif

endif # _BOARD_FOUND check
