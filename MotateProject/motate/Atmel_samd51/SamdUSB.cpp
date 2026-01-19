/*
 utility/SamdUSB.cpp - USB CDC implementation for SAMD51
 http://github.com/synthetos/motate/

 Copyright (c) 2026 Riley Porter
 Copyright (c) 2026 Synthetos LLC

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

#include "SamdUSB.h"

#if defined(__SAMD51J19A__) || defined(__SAMD51J20A__) || defined(__SAMD51G19A__) || defined(__SAMD51P20A__)

namespace Motate {

    /*** Static member definitions ***/

    USBDeviceHardware *USBDeviceHardware::hw = nullptr;

    // Endpoint descriptors in SRAM - CRITICAL alignment requirement!
    // SAMD51 datasheet section 38.6.2.3 requires DESCADD to point to aligned descriptor table.
    // Each descriptor is 32 bytes (2 banks x 16 bytes). 32-byte alignment ensures no
    // descriptor spans an alignment boundary. Marked volatile since USB hardware modifies these.
    alignas(32) volatile UsbDeviceDescriptor USBDeviceHardware::_ep_descriptors[8];

    // Endpoint data buffers - 64 bytes per bank, 2 banks per endpoint (8 endpoints)
    // Word-aligned for USB hardware DMA access
    alignas(4) uint8_t USBDeviceHardware::_ep_buffers[8][2][64];

    // Control buffer for USB control transfers (used by MotateUSB.h)
    // SAMD51 is full-speed only so 64 bytes is sufficient, but keep 512 for compatibility
    uint8_t USBControlBuffer[512];

    /*** Language string (required by USB spec) ***/

    const char16_t *getUSBLanguageString(int8_t &length) {
        static const char16_t lang_string[] = { 0x0409 };  // English (US)
        length = sizeof(lang_string);
        return lang_string;
    }

    /*** Endpoint size hardware limits ***/
    // SAMD51 is full-speed only: max 64 bytes for bulk/interrupt, 1023 for isochronous
    uint16_t checkEndpointSizeHardwareLimits(const uint16_t inSize, const uint8_t endpointNumber,
                                              const USBEndpointType_t endpointType, const bool otherSpeed) {
        uint16_t tempSize = inSize;

        if (endpointNumber == 0) {
            // Control endpoint: max 64 bytes for full-speed
            if (tempSize > 64) {
                tempSize = 64;
            }
        } else if (tempSize > 1023) {
            // SAMD51 max packet size is 1023 bytes (for isochronous)
            // For bulk/interrupt full-speed, practical limit is 64 bytes
            tempSize = 1023;
        }

        return tempSize;
    }

}  // namespace Motate

/*** USB Interrupt Handlers ***/

// SAMD51 USB has 4 interrupt vectors:
// - USB_0: Device/Reset events (EORST, WAKEUP, SUSPEND, etc.)
// - USB_1: Start of Frame (SOF)
// - USB_2: TRCPT0 - Transfer complete bank 0 (OUT/SETUP)
// - USB_3: TRCPT1 - Transfer complete bank 1 (IN)

extern "C" {

// USB_0_Handler - Device events: Reset, Suspend, Wakeup
// CRITICAL: This handler can get into an infinite loop if SUSPEND/WAKEUP
// events toggle continuously during failed enumeration. To prevent starving
// the main loop, we clear ALL device-level flags unconditionally at the end.
void USB_0_Handler(void) {
    if (Motate::USBDeviceHardware::hw) {
        auto *hw = Motate::USBDeviceHardware::hw;

        // Handle reset first (highest priority)
        if (hw->checkAndHandleReset()) {
            // CRITICAL FIX: Poll for SETUP immediately after reset!
            // The host sends SETUP (GET_DEVICE_DESCRIPTOR) within microseconds
            // of reset completing. SOF hasn't started yet, so SOF-based polling
            // won't help. We MUST check for SETUP here before returning.
            hw->checkAndHandleControl();
            return;
        }

        // Handle suspend/wakeup
        hw->checkAndHandleWakeupSuspend();

        // Handle VBUS changes
        hw->checkAndHandleVbusChange();
    }

    // CRITICAL: Clear ALL device-level interrupt flags to prevent interrupt storm
    // This ensures we don't get stuck in a SUSPEND/WAKEUP loop that starves main()
    // The polling code in hardware_periodic() will handle these events properly
    // Clear ALL possible USB_0 interrupt sources (bits 0-9 of INTFLAG)
    USB->DEVICE.INTFLAG.reg = 0x03FF;  // Clear all device-level flags
}

// USB_1_Handler - Start of Frame (SOF) - fires every 1ms
// CRITICAL: We use SOF to poll for SETUP packets because RXSTP doesn't
// generate its own interrupt on SAMD51. This gives us 1ms timing which
// is sufficient for USB enumeration (need to respond within ~500us after
// the host IN token, which comes after SETUP processing).
void USB_1_Handler(void) {
    if (Motate::USBDeviceHardware::hw) {
        auto *hw = Motate::USBDeviceHardware::hw;

        // CRITICAL: Poll for SETUP packets on every SOF
        // This is our primary mechanism for detecting and handling SETUP packets
        hw->checkAndHandleControl();

        // Handle SOF acknowledgment
        hw->checkAndHandleSOF();
    }
}

// USB_2_Handler - TRCPT0 (Transfer complete bank 0 - OUT/SETUP)
void USB_2_Handler(void) {
    if (Motate::USBDeviceHardware::hw) {
        auto *hw = Motate::USBDeviceHardware::hw;

        // Handle control endpoint first (SETUP packets)
        hw->checkAndHandleControl();

        // Handle bulk/interrupt OUT endpoints
        hw->checkAndHandleEndpoint();
    }
}

// USB_3_Handler - TRCPT1 (Transfer complete bank 1 - IN)
void USB_3_Handler(void) {
    if (Motate::USBDeviceHardware::hw) {
        auto *hw = Motate::USBDeviceHardware::hw;

        // Handle control endpoint IN
        hw->checkAndHandleControl();

        // Handle bulk/interrupt IN endpoints
        hw->checkAndHandleEndpoint();
    }
}

}  // extern "C"

#endif  // __SAMD51*__
