/*
  MotateSerial.h - Serial (UART or USB) library for the Motate system
  http://github.com/synthetos/motate/

  Copyright (c) 2013 Robert Giseburt

	This file is part of the Motate Library.

	This file ("the software") is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License, version 2 as published by the
	Free Software Foundation. You should have received a copy of the GNU General Public
	License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.

	As a special exception, you may use this file as part of a software library without
	restriction. Specifically, if other files instantiate templates or use macros or
	inline functions from this file, or you compile this file and link it with  other
	files to produce an executable, this file does not by itself cause the resulting
	executable to be covered by the GNU General Public License. This exception does not
	however invalidate any other reasons why the executable file might be covered by the
	GNU General Public License.

	THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
	WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
	OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
	SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
	OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MOTATESERIAL_H_ONCE
#define MOTATESERIAL_H_ONCE

#include <cinttypes>

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
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

Motate::USBDevice< Motate::USBCDC > usb;
//Motate::USBDevice< Motate::USBCDC, Motate::USBCDC > usb;

namespace Motate {

    auto &Serial = usb.mixin<0>::Serial;
//  auto &Serial1 = usb.mixin<1>::Serial;

} // namespace Motate

MOTATE_SET_USB_VENDOR_STRING( {'S' ,'y', 'n', 't', 'h', 'e', 't', 'o', 's'} )
MOTATE_SET_USB_PRODUCT_STRING( {'M', 'o', 't', 'a', 't', 'e', ' ', 'D' , 'e', 'm', 'o'} )
MOTATE_SET_USB_SERIAL_NUMBER_STRING_FROM_CHIPID()

struct _USBAttacher {
    _USBAttacher() {
        usb.attach();
    };
};

static _USBAttacher _attacher;

#else

#include "MotateUART.h"

namespace Motate {
    // XonXoffFlowControl
    BufferedUART<kSerial_RX, kSerial_TX, kSerial_RTS, kSerial_CTS> Serial {115200, UARTMode::RTSCTSFlowControl}; // 115200 is the default, as well.
} // namespace Motate

namespace Motate {
MOTATE_PIN_INTERRUPT(kSerial_CTS) {
    Motate::Serial.pinChangeInterrupt();
}
}

#endif //defined(__SAM3X8E__) || defined(__SAM3X8C__)

#endif /* end of include guard: MOTATESERIAL_H_ONCE */