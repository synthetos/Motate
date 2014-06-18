/*
 * main.cpp - Motate
 * This file is part of the Motate project.
 *
 * Copyright (c) 2013 - 2014 Alden S. Hart, Jr.
 * Copyright (c) 2013 - 2014 Robert Giseburt
 *
 *  This file is part of the Motate Library.
 *
 *  This file ("the software") is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2 as published by the
 *  Free Software Foundation. You should have received a copy of the GNU General Public
 *  License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  As a special exception, you may use this file as part of a software library without
 *  restriction. Specifically, if other files instantiate templates or use macros or
 *  inline functions from this file, or you compile this file and link it with  other
 *  files to produce an executable, this file does not by itself cause the resulting
 *  executable to be covered by the GNU General Public License. This exception does not
 *  however invalidate any other reasons why the executable file might be covered by the
 *  GNU General Public License.
 *
 *  THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 *  WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 *  SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. *
 */

#include "MotatePins.h"
#include "MotateTimers.h"
using Motate::delay;

/******************** External interface setup ************************/

#ifdef MOTATE_CONFIG_HAS_USBSERIAL
#include "MotateUSB.h"
#include "MotateUSBCDC.h"

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION 8
#endif

const Motate::USBSettings_t Motate::USBSettings = {
	/*gVendorID         = */ 0x1d50,
	/*gProductID        = */ 0x606d,
	/*gProductVersion   = */ FIRMWARE_VERSION,
	/*gAttributes       = */ kUSBConfigAttributeSelfPowered,
	/*gPowerConsumption = */ 500
};
	/*gProductVersion   = */ //0.1,

Motate::USBDevice< Motate::USBCDC > usb;
//Motate::USBDevice< Motate::USBCDC, Motate::USBCDC > usb;

typeof usb._mixin_0_type::Serial &SerialUSB = usb._mixin_0_type::Serial;
//typeof usb._mixin_1_type::Serial &SerialUSB1 = usb._mixin_1_type::Serial;

MOTATE_SET_USB_VENDOR_STRING( {'S' ,'y', 'n', 't', 'h', 'e', 't', 'o', 's'} )
MOTATE_SET_USB_PRODUCT_STRING( {'M', 'o', 't', 'a', 't', 'e', ' ', 'D' , 'e', 'm', 'o'} )
MOTATE_SET_USB_SERIAL_NUMBER_STRING( {'0','0','7'} )
#endif //MOTATE_CONFIG_HAS_USBSERIAL


#ifdef MOTATE_CONFIG_HAS_SPI
#include "MotateSPI.h"
Motate::SPI<kSocket4_SPISlaveSelectPinNumber> spi;
#endif

/******************** Initialization setup ************************/

void setup() __attribute__ ((weak));

extern void loop();

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

	// These two routines are defined with C linkage:

	void _init() __attribute__ ((weak));
	void _init() {;}

	void __libc_init_array(void);

#ifdef __cplusplus
}
#endif // __cplusplus


/*
 * _system_init()
 */

void _system_init(void)
{
	SystemInit();

	// WHAT!?!
	// Disable watchdog
	WDT->WDT_MR = WDT_MR_WDDIS;

	// Initialize C library
	__libc_init_array();

#ifdef MOTATE_CONFIG_HAS_USBSERIAL
	usb.attach();					// USB setup
#endif
}


/*
 * main()
 */

int main(void) {

	_system_init();

	if (setup)
		setup();

	// main loop
	for (;;) {
		loop();
	}
	return 0;
}
