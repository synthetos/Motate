/*
 utility/SamdUSB.h - USB CDC implementation for SAMD51
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

// This goes outside the guard! We need to ensure this happens first.
#include "MotateUSB.h"

#ifndef SAMDUSB_ONCE
#define SAMDUSB_ONCE

#include <functional>

#include "sam.h"  // Must be before other Motate includes to define PORT

#include "SamdCommon.h"
#include "MotateUSBHelpers.h"
#include "MotateUtilities.h"
#include "MotateUniqueID.h"
#include <cstring>
#include <algorithm>

#if !defined(__SAMD51J19A__) && !defined(__SAMD51J20A__) && !defined(__SAMD51G19A__) && !defined(__SAMD51P20A__)
# error The current SAMD USB Device Driver supports only SAMD51 family.
#endif

// USB debug helper (disabled by default)
template<int32_t len>
void usb_debug(const char (&)[len]) { /* disabled */ }

// Global debug counters for tracing USB transfer() calls
// These are inline to avoid multiple definition errors in C++17
inline volatile uint32_t g_usb_transfer_calls = 0;
inline volatile uint32_t g_usb_transfer_failed = 0;
inline volatile uint32_t g_usb_transfer_out_started = 0;
inline volatile uint32_t g_usb_config_at_transfer = 0;

namespace Motate {

    /*** ENDPOINT CONFIGURATION ***/

    // SAMD51 full-speed USB: 8 endpoints, 64-byte max packet size for bulk/interrupt
    // Endpoint size encoding: 0=8, 1=16, 2=32, 3=64, 4=128, 5=256, 6=512, 7=1023
    enum USBEndpointBufferSettingsFlags_t {
        // null endpoint is all zeros
        kEndpointBufferNull            = 0,

        // endpoint direction - stored in bit 11, OUTSIDE the type mask (0x0700)
        // CRITICAL: Must not overlap with type bits 8-10, otherwise direction
        // detection fails for Bulk/Control types which have bit 8 set
        kEndpointBufferOutputFromHost  = 0 << 11,  // OUT endpoint (host to device)
        kEndpointBufferInputToHost     = 1 << 11,  // IN endpoint (device to host)

        // This mask is not part of the public interface:
        kEndpointBufferDirectionMask   = 1 << 11,

        // buffer sizes (these map to PCKSIZE.SIZE field encoding)
        kEnpointBufferSizeUpTo8        = 0 << 0,  // SIZE = 0
        kEnpointBufferSizeUpTo16       = 1 << 0,  // SIZE = 1
        kEnpointBufferSizeUpTo32       = 2 << 0,  // SIZE = 2
        kEnpointBufferSizeUpTo64       = 3 << 0,  // SIZE = 3
        kEnpointBufferSizeUpTo128      = 4 << 0,  // SIZE = 4
        kEnpointBufferSizeUpTo256      = 5 << 0,  // SIZE = 5
        kEnpointBufferSizeUpTo512      = 6 << 0,  // SIZE = 6
        kEnpointBufferSizeUpTo1024     = 7 << 0,  // SIZE = 7 (1023 bytes actually)

        // This mask is not part of the public interface:
        kEnpointBufferSizeMask         = 0x07,

        // buffer "blocks" -- SAMD51 has 2 banks per endpoint
        kEndpointBufferBlocks1         = 0 << 4,
        kEndpointBufferBlocksUpTo2     = 1 << 4,
        kEndpointBufferBlocksUpTo3     = 1 << 4,  // Alias - SAMD51 only has 2 banks

        // This mask is not part of the public interface:
        kEndpointBufferBlocksMask      = 1 << 4,

        // endpoint types - EPCFG.EPTYPE values for SAMD51
        // OUT types (bits 0-2): 0=disabled, 1=control, 2=iso, 3=bulk, 4=interrupt, 5=dual-bank
        // IN types (bits 4-6): same encoding
        kEndpointBufferTypeControl     = 0x0100,  // Control endpoint
        kEndpointBufferTypeIsochronous = 0x0200,  // Isochronous endpoint
        kEndpointBufferTypeBulk        = 0x0300,  // Bulk endpoint
        kEndpointBufferTypeInterrupt   = 0x0400,  // Interrupt endpoint

        // This mask is not part of the public interface:
        kEndpointBufferTypeMask        = 0x0700
    };

    // Convert from number to EndpointBufferSettings_t
    static const EndpointBufferSettings_t getBufferSizeFlags(const uint16_t size) {
        if (size > 512) {
            return kEnpointBufferSizeUpTo1024;
        } else if (size > 256) {
            return kEnpointBufferSizeUpTo512;
        } else if (size > 128) {
            return kEnpointBufferSizeUpTo256;
        } else if (size > 64) {
            return kEnpointBufferSizeUpTo128;
        } else if (size > 32) {
            return kEnpointBufferSizeUpTo64;
        } else if (size > 16) {
            return kEnpointBufferSizeUpTo32;
        } else if (size > 8) {
            return kEnpointBufferSizeUpTo16;
        } else {
            return kEnpointBufferSizeUpTo8;
        }
        return kEndpointBufferNull;
    };

    /*** STRINGS ***/

    const char16_t *getUSBVendorString(int8_t &length) ATTR_WEAK;
    const char16_t *getUSBProductString(int8_t &length) ATTR_WEAK;
    const char16_t *getUSBSerialNumberString(int8_t &length) ATTR_WEAK;

    // We break the rules here, sortof, by providing a macro shortcut that gets used in userland.
#define MOTATE_SET_USB_VENDOR_STRING(...)\
    const char16_t MOTATE_USBVendorString[] = __VA_ARGS__;\
    const char16_t *Motate::getUSBVendorString(int8_t &length) {\
        length = sizeof(MOTATE_USBVendorString);\
        return MOTATE_USBVendorString;\
    }

#define MOTATE_SET_USB_PRODUCT_STRING(...)\
    const char16_t MOTATE_USBProductString[] = __VA_ARGS__;\
    const char16_t *Motate::getUSBProductString(int8_t &length) {\
        length = sizeof(MOTATE_USBProductString);\
        return MOTATE_USBProductString;\
    }

#define MOTATE_SET_USB_SERIAL_NUMBER_STRING(...)\
    const char16_t MOTATE_USBSerialNumberString[] = __VA_ARGS__;\
    const char16_t *Motate::getUSBSerialNumberString(int8_t &length) {\
        length = sizeof(MOTATE_USBSerialNumberString);\
        return MOTATE_USBSerialNumberString;\
    }

#define MOTATE_SET_USB_SERIAL_NUMBER_STRING_FROM_CHIPID() \
    char16_t MOTATE_USBSerialNumberString[Motate::UUID_t::length]; \
    const char16_t *Motate::getUSBSerialNumberString(int8_t &length) { \
        const char *uuid = Motate::UUID; \
        uint8_t i = Motate::UUID_t::length; \
        length = i * sizeof(uint16_t); \
        static bool inited = false; \
        if (inited == true) { return MOTATE_USBSerialNumberString; } \
        for (uint8_t j = 0; j < i; j++) { \
            MOTATE_USBSerialNumberString[j] = *uuid++; \
        } \
        return MOTATE_USBSerialNumberString; \
    }

    // This needs to be provided in the hardware file
    const char16_t *getUSBLanguageString(int8_t &length);

    /*** USB_DMA_Descriptor - Software-driven transfer descriptor for SAMD51 ***/
    // SAMD51 doesn't have dedicated USB DMA like SAMS70, but we use this struct
    // to maintain API compatibility

