#!/bin/bash

###### NEEDS UPDATED TO WORK WITH NON-OSX ######

arduinoAppDir=/Applications
PATH=$PATH:$arduinoAppDir/Arduino.app/Contents/Resources/Java/hardware/tools/g++_arm_none_eabi/bin/:$arduinoAppDir/Arduino.app/Contents/Resources/Java/hardware/tools/

BOSSAC=$arduinoAppDir/Arduino.app/Contents/Resources/Java/hardware/tools/bossac

function show_usage() {
	cat <<END
	USAGE: $0 -f firmware.elf -p <native port>
    
	Replacing <native port> with the device name of the Arduino Due native port.

	One of the following may be a valid choice:
	
END
	
	ls -1 /dev/tty.usbmodem* | xargs echo "          "
	
	cat <<END

	NOTE: You may be able to use /dev/tty.usbmodem* (with the star) if there is only one.
END
}

file=""
port=""
native_port="true"

while getopts h?f:p:P: flag; do
	case $flag in
		f)
			file=$OPTARG
			;;
		p)
			port=$OPTARG
			;;
		[h])
			show_usage;
			exit 0
			;;
	esac
done

if [[ ${file} == "" ]]; then
	echo "Error: Please specify a file to program. The name will most likely be 'TinyG.elf'."
	exit 1;
fi

if [[ ${port} == "" ]]; then
	echo "Error: Please specify a port with -p or -P. Currently visible ports:"
	ls -1 /dev/tty.usbmodem* | xargs echo "    "
	exit 1;
fi


arm-none-eabi-objcopy -O binary "${file}" "${file/.elf/.bin}" 

echo "Forcing reset using 1200bps open/close on port ${port}"

stty -f "${port}" 1200

sleep 1

echo "Starting programming of file ${file} -> ${file/.elf/.bin} on port ${port/\/dev\//}"
"${BOSSAC}" -e -w -v -b "${file/.elf/.bin}"

echo
echo "WARNING: You may need to hit the RESET button on the device at this point."

##----------- PROGRAMMING port
#Forcing reset using 1200bps open/close on port /dev/tty.usbmodem26231
#bossac --port=tty.usbmodem26231 -U false -e -w -v -b $tmp/BareMinimum.cpp.bin -R 