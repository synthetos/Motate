# Setup for non-wrapped lines and non-pages prints
set width 0
set height 0

# Open and connect to openocd with the ATMEL-ICE
target remote | openocd -f ../../motate/board/frdm-kl05z.cfg -c "adapter_khz 50" -c init -c halt 

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