    struct alignas(16) USB_DMA_Descriptor {
        enum _commands {
            stop_now        = 0,
            run_and_stop    = 1,
            load_next_desc  = 2,
            run_and_link    = 3
        };

        USB_DMA_Descriptor *next_descriptor;
        char *buffer_address;
        struct {
            _commands command : 2;
            bool end_transfer_enable : 1;
            bool end_buffer_enable : 1;
            bool end_transfer_interrupt_enable : 1;
            bool end_buffer_interrupt_enable : 1;
            bool descriptor_loaded_interrupt_enable : 1;
            bool bust_lock_enable : 1;
            uint8_t _unused_1 : 8;
            uint16_t buffer_length : 16;
        };

        void setBuffer(char* data, uint16_t len) {
            buffer_address = data;
            buffer_length = len;
        }
    };

    // Transfer state tracking for software-driven transfers
    struct TransferState {
        char *buffer;
        uint32_t total_length;
        uint32_t transferred;
        bool active;
    };

    /*** USBDeviceHardware ***/

    struct USBDeviceHardware
    {
        uint32_t _inited = 0;
        uint32_t config_number = 0;
        bool _address_available = false;
        bool _attached = false;  // Track physical USB attachment state

        enum USBSetupState_t {
            SETUP                  = 0,
            DATA_OUT               = 1,
            DATA_IN                = 2,
            HANDSHAKE_WAIT_IN_ZLP  = 3,
            HANDSHAKE_WAIT_OUT_ZLP = 4,
            STALL_REQ              = 5,
        };

        USBSetupState_t setup_state = SETUP;
        USBDevice_t * const proxy;
        static USBDeviceHardware *hw;

        static const uint8_t master_control_endpoint = 0;

        // SAMD51 has 8 endpoints (0-7)
        static constexpr uint32_t _get_endpoint_max_nbr() { return 7; };
        static constexpr uint32_t MAX_PEP_NB() { return 8; };

        // Endpoint descriptors in SRAM - CRITICAL alignment requirement!
        // SAMD51 datasheet section 38.6.2.3: DESCADD must point to endpoint descriptor table
        // Each descriptor is 32 bytes (2 banks x 16 bytes). For 8 endpoints = 256 bytes.
        // Per TinyUSB/Microchip examples, 32-byte alignment is sufficient, but we use 256
        // to be safe and ensure the entire table doesn't cross alignment boundaries.
        // Marked volatile since USB hardware modifies these asynchronously.
        static volatile UsbDeviceDescriptor _ep_descriptors[8] __attribute__((aligned(32)));

        // Endpoint data buffers - 64 bytes per bank, 2 banks per endpoint
        // USB transfers require buffer addresses to be word-aligned for DMA
        static uint8_t _ep_buffers[8][2][64] __attribute__((aligned(4)));

        // Transfer state for software-driven transfers (no HW DMA on SAMD51)
        uint32_t _transfer_active_by_endpoint;
        TransferState _transfers[8];

        // callback for after a control read is done
        std::function<void(void)> _control_read_completed_callback;

        // hold the buffer pointers to setup responses
        struct SetupBuffer_t {
            char *buf_addr_0;
            uint16_t length_0;
            char *buf_addr_1;
            uint16_t length_1;
        } _setup_buffer;

        // Init
        USBDeviceHardware(USBDevice_t * const _proxy) : proxy{_proxy}
        {
            hw = this;
            _init();
        };

        // ensure we can't copy or move a USBDeviceHardware
        USBDeviceHardware(const USBDeviceHardware&) = delete;
        USBDeviceHardware(USBDeviceHardware&&) = delete;
        USBDeviceHardware() = delete;

        /*** USB Pad Calibration ***/
        void _loadUSBCalibration() {
            // SAMD51 USB pad calibration values are stored in NVM Software Calibration Area
            // Per datasheet Table 9-3 "NVM Software Calibration Area Mapping":
            //   Address: 0x00800080
            //   USB TRANSN: bits 36:32 (5 bits) - default 0x09
            //   USB TRANSP: bits 41:37 (5 bits) - default 0x19
            //   USB TRIM:   bits 44:42 (3 bits) - default 0x06
            // All USB calibration bits are in word 1 (bits 32-63)

            const uint32_t *cal_area = reinterpret_cast<const uint32_t *>(0x00800080);
            uint32_t cal_word_1 = cal_area[1];

            // Extract calibration values from word 1
            uint32_t transn = (cal_word_1 >> 0) & 0x1F;   // bits 36:32 = word1[4:0]
            uint32_t transp = (cal_word_1 >> 5) & 0x1F;   // bits 41:37 = word1[9:5]
            uint32_t trim   = (cal_word_1 >> 10) & 0x07;  // bits 44:42 = word1[12:10]

            // If calibration values are all ones, use datasheet defaults
            if (transn == 0x1F) transn = 0x09;
            if (transp == 0x1F) transp = 0x19;
            if (trim == 0x07) trim = 0x06;

            // Write calibration to PADCAL register
            USB->DEVICE.PADCAL.reg = USB_PADCAL_TRANSN(transn) |
                                     USB_PADCAL_TRANSP(transp) |
                                     USB_PADCAL_TRIM(trim);
        }

        /*** Initialization ***/
        void _init() {
            SamdCommon::InterruptDisabler disabler;

            // 0. Zero descriptor memory to ensure no garbage in AUTO_ZLP or other fields
            memset((void*)_ep_descriptors, 0, sizeof(_ep_descriptors));

            // 1. Enable USB AHB clock via MCLK APB-B
            MCLK->APBBMASK.reg |= MCLK_APBBMASK_USB;
            __DSB();  // Ensure write completes before peripheral access

            // 2. Enable 48MHz GCLK for USB peripheral BEFORE any USB register access
            // USB uses GCLK_USB (peripheral channel 10), GCLK1 is configured in SystemInit
            GCLK->PCHCTRL[USB_GCLK_ID].reg = GCLK_PCHCTRL_GEN_GCLK1 | GCLK_PCHCTRL_CHEN;
            while (GCLK->PCHCTRL[USB_GCLK_ID].bit.CHEN == 0);

            // 3. Configure USB pins (PA24 = D-, PA25 = D+)
            // Set pin mux for USB function (function H = 7)
            PORT->Group[0].PINCFG[24].reg = PORT_PINCFG_PMUXEN;
            PORT->Group[0].PINCFG[25].reg = PORT_PINCFG_PMUXEN;
            PORT->Group[0].PMUX[12].reg = PORT_PMUX_PMUXE(7) | PORT_PMUX_PMUXO(7);

            // 4. NOW it's safe to access USB registers - load pad calibration
            _loadUSBCalibration();

            // 5. Reset USB peripheral (TinyUSB double-wait pattern)
            USB->DEVICE.CTRLA.bit.SWRST = 1;
            while (USB->DEVICE.SYNCBUSY.bit.SWRST == 0) {}  // Wait for reset to START
            while (USB->DEVICE.SYNCBUSY.bit.SWRST) {}       // Wait for reset to COMPLETE

            // 6. Set Quality of Service - required for proper interrupt generation
            // Per TinyUSB and Microchip examples, both CQOS and DQOS must be set to 3
            USB->DEVICE.QOSCTRL.bit.CQOS = 3;  // Critical QoS (for control transfers)
            USB->DEVICE.QOSCTRL.bit.DQOS = 3;  // Data QoS (for data transfers)

            // 7. Configure USB in device mode, full-speed
            USB->DEVICE.CTRLA.bit.MODE = 0;  // 0 = Device mode
            USB->DEVICE.CTRLB.bit.SPDCONF = USB_DEVICE_CTRLB_SPDCONF_FS_Val;

            // 8. Set descriptor base address
            USB->DEVICE.DESCADD.reg = (uint32_t)&_ep_descriptors[0];

            // 9. Configure and enable interrupts (SAMD51 has 4 USB interrupt handlers)
            NVIC_SetPriority(USB_0_IRQn, 1);  // Device/reset events
            NVIC_SetPriority(USB_1_IRQn, 1);  // SOF
            NVIC_SetPriority(USB_2_IRQn, 1);  // TRCPT0
            NVIC_SetPriority(USB_3_IRQn, 1);  // TRCPT1

            NVIC_EnableIRQ(USB_0_IRQn);
            NVIC_EnableIRQ(USB_1_IRQn);
            NVIC_EnableIRQ(USB_2_IRQn);
            NVIC_EnableIRQ(USB_3_IRQn);

            // 10. Initialize state
            _inited = 1UL;
            config_number = 0UL;

            _setup_buffer = {nullptr, 0, nullptr, 0};
            setup_state = SETUP;
            _transfer_active_by_endpoint = 0;

            // Clear all transfer states
            for (uint32_t i = 0; i < MAX_PEP_NB(); i++) {
                _transfers[i] = {nullptr, 0, 0, false};
            }

            _detach();
        };

