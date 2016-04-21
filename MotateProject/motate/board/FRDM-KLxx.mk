# ----------------------------------------------------------------------------
# This file is part of the Motate project.



ifeq ("$(BOARD)","FRDM-KL05Z")
	BASE_BOARD = frdm-kl05z
	DEVICE_DEFINES += MOTATE_BOARD="FRDM-KL05Z"
endif

ifeq ("$(BASE_BOARD)","frdm-kl05z")
	_BOARD_FOUND = 1
	
	FIRST_LINK_SOURCES += $(wildcard ${MOTATE_PATH}/Freescale_klxx/*.cpp)

	CHIP = MKL05Z32VFM4
    export CHIP
	CHIP_LOWERCASE = mkl05z32vfm4

	BOARD_PATH = ${MOTATE_PATH}/board/FRDM-KLxx
	DEVICE_INCLUDE_DIRS += $(BOARD_PATH)

    PLATFORM_BASE = ${MOTATE_PATH}/platform/freescale_klxx
	include $(PLATFORM_BASE).mk
endif
