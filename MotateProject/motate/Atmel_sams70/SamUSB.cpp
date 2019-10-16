/*
  utility/SamUSB.h - Library for the Motate system
  http://github.com/synthetos/motate/

  Copyright (c) 2015 - 2016 Robert Giseburt

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

#include "SamUSB.h"

namespace Motate {

    USBDeviceHardware *USBDeviceHardware::hw = nullptr;

    uint8_t USBControlBuffer[512];

    const char16_t MOTATE_USBLanguageString[] = {0x0409}; // English
    const char16_t *getUSBLanguageString(int8_t &length) {
        //      length = sizeof(MOTATE_USBLanguageString);
        length = 2;
        return MOTATE_USBLanguageString;
    }

    uint16_t checkEndpointSizeHardwareLimits(const uint16_t inSize, const uint8_t endpointNumber, const USBEndpointType_t endpointType, const bool otherSpeed) {
        uint16_t tempSize = inSize;

        if (endpointNumber == 0) {
            if (tempSize > 64)
                tempSize = 64;
        } else if (tempSize > 1024) {
            tempSize = 1024;
        }

        return tempSize;
    }

    extern "C" void USBHS_Handler(void) HOT_FUNC;

    extern "C"
    void USBHS_Handler(void) {
        USBDeviceHardware *hw = USBDeviceHardware::hw;

        // Check for and handle connect and disconnect
        if (hw->checkAndHandleVbusChange()) { return; }

        // Check for and handle SOF and MSOF
        if (hw->checkAndHandleSOF()) { return; }

        // Check for and handle Control (ep == 0) interrupts
        if (hw->checkAndHandleControl()) { return; }

        // Check for and handle endpoint interrupts (including DMA)
        if (hw->checkAndHandleEndpoint()) { return; }

        // Check for and handle reset
        if (hw->checkAndHandleReset()) { return; }

        // Check for and handle wakeup/suspend
        if (hw->checkAndHandleWakeupSuspend()) { return; }

        // if (!hw->_is_wake_up()) {
        //     __asm__("BKPT"); // USBHS_Handler was not handled
        // }

    }

}; // namespace Motate
