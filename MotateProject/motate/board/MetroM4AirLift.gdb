# Setup for non-wrapped lines and non-pages prints
set width 0
set height 0

# Open and connect to J-Link for SAMD51
target remote | JLinkGDBServer -device ATSAMD51J19A -if SWD -speed 4000 -port 2331

# Turn on history saving
set history save on
set history expansion on

set print pretty on

define reset
    monitor reset
end

define flash
    make
    load
    reset
end
