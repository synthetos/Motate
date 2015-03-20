# ----------------------------------------------------------------------------
# This file is part of the Motate project. It is heavily modified from a file
# that contained this original copyright:

# ----------------------------------------------------------------------------
#         ATMEL Microcontroller Software Support
# ----------------------------------------------------------------------------
# Copyright (c) 2010, Atmel Corporation
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following condition is met:
#
# - Redistributions of source code must retain the above copyright notice,
# this list of conditions and the disclaimer below.
#
# Atmel's name may not be used to endorse or promote products derived from
# this software without specific prior written permission.
#
# DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
# DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
# OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
# LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
# NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
# EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# ----------------------------------------------------------------------------

ifeq ("$(BOARD)","TinyGv8")
    BASE_BOARD = xmega
    DEVICE_DEFINES += MOTATE_BOARD="TinyGv8"
    DEVICE_DEFINES += MOTATE_CONFIG_HAS_USBSERIAL=0
    DEVICE_DEFINES += F_CPU=32000000UL
    DEVICE_DEFINES += __CLOCK_EXTERNAL_16MHZ=1
endif

ifeq ("$(BASE_BOARD)","xmega")
    _BOARD_FOUND = 1

    FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Atmel_XMega/*.cpp)

    CHIP = atxmega192a3
    CHIP_LOWERCASE = atxmega192a3

    BOARD_PATH = ${MOTATE_PATH}/board/TinyGv8
    DEVICE_INCLUDE_DIRS += $(BOARD_PATH)

    PLATFORM_BASE = ${MOTATE_PATH}/platform/atmel_xmega
    include $(PLATFORM_BASE).mk
endif