        /*** Attach/Detach ***/
        void _attach() {
            SamdCommon::InterruptDisabler disabler;

            // Initialize EP0 BEFORE attaching to bus
            // This ensures we're ready to handle SETUP packets immediately
            // The host may send SETUP as soon as it detects device attachment
            _init_endpoint(0, proxy->getEndpointConfig(0, false));

            // Clear any stale interrupt flags on EP0
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = 0xFF;

            // Enable EP0 interrupts for SETUP handling
            _enable_setup_received_interrupt(0);
            _enable_out_received_interrupt(0);
            _enable_endpoint_interrupt(0);

            // Make bank 0 ready for OUT data
            _set_bank0_ready(0);
            // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;

            // Reset setup state
            setup_state = SETUP;
            _setup_buffer = {nullptr, 0, nullptr, 0};

            // Enable USB FIRST with RUNSTDBY (keeps USB running during power transitions)
            // Must enable before clearing DETACH per SAMD51 errata
            USB->DEVICE.CTRLA.reg = USB_CTRLA_MODE_DEVICE | USB_CTRLA_ENABLE | USB_CTRLA_RUNSTDBY;
            while (USB->DEVICE.SYNCBUSY.bit.ENABLE) {}  // Wait for enable to complete

            // THEN connect to bus (after USB is fully enabled)
            USB->DEVICE.CTRLB.bit.DETACH = 0;

            // Enable device-level interrupts
            _enable_reset_interrupt();
            _enable_suspend_interrupt();
            _enable_wake_up_interrupt();

            // CRITICAL: Enable SOF interrupt for polling SETUP packets
            // RXSTP doesn't trigger its own interrupt on SAMD51, so we must poll
            // for SETUP packets on every SOF (1ms intervals) to meet USB timing
            _enable_sof_interrupt();

            _attached = true;  // Mark as attached to bus
        };

        bool attach() {
            if (_inited) {
                _attach();
                return true;
            }
            return false;
        };

        void _detach() {
            _attached = false;  // Mark as detached from bus
            // Set detach bit to disconnect from bus
            USB->DEVICE.CTRLB.bit.DETACH = 1;
        };

        bool detach() {
            if (_inited) {
                _detach();
                return true;
            }
            return false;
        };

        /*** Interrupt Enable/Disable helpers ***/

        // Reset events
        void _enable_reset_interrupt()     { USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_EORST; };
        void _disable_reset_interrupt()    { USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTENCLR_EORST; };
        void _ack_reset()                  { USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_EORST; };
        bool _is_reset()                   { return (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_EORST); };

        // Suspend events
        void _enable_suspend_interrupt()   { USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_SUSPEND; };
        void _disable_suspend_interrupt()  { USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTENCLR_SUSPEND; };
        void _ack_suspend()                { USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_SUSPEND; };
        bool _is_suspend()                 { return (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_SUSPEND); };

        // Wake-up events
        void _enable_wake_up_interrupt()   { USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_WAKEUP; };
        void _disable_wake_up_interrupt()  { USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTENCLR_WAKEUP; };
        void _ack_wake_up()                { USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_WAKEUP; };
        bool _is_wake_up()                 { return (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_WAKEUP); };

        // SOF events
        void _enable_sof_interrupt()       { USB->DEVICE.INTENSET.reg = USB_DEVICE_INTENSET_SOF; };
        void _disable_sof_interrupt()      { USB->DEVICE.INTENCLR.reg = USB_DEVICE_INTENCLR_SOF; };
        void _ack_sof()                    { USB->DEVICE.INTFLAG.reg = USB_DEVICE_INTFLAG_SOF; };
        bool _is_sof()                     { return (USB->DEVICE.INTFLAG.reg & USB_DEVICE_INTFLAG_SOF); };

        /*** Address handling ***/
        void _configure_address(uint32_t addr) {
            USB->DEVICE.DADD.reg = USB_DEVICE_DADD_ADDEN | USB_DEVICE_DADD_DADD(addr);
        };
        void _enable_address() {
            USB->DEVICE.DADD.bit.ADDEN = 1;
        };
        void _disable_address() {
            USB->DEVICE.DADD.bit.ADDEN = 0;
        };
        void setAddressAvailable() { _address_available = true; };

        /*** Endpoint Configuration ***/

        // Get size encoding from buffer size flags
        uint8_t _get_size_encoding(EndpointBufferSettings_t config) {
            return config & kEnpointBufferSizeMask;
        }

        // Get endpoint type encoding
        uint8_t _get_type_encoding(EndpointBufferSettings_t config) {
            switch (config & kEndpointBufferTypeMask) {
                case kEndpointBufferTypeControl:     return 1;
                case kEndpointBufferTypeIsochronous: return 2;
                case kEndpointBufferTypeBulk:        return 3;
                case kEndpointBufferTypeInterrupt:   return 4;
                default: return 0; // disabled
            }
        }

        // Check if endpoint is IN (device to host)
        bool _is_endpoint_a_tx_in(uint32_t ep) {
            // Check EPCFG to determine direction
            // IN endpoints have EPTYPE1 configured
            return (USB->DEVICE.DeviceEndpoint[ep].EPCFG.bit.EPTYPE1 != 0);
        };

        static const EndpointBufferSettings_t _enforce_endpoint_limits(const uint8_t endpoint, EndpointBufferSettings_t config) {
            if (endpoint > 7)
                return kEndpointBufferNull;

            // Endpoint 0 is control only, max 64 bytes
            if (endpoint == 0) {
                if ((config & kEnpointBufferSizeMask) > kEnpointBufferSizeUpTo64)
                    config = (config & ~kEnpointBufferSizeMask) | kEnpointBufferSizeUpTo64;
                config = (config & ~kEndpointBufferBlocksMask) | kEndpointBufferBlocks1;
            }
            // Other endpoints max 64 bytes for full-speed bulk/interrupt
            else {
                if ((config & kEnpointBufferSizeMask) > kEnpointBufferSizeUpTo64)
                    config = (config & ~kEnpointBufferSizeMask) | kEnpointBufferSizeUpTo64;
            }

            return config;
        };

        // Debug: Track endpoint configuration
        volatile uint8_t _dbg_last_ep_configured = 0xFF;
        volatile uint32_t _dbg_last_ep_config = 0;
        volatile uint8_t _dbg_last_ep_is_in = 0;
        volatile uint8_t _dbg_last_ep_type = 0;
        volatile uint8_t _dbg_ep_init_count = 0;

