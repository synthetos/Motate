# Setup for non-wrapped lines and non-pages prints
set width 0
set height 0

# Open and connect to openocd with the ATMEL-ICE
#target remote | openocd -f ../../motate/board/frdm-kl05z.cfg -c "adapter_khz 50" -c init -c halt
# Open and connect to openocd with the CMSIS-DAP
target remote | /usr/local/bin/openocd -c "set CHIPNAME ${CHIP}" -f ${MOTATE_PATH}/openocd.cfg -f ${MOTATE_PATH}/board/frdm-kl05z.cfg -c "gdb_port pipe; log_output openocd.log"


# Turn on history saving
set history save on

define reset
    monitor reset init
end

define flash
    make
    load
    reset
end

source -s arm.gdb