        void _init_endpoint(uint32_t endpoint, const uint32_t configuration) {
            endpoint = endpoint & 0x7;  // SAMD51 has 8 endpoints (0-7)

            volatile uint32_t config_fixed = _enforce_endpoint_limits(endpoint, configuration);
            if (config_fixed == kEndpointBufferNull) {
                return;
            }

            uint8_t type = _get_type_encoding(config_fixed);
            uint8_t size = _get_size_encoding(config_fixed);
            bool is_in = (config_fixed & kEndpointBufferDirectionMask);

            // Debug: track what we're configuring
            _dbg_ep_init_count++;
            _dbg_last_ep_configured = endpoint;
            _dbg_last_ep_config = config_fixed;
            _dbg_last_ep_is_in = is_in ? 1 : 0;
            _dbg_last_ep_type = type;

            // CRITICAL: Configure endpoint descriptors BEFORE enabling the endpoint type
            // The USB hardware reads descriptors as soon as endpoint is enabled
            // Descriptors must be valid in SRAM before EPCFG is written

            // Configure endpoint descriptor (bank 0 for OUT, bank 1 for IN)
            if (endpoint == 0 || !is_in) {
                // OUT bank (bank 0) - clear entire descriptor first
                _ep_descriptors[endpoint].DeviceDescBank[0].ADDR.reg = (uint32_t)&_ep_buffers[endpoint][0];
                _ep_descriptors[endpoint].DeviceDescBank[0].PCKSIZE.reg =
                    USB_DEVICE_PCKSIZE_SIZE(size);  // Clear all bits, set only SIZE
                _ep_descriptors[endpoint].DeviceDescBank[0].STATUS_BK.reg = 0;
            }

            if (endpoint == 0 || is_in) {
                // IN bank (bank 1) - clear entire descriptor first
                _ep_descriptors[endpoint].DeviceDescBank[1].ADDR.reg = (uint32_t)&_ep_buffers[endpoint][1];
                _ep_descriptors[endpoint].DeviceDescBank[1].PCKSIZE.reg =
                    USB_DEVICE_PCKSIZE_SIZE(size);  // Clear all bits, set only SIZE
                _ep_descriptors[endpoint].DeviceDescBank[1].STATUS_BK.reg = 0;
            }

            // Memory barrier to ensure descriptor writes are visible before enabling endpoint
            __DSB();

            // NOW configure endpoint type (enables the endpoint)
            if (endpoint == 0) {
                // Control endpoint uses both banks for IN and OUT
                USB->DEVICE.DeviceEndpoint[endpoint].EPCFG.reg =
                    USB_DEVICE_EPCFG_EPTYPE0(type) | USB_DEVICE_EPCFG_EPTYPE1(type);
            } else if (is_in) {
                USB->DEVICE.DeviceEndpoint[endpoint].EPCFG.reg =
                    USB_DEVICE_EPCFG_EPTYPE1(type);  // IN endpoint
            } else {
                USB->DEVICE.DeviceEndpoint[endpoint].EPCFG.reg =
                    USB_DEVICE_EPCFG_EPTYPE0(type);  // OUT endpoint

                // CRITICAL: For OUT endpoints, prepare to receive data immediately
                // Clear BK0RDY to indicate buffer is ready to receive
                // Enable TRCPT0 interrupt to be notified when data arrives
                _set_bank0_ready(endpoint);
                // Clear TRFAIL0 AFTER clearing BK0RDY (per TinyUSB implementation)
                USB->DEVICE.DeviceEndpoint[endpoint].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
                _enable_out_received_interrupt(endpoint);
                _enable_endpoint_interrupt(endpoint);
            }
        };

        /*** Endpoint Status helpers ***/

        // Get byte count from endpoint
        uint32_t get_byte_count(uint32_t ep) {
            return _ep_descriptors[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
        };

        uint32_t _get_endpoint_size(uint32_t ep) {
            uint8_t size_code = _ep_descriptors[ep].DeviceDescBank[0].PCKSIZE.bit.SIZE;
            return (8 << size_code);  // 0=8, 1=16, 2=32, 3=64, etc.
        }

        // SETUP received check - use direct mask instead of bitfield to avoid potential compiler issues
        volatile uint8_t _dbg_is_setup_flag = 0;
        bool _is_setup_received(uint32_t ep) {
            uint8_t flag = USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg;
            _dbg_is_setup_flag = flag;
            return (flag & USB_DEVICE_EPINTFLAG_RXSTP) != 0;
        };
        void _ack_setup_received(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_RXSTP;
        };
        void _enable_setup_received_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENSET.reg = USB_DEVICE_EPINTENSET_RXSTP;
        };
        void _disable_setup_received_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENCLR.reg = USB_DEVICE_EPINTENCLR_RXSTP;
        };

        void ackSetupRX() {
            _ack_setup_received(0);
        }

        // OUT received check
        bool _is_out_received(uint32_t ep) {
            return (USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.bit.TRCPT0 != 0);
        };
        void _ack_out_received(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT0;
        };
        void _enable_out_received_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT0;
        };
        void _disable_out_received_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENCLR.reg = USB_DEVICE_EPINTENCLR_TRCPT0;
        };
        void ackRXOut(uint32_t ep) { _ack_out_received(ep); };

        // IN send check
        bool _is_in_send(uint32_t ep) {
            return (USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.bit.TRCPT1 != 0);
        };
        void _ack_in_send(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRCPT1;
        };
        void _enable_in_send_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENSET.reg = USB_DEVICE_EPINTENSET_TRCPT1;
        };
        void _disable_in_send_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENCLR.reg = USB_DEVICE_EPINTENCLR_TRCPT1;
        };

        void enableTXSentInterrupt(uint32_t ep) { _enable_in_send_interrupt(ep); };
        void disableTXSentInterrupt(uint32_t ep) { _disable_in_send_interrupt(ep); };

        // STALL handling
        void _enable_stall_handshake(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_STALLRQ0 |
                                                             USB_DEVICE_EPSTATUSSET_STALLRQ1;
        };
        void _disable_stall_handshake(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_STALLRQ0 |
                                                              USB_DEVICE_EPSTATUSCLR_STALLRQ1;
        };

        void _request_stall_handshake(uint32_t ep) {
            setup_state = STALL_REQ;
            _enable_stall_handshake(ep);
        }

        // Bank ready control
        void _set_bank0_ready(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK0RDY;
        }
        void _set_bank1_ready(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPSTATUSSET.reg = USB_DEVICE_EPSTATUSSET_BK1RDY;
        }
        void _clear_bank1_ready(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPSTATUSCLR.reg = USB_DEVICE_EPSTATUSCLR_BK1RDY;
        }

        bool _is_bank0_ready(uint32_t ep) {
            return (USB->DEVICE.DeviceEndpoint[ep].EPSTATUS.reg & USB_DEVICE_EPSTATUS_BK0RDY);
        }
        bool _is_bank1_ready(uint32_t ep) {
            return (USB->DEVICE.DeviceEndpoint[ep].EPSTATUS.reg & USB_DEVICE_EPSTATUS_BK1RDY);
        }

        // Endpoint interrupt control
        void _enable_endpoint_interrupt(uint32_t ep) {
            // Enable TRCPT0 (OUT) and TRCPT1 (IN) interrupts for endpoint
            USB->DEVICE.DeviceEndpoint[ep].EPINTENSET.reg =
                USB_DEVICE_EPINTENSET_TRCPT0 | USB_DEVICE_EPINTENSET_TRCPT1;
        };
        void _disable_endpoint_interrupt(uint32_t ep) {
            USB->DEVICE.DeviceEndpoint[ep].EPINTENCLR.reg =
                USB_DEVICE_EPINTENCLR_TRCPT0 | USB_DEVICE_EPINTENCLR_TRCPT1;
        };
        bool _is_endpoint_interrupt(uint32_t ep) {
            return (USB->DEVICE.EPINTSMRY.reg & (1 << ep));
        };

        /*** Reset handling ***/
        void reset() {
            _ack_reset();

            _configure_address(0);
            _address_available = false;
            config_number = 0;  // Reset configuration - device is no longer configured
            // Note: Don't set _vbus_changed here - bus reset is not a disconnect event
            // The host will re-enumerate and send SET_CONFIGURATION to restore config_number

            // Clear any active transfers
            for (uint32_t ep = 0; ep < MAX_PEP_NB(); ep++) {
                if (_transfer_active_by_endpoint & (1 << ep)) {
                    _transfer_active_by_endpoint &= ~(1 << ep);
                    _transfers[ep].active = false;
                    proxy->handleTransferDone(ep);
                }
            }

            _init_endpoint(0, proxy->getEndpointConfig(0, false));

            // DEBUG: Capture descriptor state after initialization (read via J-Link)
            _dbg_bank0_addr = _ep_descriptors[0].DeviceDescBank[0].ADDR.reg;
            _dbg_bank1_addr = _ep_descriptors[0].DeviceDescBank[1].ADDR.reg;
            _dbg_bank0_pcksize = _ep_descriptors[0].DeviceDescBank[0].PCKSIZE.reg;
            _dbg_bank1_pcksize = _ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg;

            // CRITICAL: Clear ALL endpoint 0 interrupt flags after reset
            // Any pending RXSTP/TRCPT flags from before the bus reset are stale
            // and must be cleared to avoid processing aborted transactions
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = 0xFF;  // Clear all flags

            _enable_setup_received_interrupt(0);
            _enable_out_received_interrupt(0);
            _enable_endpoint_interrupt(0);

            // Make bank 0 ready for OUT data (clear BK0RDY to indicate ready to receive)
            _set_bank0_ready(0);
            // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
        };

        void initSetup() {
            disableTXSentInterrupt(0);
            ackRXOut(0);
            setup_state = SETUP;
            _setup_buffer = {nullptr, 0, nullptr, 0};
        };

        /*** Control Transfer handling ***/

        void readSetupPacket(Setup_t &setup) {
            _readFromControl((char *)USBControlBuffer, 8);
            setup.set((char *)USBControlBuffer);
            ackSetupRX();
        }

        int16_t _readFromControl(char* buffer, int16_t length) {
            if (length < 0)
                return -1;

            int16_t to_read = length;
            uint8_t *src = _ep_buffers[0][0];  // Bank 0 for OUT
            while (to_read-- > 0) {
                *buffer++ = *src++;
            }
            return length;
        };

        void readFromControlThen(char *buffer, uint16_t length, std::function<void(void)> &&callback) {
            _control_read_completed_callback = std::move(callback);
            _setup_buffer = {buffer, length, nullptr, 0};
            setup_state = DATA_OUT;
        }

        void readFromControlThen(char *buffer, uint16_t length, const std::function<void(void)> &callback) {
            _control_read_completed_callback = callback;
            _setup_buffer = {buffer, length, nullptr, 0};
            setup_state = DATA_OUT;
        }

        void writeToControl(char* buffer_0, uint16_t length_0, char* buffer_1 = nullptr, uint16_t length_1 = 0) {
            _setup_buffer = {buffer_0, length_0, buffer_1, length_1};
        };

        void _send_zlp_on_in() {
            setup_state = HANDSHAKE_WAIT_IN_ZLP;

            // CRITICAL FIX: Clear TRFAIL1 FIRST, before any descriptor modifications
            // If TRFAIL1 is set (from host sending IN before we're ready), the USB
            // hardware may be in an error state. We must clear it before attempting
            // a new transfer. The host will have received NAK and will retry.
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;

            // Set Bank1 ADDR (defensive - should be set in init)
            _ep_descriptors[0].DeviceDescBank[1].ADDR.reg = (uint32_t)_ep_buffers[0][1];

            // Set PCKSIZE with MULTI_PACKET_SIZE=0
            // Per SAMD51 datasheet 38.6.2.10: MULTI_PACKET_SIZE must be 0 for single-packet
            _ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg =
                USB_DEVICE_PCKSIZE_SIZE(3) |              // 64 bytes max packet
                USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) | // CRITICAL: must be 0
                USB_DEVICE_PCKSIZE_BYTE_COUNT(0);         // ZLP = 0 bytes

            // Memory barrier AND readback to ensure descriptor write completes
            __DSB();
            (void)_ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg;  // Force completion

            // NOW set BK1RDY - signals IN data is ready for host
            _set_bank1_ready(0);
            _dbg_bk1rdy_set_count++;
            _enable_in_send_interrupt(0);
        };

        void _send_zlp_on_out() {
            setup_state = HANDSHAKE_WAIT_OUT_ZLP;
            _set_bank0_ready(0);
            // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
        };

        void _handleControlTX() {
            _disable_in_send_interrupt(0);

            // CRITICAL FIX: Clear TRFAIL1 at the VERY START of TX handling
            // Host may have sent IN token that NAK'd while we were processing SETUP
            // We must clear this before doing anything else to avoid stale error state
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;

            if (HANDSHAKE_WAIT_IN_ZLP == setup_state) {
                if (_address_available) {
                    _configure_address(proxy->getSetup().valueLow());
                    _enable_address();
                    _address_available = false;
                }
                initSetup();
                return;
            }

            // Limit to setup.maxLength
            if (proxy->getSetup().length() < (_setup_buffer.length_0 + _setup_buffer.length_1)) {
                if (_setup_buffer.length_0 < proxy->getSetup().length()) {
                    _setup_buffer.length_1 = proxy->getSetup().length() - _setup_buffer.length_0;
                } else {
                    _setup_buffer.length_0 = proxy->getSetup().length();
                    _setup_buffer.length_1 = 0;
                }
            }

            uint16_t to_send = _setup_buffer.length_0 + _setup_buffer.length_1;
            if (to_send > _get_endpoint_size(0)) {
                to_send = _get_endpoint_size(0);
            } else if (to_send == 0) {
                _send_zlp_on_out();
                return;
            }

            if (_is_out_received(0)) {
                setup_state = HANDSHAKE_WAIT_OUT_ZLP;
                return;
            }

            // TIMING CRITICAL: Must set BK1RDY before host IN token arrives (~500us after SETUP)
            // Use zero-copy DMA when possible to minimize latency

            uint16_t sent;
            if (_setup_buffer.length_1 == 0 || to_send <= _setup_buffer.length_0) {
                // FAST PATH: All data in one contiguous buffer - zero-copy DMA
                // Point USB peripheral directly at source data, no copy needed
                sent = std::min(to_send, _setup_buffer.length_0);

                _ep_descriptors[0].DeviceDescBank[1].ADDR.reg = (uint32_t)_setup_buffer.buf_addr_0;
                _ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg =
                    USB_DEVICE_PCKSIZE_SIZE(3) |              // 64 bytes max packet
                    USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) | // CRITICAL: must be 0
                    USB_DEVICE_PCKSIZE_BYTE_COUNT(sent);

                // Update buffer pointers for multi-packet transfers
                _setup_buffer.buf_addr_0 += sent;
                _setup_buffer.length_0 -= sent;
            } else {
                // SLOW PATH: Data spans two buffers, must copy to contiguous Bank1 buffer
                uint8_t *dst = _ep_buffers[0][1];
                sent = 0;
                while (sent < to_send) {
                    if (_setup_buffer.length_0 == 0) {
                        _setup_buffer.length_0 = _setup_buffer.length_1;
                        _setup_buffer.length_1 = 0;
                        _setup_buffer.buf_addr_0 = _setup_buffer.buf_addr_1;
                        _setup_buffer.buf_addr_1 = nullptr;
                    }
                    *dst++ = *_setup_buffer.buf_addr_0++;
                    _setup_buffer.length_0--;
                    sent++;
                }

                _ep_descriptors[0].DeviceDescBank[1].ADDR.reg = (uint32_t)_ep_buffers[0][1];
                _ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg =
                    USB_DEVICE_PCKSIZE_SIZE(3) |
                    USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) |
                    USB_DEVICE_PCKSIZE_BYTE_COUNT(sent);
            }

            // Memory barrier AND readback to ensure descriptor write completes
            __DSB();
            (void)_ep_descriptors[0].DeviceDescBank[1].PCKSIZE.reg;  // Force completion

            // Set BK1RDY to signal data is ready for host
            _set_bank1_ready(0);
            _dbg_bk1rdy_set_count++;
            _enable_in_send_interrupt(0);
        };

        void _handleControlRX() {
            if (DATA_OUT != setup_state) {
                if ((HANDSHAKE_WAIT_OUT_ZLP == setup_state) || (DATA_IN == setup_state)) {
                    // End of SETUP request
                } else {
                    _request_stall_handshake(0);
                }
                initSetup();
                return;
            }

            if (_setup_buffer.length_0) {
                uint16_t amt_sent = (uint16_t)std::min((uint16_t)proxy->getSetup().length(), _setup_buffer.length_0);
                _readFromControl(_setup_buffer.buf_addr_0, amt_sent);
                _setup_buffer.buf_addr_0 += _setup_buffer.length_0;
                _setup_buffer.length_0 -= amt_sent;
                if (_control_read_completed_callback) {
                    _control_read_completed_callback();
                }

                if ((amt_sent != _get_endpoint_size(0)) || (_setup_buffer.length_0)) {
                    _ack_out_received(0);
                    _send_zlp_on_in();
                    return;
                }
            }

            _ack_out_received(0);
            _set_bank0_ready(0);
            // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
            USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
        };

        /*** Control endpoint handling ***/

        // Debug counters - read via J-Link to trace execution
        volatile uint32_t _dbg_poll_count = 0;
        volatile uint32_t _dbg_ctrl_called = 0;
        volatile uint32_t _dbg_ctrl_has_flag = 0;
        volatile uint32_t _dbg_setup_received = 0;
        volatile uint32_t _dbg_setup_handled = 0;
        volatile uint8_t _dbg_last_epintflag = 0;
        volatile uint8_t _dbg_first_epintflag = 0;  // First non-zero flag seen
        volatile uint8_t _dbg_last_epstatus = 0;    // EP0 status register (BK0RDY/BK1RDY state)
        volatile uint32_t _dbg_trfail1_count = 0;   // Number of TRFAIL1 events seen
        volatile uint8_t _dbg_setup_state_at_trfail = 0;  // setup_state when TRFAIL1 seen
        volatile uint32_t _dbg_bk1rdy_set_count = 0;  // How many times we set BK1RDY
        volatile uint32_t _dbg_trcpt1_count = 0;      // How many times TRCPT1 fired (IN complete)

        // Debug: Descriptor state after reset (verify initialization is correct)
        volatile uint32_t _dbg_bank0_addr = 0;      // Bank0 ADDR right after _init_endpoint()
        volatile uint32_t _dbg_bank1_addr = 0;      // Bank1 ADDR right after _init_endpoint()
        volatile uint32_t _dbg_bank0_pcksize = 0;   // Bank0 PCKSIZE right after _init_endpoint()
        volatile uint32_t _dbg_bank1_pcksize = 0;   // Bank1 PCKSIZE right after _init_endpoint()
        volatile uint8_t _dbg_rxstp_ever_seen = 0;  // Was RXSTP ever set in EPINTFLAG?
        volatile uint32_t _dbg_reset_before_setup = 0; // Count of resets while RXSTP pending

        bool checkAndHandleControl() {
            _dbg_ctrl_called++;

            // Check for any control-relevant flags directly on EPINTFLAG
            // Note: Can't use EPINTSMRY here because it only reflects TRCPT0/TRCPT1,
            // not RXSTP (SETUP received). SETUP packets are missed if we check EPINTSMRY.
            uint8_t epintflag = USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg;
            uint8_t epstatus = USB->DEVICE.DeviceEndpoint[0].EPSTATUS.reg;
            _dbg_last_epintflag = epintflag;
            _dbg_last_epstatus = epstatus;
            if (epintflag && !_dbg_first_epintflag) _dbg_first_epintflag = epintflag;
            if (epintflag & USB_DEVICE_EPINTFLAG_RXSTP) _dbg_rxstp_ever_seen = 1;
            if (epintflag & USB_DEVICE_EPINTFLAG_TRFAIL1) {
                _dbg_trfail1_count++;
                _dbg_setup_state_at_trfail = setup_state;
            }

            // CRITICAL FIX: Handle TRFAIL1 (IN transfer failure) separately
            // TRFAIL1 is set when host sends IN token but device NAKs because BK1RDY=0
            // This happens when host sends IN before we've prepared the response buffer
            // Per SAMD51 datasheet 38.6.2.6: TRFAIL1 is set on transfer failure
            // We must clear it to allow future transfers, but we DON'T re-arm BK1RDY
            // because we haven't prepared data yet - that's the point of the NAK!
            if (epintflag & USB_DEVICE_EPINTFLAG_TRFAIL1) {
                USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;
                // Note: Do NOT re-arm BK1RDY here - we NAK'd because data isn't ready
                // The proper flow will set BK1RDY when _handleControlTX() prepares data
            }

            if (!(epintflag & (USB_DEVICE_EPINTFLAG_RXSTP | USB_DEVICE_EPINTFLAG_TRCPT0 | USB_DEVICE_EPINTFLAG_TRCPT1))) {
                return (epintflag & USB_DEVICE_EPINTFLAG_TRFAIL1) != 0;  // Return true if we handled TRFAIL1
            }
            _dbg_ctrl_has_flag++;

            // Use already-read epintflag instead of re-reading register (flag may be cleared by HW between reads)
            if (epintflag & USB_DEVICE_EPINTFLAG_RXSTP) {
                _dbg_setup_received++;

                // CRITICAL FIX: Clear TRFAIL1 immediately when we see RXSTP
                // Host may have sent IN token that NAK'd while SETUP was pending
                USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;

                if (false == proxy->handleSetupPacket()) {
                    _request_stall_handshake(0);
                    ackSetupRX();
                    _set_bank0_ready(0);  // Clear BK0RDY after reading SETUP
                    // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
                    USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
                    return true;
                }
                _dbg_setup_handled++;  // handleSetupPacket returned true

                // Clear BK0RDY after reading SETUP data (hardware sets it on SETUP reception)
                // Must be done before expecting any further packets
                ackSetupRX();
                _set_bank0_ready(0);
                // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
                USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;

                if (proxy->getSetup().isADeviceToHostRequest()) {
                    setup_state = DATA_IN;
                    _handleControlTX();
                } else {
                    if (0 == proxy->getSetup().length()) {
                        _send_zlp_on_in();
                        return true;
                    }
                    setup_state = DATA_OUT;
                    // Bank already cleared above
                }
            }
            else if (_is_in_send(0)) {
                _ack_in_send(0);
                _dbg_trcpt1_count++;
                _handleControlTX();
            }
            else if (_is_out_received(0)) {
                _handleControlRX();
            }

            return true;
        };

        volatile uint32_t _dbg_reset_count = 0;
        bool checkAndHandleReset() {
            if (!_is_reset()) { return false; }
            _dbg_reset_count++;

            // Debug: Check if RXSTP was pending when reset happened
            // This would indicate the race condition we're trying to fix
            if (USB->DEVICE.DeviceEndpoint[0].EPINTFLAG.reg & USB_DEVICE_EPINTFLAG_RXSTP) {
                _dbg_reset_before_setup++;
            }

            reset();
            initSetup();
            return true;
        };

        bool checkAndHandleSOF() {
            if (_is_sof()) {
                _ack_sof();
                return true;
            }
            return false;
        };

        virtual bool isConnected() { return _attached; };  // Return physical attach state, not protocol state

        bool checkAndHandleWakeupSuspend() {
            if (_is_wake_up()) {
                _ack_wake_up();
                _disable_wake_up_interrupt();
                _enable_suspend_interrupt();
                return true;
            }
            if (_is_suspend()) {
                _ack_suspend();
                _disable_suspend_interrupt();
                _enable_wake_up_interrupt();
                return true;
            }
            return false;
        }

        /*** USB Polling - workaround for hardware interrupt issue on SAMD51 ***/
        // Call this from the main loop to poll for USB events when hardware
        // interrupts are not being generated properly.
        //
        // CRITICAL ORDERING: On SAMD51, RXSTP (SETUP received) does NOT trigger
        // any of the USB interrupt lines (USB_0/1/2/3). Only TRCPT0/TRCPT1 do.
        // We must check for SETUP packets via polling, and we must do so BEFORE
        // handling bus resets to avoid losing SETUP packets in a race condition.
        //
        // Race condition scenario without proper ordering:
        // 1. Host sends SETUP packet -> RXSTP set
        // 2. poll() runs, checkAndHandleReset() runs first (no reset pending)
        // 3. Before checkAndHandleControl() runs, host times out and sends bus reset
        // 4. EORST flag set, but we haven't processed RXSTP yet
        // 5. Next poll() iteration: checkAndHandleReset() clears state
        // 6. SETUP packet is lost
        //
        // Solution: Check and handle SETUP (control) FIRST, before reset handling.
        void poll() {
            if (!_inited) return;
            _dbg_poll_count++;

            // CRITICAL: Handle endpoint events FIRST, especially SETUP packets
            // This ensures we process any pending SETUP before a bus reset clears state
            // SETUP packets (RXSTP) don't trigger USB interrupts on SAMD51!
            checkAndHandleControl();
            checkAndHandleEndpoint();

            // Handle device-level events (what USB_0_Handler would do)
            // These are checked AFTER endpoint events to avoid race conditions
            checkAndHandleReset();
            checkAndHandleWakeupSuspend();
            checkAndHandleVbusChange();
        }

        /*** Bulk/Interrupt Transfer handling ***/

        bool transfer(const uint8_t ep, USB_DMA_Descriptor& desc) {
            g_usb_transfer_calls++;
            g_usb_config_at_transfer = config_number;

            // Block transfers if not attached OR not configured
            // During re-enumeration, config_number is temporarily 0
            // Data sent before configuration is lost/queued without host polling
            if (!_attached || !config_number) {
                g_usb_transfer_failed++;
                return false;
            }

            // Store transfer info
            _transfers[ep].buffer = desc.buffer_address;
            _transfers[ep].total_length = desc.buffer_length;
            _transfers[ep].transferred = 0;
            _transfers[ep].active = true;

            _transfer_active_by_endpoint |= (1 << ep);

            if (_is_endpoint_a_tx_in(ep)) {
                // IN transfer - copy data to endpoint buffer and send
                _startINTransfer(ep);
            } else {
                // OUT transfer - prepare to receive
                g_usb_transfer_out_started++;
                _startOUTTransfer(ep);
            }

            _enable_endpoint_interrupt(ep);
            return true;
        };

        void _startINTransfer(uint8_t ep) {
            TransferState &ts = _transfers[ep];
            if (!ts.active) return;

            // Step 1: Wait for previous transfer to complete (BK1RDY must be clear)
            uint32_t timeout = 10000;
            while ((USB->DEVICE.DeviceEndpoint[ep].EPSTATUS.reg & USB_DEVICE_EPSTATUS_BK1RDY) && --timeout) {
                // BK1RDY should already be clear from previous transfer completion
            }

            uint32_t remaining = ts.total_length - ts.transferred;
            char *src_buffer = ts.buffer + ts.transferred;

            // Limit to 64 bytes per USB packet (Full-Speed bulk endpoint max)
            uint32_t transfer_size = (remaining > 64) ? 64 : remaining;
            char *dma_buffer = src_buffer;

            // CRITICAL FIX: SAMD51 USB DMA requires 4-byte aligned addresses for IN transfers
            // Always copy through aligned endpoint buffer when source is misaligned
            // This fixes corruption on ALL transfers, not just small ones
            if (((uint32_t)src_buffer & 0x3) != 0) {
                // Copy to aligned endpoint buffer (bank 1 for IN)
                dma_buffer = (char *)_ep_buffers[ep][1];
                memcpy(dma_buffer, src_buffer, transfer_size);
            }

            // Step 2: Set up descriptor with (now aligned) buffer address
            _ep_descriptors[ep].DeviceDescBank[1].ADDR.reg = (uint32_t)dma_buffer;

            // Clear PCKSIZE first, then set values
            _ep_descriptors[ep].DeviceDescBank[1].PCKSIZE.reg = 0;
            __DSB();

            uint32_t pcksize = USB_DEVICE_PCKSIZE_SIZE(3) |  // 64-byte max packet
                               USB_DEVICE_PCKSIZE_BYTE_COUNT(transfer_size);
            _ep_descriptors[ep].DeviceDescBank[1].PCKSIZE.reg = pcksize;

            // Step 3: Memory barrier to ensure descriptor writes complete
            __DSB();

            // Step 4: Set BK1RDY - signals IN data is ready for transmission
            _set_bank1_ready(ep);

            // Step 5: Clear TRFAIL1 AFTER BK1RDY (per TinyUSB pattern)
            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;

            _enable_in_send_interrupt(ep);
        }

        void _startOUTTransfer(uint8_t ep) {
            TransferState &ts = _transfers[ep];
            if (!ts.active) return;

            uint32_t remaining = ts.total_length - ts.transferred;

            // Step 1: Point ADDR directly to user buffer - USB hardware will DMA to there
            _ep_descriptors[ep].DeviceDescBank[0].ADDR.reg = (uint32_t)(ts.buffer + ts.transferred);

            // Step 2: Build PCKSIZE as full register write (avoid R-M-W race with hardware)
            // SIZE=3 means 64-byte max packet size for Full-Speed bulk endpoint
            uint32_t pcksize = USB_DEVICE_PCKSIZE_SIZE(3);  // 64 bytes max

            // For multi-packet OUT: set MULTI_PACKET_SIZE to expected total
            // Hardware will accumulate packets until BYTE_COUNT reaches MULTI_PACKET_SIZE
            if (remaining > 64) {
                // Multi-packet: round up to multiple of 64 for MULTI_PACKET_SIZE
                uint32_t multi_size = (remaining + 63) & ~63;
                pcksize |= USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(multi_size) |
                           USB_DEVICE_PCKSIZE_BYTE_COUNT(0);
            } else {
                pcksize |= USB_DEVICE_PCKSIZE_MULTI_PACKET_SIZE(0) |
                           USB_DEVICE_PCKSIZE_BYTE_COUNT(0);
            }
            _ep_descriptors[ep].DeviceDescBank[0].PCKSIZE.reg = pcksize;  // FULL REGISTER WRITE

            // Step 3: Memory barrier to ensure descriptor writes complete before HW access
            __DSB();

            // Step 4: Clear BK0RDY to indicate buffer is ready to receive
            _set_bank0_ready(ep);

            // Step 5: Clear TRFAIL0 AFTER clearing BK0RDY (per TinyUSB implementation)
            // This ensures any stale transfer failure doesn't block new transfers
            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;

            _enable_out_received_interrupt(ep);
        }

        void _completeTransfer(const uint8_t ep) {
            _transfer_active_by_endpoint &= ~(1 << ep);
            _transfers[ep].active = false;

            if (_is_endpoint_a_tx_in(ep)) {
                _disable_in_send_interrupt(ep);
            } else {
                _disable_out_received_interrupt(ep);
            }

            proxy->handleTransferDone(ep);
        }

        bool checkAndHandleEndpoint() {
            bool handled = false;

            for (uint32_t ep = 1; ep <= _get_endpoint_max_nbr(); ep++) {
                if (!_is_endpoint_interrupt(ep)) continue;

                uint8_t epintflag = USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg;
                TransferState &ts = _transfers[ep];

                if (_is_endpoint_a_tx_in(ep)) {
                    // IN endpoint - check if transfer complete
                    if (_is_in_send(ep)) {
                        _ack_in_send(ep);

                        if (ts.active) {
                            // MULTI-PACKET TRANSFER FIX:
                            // We manually chunk transfers into 64-byte USB packets. Each TRCPT1
                            // completion means one 64-byte (or smaller) packet was sent.
                            // Calculate how much we just sent - same formula as _startINTransfer
                            uint32_t remaining = ts.total_length - ts.transferred;
                            uint32_t just_sent = (remaining > 64) ? 64 : remaining;
                            ts.transferred += just_sent;

                            // Clear descriptor state for next packet
                            // CRITICAL: Must clear PCKSIZE and STATUS_BK before next transfer
                            // This ensures no stale BYTE_COUNT or status bits affect the new packet
                            _ep_descriptors[ep].DeviceDescBank[1].PCKSIZE.reg = USB_DEVICE_PCKSIZE_SIZE(3);
                            _ep_descriptors[ep].DeviceDescBank[1].STATUS_BK.reg = 0;
                            __DSB();

                            // Check if more data to send
                            if (ts.transferred < ts.total_length) {
                                // Continue multi-packet transfer - send next chunk
                                _startINTransfer(ep);
                            } else {
                                // All data sent
                                _completeTransfer(ep);
                            }
                        }
                        handled = true;
                    }
                    // Handle TRFAIL1 (IN transfer failure) - host sent IN but we NAK'd
                    if (epintflag & USB_DEVICE_EPINTFLAG_TRFAIL1) {
                        USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL1;
                        // Don't re-arm - we NAK'd because data wasn't ready
                        handled = true;
                    }
                } else {
                    // OUT endpoint - handle TRFAIL0 (before checking TRCPT0)
                    // TRFAIL0 is set when host sends data but BK0RDY was set (buffer not ready)
                    // Per SAMD51 datasheet: "TRFAIL0 is set when an error is detected in
                    // the data field during an OUT or SETUP transaction"
                    if (epintflag & USB_DEVICE_EPINTFLAG_TRFAIL0) {
                        // CRITICAL ORDER (per TinyUSB): Clear BK0RDY FIRST, then TRFAIL0
                        // Re-arm the endpoint to receive by clearing BK0RDY
                        _set_bank0_ready(ep);
                        // Clear the failure flag AFTER arming endpoint
                        USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
                        handled = true;
                    }

                    // OUT endpoint - check if data received (TRCPT0)
                    if (_is_out_received(ep)) {
                        _ack_out_received(ep);

                        if (ts.active) {
                            // BYTE_COUNT contains total bytes received (hardware DMA'd directly to buffer)
                            uint32_t received = _ep_descriptors[ep].DeviceDescBank[0].PCKSIZE.bit.BYTE_COUNT;
                            ts.transferred = received;  // Hardware tracks cumulative count

                            // With multi-packet, TRCPT fires when:
                            // - BYTE_COUNT reaches MULTI_PACKET_SIZE, or
                            // - A short packet is received (less than SIZE bytes)
                            // Either way, transfer is complete
                            _completeTransfer(ep);
                        } else {
                            // No active transfer, but data arrived
                            // Re-arm endpoint to accept more data (prevent endpoint hang)
                            // Data was received to _ep_buffers but is discarded
                            _set_bank0_ready(ep);
                            // Clear TRFAIL0 AFTER BK0RDY (per TinyUSB)
                            USB->DEVICE.DeviceEndpoint[ep].EPINTFLAG.reg = USB_DEVICE_EPINTFLAG_TRFAIL0;
                        }
                        handled = true;
                    }
                }
            }

            return handled;
        };

        char * getTransferPositon(const uint8_t endpoint) {
            return _transfers[endpoint].buffer + _transfers[endpoint].transferred;
        }

        void flush(const uint8_t endpoint) {
            // No-op for now
        };

        void flushRead(const uint8_t endpoint) {
            // No-op for now
        }

        void enableRXInterrupt(const uint8_t endpoint) {
            _enable_out_received_interrupt(endpoint);
        };

        void disableRXInterrupt(const uint8_t endpoint) {
            _disable_out_received_interrupt(endpoint);
        };

        /*** Speed and configuration ***/

        static const USBDeviceSpeed_t getDeviceSpeed() {
            // SAMD51 is full-speed only
            return kUSBDeviceFullSpeed;
        }

        uint16_t getEndpointSizeFromHardware(const uint8_t &endpoint, const bool otherSpeed) {
            if (endpoint == 0) {
                return 64;  // Full-speed control endpoint
            }
            return 0;  // Let interface define size
        };

        EndpointBufferSettings_t getEndpointConfigFromHardware(const uint8_t endpoint) {
            if (endpoint == 0) {
                return getBufferSizeFlags(64) | kEndpointBufferBlocks1 | kEndpointBufferTypeControl;
            }
            return kEndpointBufferNull;
        };

        bool _vbus_changed = false;  // Don't trigger false disconnect on first poll
        bool handleVbusChange(const bool force = false) {
            _vbus_changed = true;
            return true;
        }

        bool checkAndHandleVbusChange() {
            if (!_vbus_changed) { return false; }
            _vbus_changed = false;

            if (!_inited) {
                return false;
            }

            if (!isConnected()) {
                // Clear active transfers on disconnect
                for (uint32_t ep = 0; ep < MAX_PEP_NB(); ep++) {
                    if (_transfer_active_by_endpoint & (1 << ep)) {
                        _transfer_active_by_endpoint &= ~(1 << ep);
                        _transfers[ep].active = false;
                        proxy->handleTransferDone(ep);
                    }
                }
                proxy->handleConnectionStateChanged();
                initSetup();  // Reset our local setup state
                return true;
            }

            proxy->handleConnectionStateChanged();
            _attach();
            return true;
        }
    };  // struct USBDeviceHardware

}  // namespace Motate

#endif  // SAMDUSB_ONCE
