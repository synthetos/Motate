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

// This goes outside the guard! We need to ensure this happens first.
#include "MotateUSB.h"

#ifndef SAMUSB_ONCE
#define SAMUSB_ONCE

#include <functional> // for std::function<>

#include "SamCommon.h"
#include "MotateUSBHelpers.h"
#include "MotateUtilities.h"
#include "MotateUniqueID.h"
#include "MotateDebug.h"

#include "sam.h" // this should be redundant, SamCommon should have pulled it in.
#include <cstring> // for memset
#include <algorithm> // for std::min, std::max

#if !(SAMV71 || SAMV70 || SAME70 || SAMS70)
# error The current USBHS Device Driver supports only SAMV71, SAMV70, SAME70 and SAMS70.
#endif

#if 0 && (IN_DEBUGGER == 1)
    template<int32_t len>
    void usb_debug(const char (&str)[len]) { Motate::debug.write(str); };
#else
    template<int32_t len>
    void usb_debug(const char (&str)[len]) { ; };
#endif

namespace Motate {
    /*** ENDPOINT CONFIGURATION ***/

    enum USBEndpointBufferSettingsFlags_t {
        // null endpoint is all zeros
        kEndpointBufferNull            = 0,

        // endpoint direction
        kEndpointBufferOutputFromHost  = USBHS_DEVEPTCFG_EPDIR_OUT,
        kEndpointBufferInputToHost     = USBHS_DEVEPTCFG_EPDIR_IN,

        // This mask is not part of the public interface:
        kEndpointBufferDirectionMask   = USBHS_DEVEPTCFG_EPDIR,

        // buffer sizes
        kEnpointBufferSizeUpTo8        = USBHS_DEVEPTCFG_EPSIZE_8_BYTE,
        kEnpointBufferSizeUpTo16       = USBHS_DEVEPTCFG_EPSIZE_16_BYTE,
        kEnpointBufferSizeUpTo32       = USBHS_DEVEPTCFG_EPSIZE_32_BYTE,
        kEnpointBufferSizeUpTo64       = USBHS_DEVEPTCFG_EPSIZE_64_BYTE,
        kEnpointBufferSizeUpTo128      = USBHS_DEVEPTCFG_EPSIZE_128_BYTE,
        kEnpointBufferSizeUpTo256      = USBHS_DEVEPTCFG_EPSIZE_256_BYTE,
        kEnpointBufferSizeUpTo512      = USBHS_DEVEPTCFG_EPSIZE_512_BYTE,
        kEnpointBufferSizeUpTo1024     = USBHS_DEVEPTCFG_EPSIZE_1024_BYTE,

        // This mask is not part of the public interface:
        kEnpointBufferSizeMask         = USBHS_DEVEPTCFG_EPSIZE_Msk,

        // buffer "blocks" -- 2 == "ping pong"
        // Note that there must be one, or this is a null endpoint.
        kEndpointBufferBlocks1         = USBHS_DEVEPTCFG_EPBK_1_BANK,
        kEndpointBufferBlocksUpTo2     = USBHS_DEVEPTCFG_EPBK_2_BANK,
        kEndpointBufferBlocksUpTo3     = USBHS_DEVEPTCFG_EPBK_3_BANK,

        // This mask is not part of the public interface:
        kEndpointBufferBlocksMask      = USBHS_DEVEPTCFG_EPBK_Msk,

        // endpoint types (mildly redundant from the config)
        kEndpointBufferTypeControl     = USBHS_DEVEPTCFG_EPTYPE_CTRL,
        kEndpointBufferTypeIsochronous = USBHS_DEVEPTCFG_EPTYPE_ISO,
        kEndpointBufferTypeBulk        = USBHS_DEVEPTCFG_EPTYPE_BLK,
        kEndpointBufferTypeInterrupt   = USBHS_DEVEPTCFG_EPTYPE_INTRPT,

        // This mask is not part of the public interface:
        kEndpointBufferTypeMask        = USBHS_DEVEPTCFG_EPTYPE_Msk
    };

    // Convert from number to EndpointBufferSettings_t
    // This should optimize out.
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
    // I apologize, but this also opens it up to later optimization without changing user code.
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


    /*** USBDeviceHardware ***/

    struct alignas(16) USB_DMA_Descriptor {
        enum _commands {  // This enum declaration takes up no space, but is in here for name scoping.
            stop_now        = 0,  // These match those of the SAM3X8n datasheet, but downcased.
            run_and_stop    = 1,
            load_next_desc  = 2,
            run_and_link    = 3
        };

        USB_DMA_Descriptor *next_descriptor;    // The address of the next Descriptor
        char *buffer_address;                    // The address of the buffer to read/write
        struct {                                // controlData is a bit field with the settings of the descriptor
            // See SAMS70 datasheet for defintions.
            // Names used there are in the comment.
            _commands command : 2;

            bool end_transfer_enable : 1;                   // END_TR_EN
            bool end_buffer_enable : 1;                     // END_B_EN
            bool end_transfer_interrupt_enable : 1;         // END_TR_IT
            bool end_buffer_interrupt_enable : 1;           // END_BUFFIT
            bool descriptor_loaded_interrupt_enable : 1;    // DESC_LD_IT
            bool bust_lock_enable : 1;                      // BURST_LCK

            uint8_t _unused_1 : 8;

            uint16_t buffer_length : 16;                    // BUFF_LENGTH
        };


        // FUNCTIONS -- these take no space, and don't have a vtable since there's nothing 'virtual'.


        void setBuffer(char* data, uint16_t len) {
            buffer_address = data;
            buffer_length = len;
        }
    };
    struct USB_DMA_Status {
        union {
            struct {
                bool channel_enable           :  1; // CHANN_ENB
                bool channel_active           :  1; // CHANN_ACT
                uint32_t reserved0            :  2;
                bool end_transfer_status      :  1; // END_TR_ST
                bool end_buffer_status        :  1; // END_BF_ST
                bool descriptor_loaded_status :  1; // DESC_LDST
                uint32_t reserved1            :  9;
                uint16_t buffer_count         : 16; // BUFF_COUNT
            };
            uint32_t STATUS;
        };

        // constexpr USB_DMA_Status(volatile const uint32_t s) {
        //     STATUS = s;
        // }
        //
        // USB_DMA_Status(volatile const USB_DMA_Status& other) {
        //     STATUS = other.STATUS;
        // }
    };
    struct USB_DMA_Channel_Info : USB_DMA_Descriptor {
        USB_DMA_Status status;
    };

    static auto _devdma(const uint32_t ep) {
        return reinterpret_cast<volatile USB_DMA_Channel_Info*>(USBHS->USBHS_DEVDMA + (ep-1));
    }

    static auto _devdma_status(const uint32_t ep) {
        return ((USBHS->USBHS_DEVDMA + (ep-1))->USBHS_DEVDMASTATUS);
    }

    static auto _devdma_address(const uint32_t ep) {
        return ((USBHS->USBHS_DEVDMA + (ep-1))->USBHS_DEVDMAADDRESS);
    }

    static auto _devdma_buffer_count(const uint32_t ep) {
        return (((USBHS->USBHS_DEVDMA + (ep-1))->USBHS_DEVDMASTATUS) & USBHS_DEVDMASTATUS_BUFF_COUNT_Msk) >> USBHS_DEVDMASTATUS_BUFF_COUNT_Pos;
    }

    using namespace Private::BitManipulation;

    // USBDeviceHardware actually talks to the hardware, and marshalls data to/from the interfaces.
    struct USBDeviceHardware
    {
        uint32_t _inited = 0;
        uint32_t config_number = 0;
        bool _address_available = false;

        enum USBSetupState_t {
            SETUP                  = 0, // Waiting for a SETUP packet
            DATA_OUT               = 1, // Waiting for a OUT data packet
            DATA_IN                = 2, // Waiting for a IN data packet
            HANDSHAKE_WAIT_IN_ZLP  = 3, // Waiting for a IN ZLP packet
            HANDSHAKE_WAIT_OUT_ZLP = 4, // Waiting for a OUT ZLP packet
            STALL_REQ              = 5, // STALL enabled on IN & OUT packet
        };

        USBSetupState_t setup_state = SETUP;
        USBDevice_t * const proxy;
        static USBDeviceHardware *hw;

        static const uint8_t master_control_endpoint = 0;

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

        // hold the buffer pointers to setup responses.
        // we hold two pointers and lengths, they are to be sent in order
        // this allows us to store the header and the content seperately
        struct SetupBuffer_t {
            char *buf_addr_0;
            uint16_t length_0;
            char *buf_addr_1;
            uint16_t length_1;
        } _setup_buffer;

        // Utility functions to control various things

        static constexpr uint32_t _get_endpoint_max_nbr() { return (9); };
        static constexpr uint32_t MAX_PEP_NB() { return (_get_endpoint_max_nbr() + 1); };

        // callback for after a control read is done
        std::function<void(void)> _control_read_completed_callback;

        // Freeze/unfreeze the clock

        void _freeze_clock() { USBHS->USBHS_CTRL |= USBHS_CTRL_FRZCLK; SamCommon::sync(); };
        void _unfreeze_clock() {
            USBHS->USBHS_CTRL &= ~USBHS_CTRL_FRZCLK;
            SamCommon::sync();
            // Wait for usable clock
            while ((USBHS->USBHS_SR & USBHS_SR_CLKUSABLE) != USBHS_SR_CLKUSABLE) {;}
        };

        // Start/stop interrupts

        // ** Manage wake-up event (=usb line activity)
        //     The USB controller is reactivated by a filtered non-idle signal from the lines
        void _enable_wake_up_interrupt()     { (USBHS->USBHS_DEVIER = USBHS_DEVIER_WAKEUPES); };
        void _disable_wake_up_interrupt()    { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_WAKEUPEC); };
        bool _is_wake_up_interrupt_enabled() { return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_WAKEUPE)); };
        void _ack_wake_up()                  { (USBHS->USBHS_DEVICR = USBHS_DEVICR_WAKEUPC); };
        void _raise_wake_up()                { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_WAKEUPS); };
        bool _is_wake_up()                   { return (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_WAKEUP)); };

        // ** Reset events
        void _enable_reset_interrupt()     { (USBHS->USBHS_DEVIER = USBHS_DEVIER_EORSTES); };
        void _disable_reset_interrupt()    { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_EORSTEC); };
        bool _is_reset_interrupt_enabled() { return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_EORSTE)); };
        void _ack_reset()                  { (USBHS->USBHS_DEVICR = USBHS_DEVICR_EORSTC); };
        void _raise_reset()                { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_EORSTS); };
        bool _is_reset()                   { return (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_EORST)); };

        // ** Start of frame events
        void _enable_sof_interrupt()      { (USBHS->USBHS_DEVIER = USBHS_DEVIER_SOFES); };
        void _disable_sof_interrupt()     { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_SOFEC); };
        bool _is_sof_interrupt_enabled()  { return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_SOFE)); };
        void _ack_sof()                   { (USBHS->USBHS_DEVICR = USBHS_DEVICR_SOFC); };
        void _raise_sof()                 { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_SOFS); };
        bool _is_sof()                    { return _is_sof_interrupt_enabled() && (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_SOF)); };
        uint32_t _frame_number()          { return (Rd_bitfield(USBHS->USBHS_DEVFNUM, USBHS_DEVFNUM_FNUM_Msk)); };
        bool _is_frame_number_crc_error() { return (Tst_bits(USBHS->USBHS_DEVFNUM, USBHS_DEVFNUM_FNCERR)); };

        // ** Micro start of frame events (High Speed Only)
        void _enable_msof_interrupt()     { (USBHS->USBHS_DEVIER = USBHS_DEVIER_MSOFES); };
        void _disable_msof_interrupt()    { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_MSOFEC); };
        bool _is_msof_interrupt_enabled() { return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_MSOFE)); };
        void _ack_msof()                  { (USBHS->USBHS_DEVICR = USBHS_DEVICR_MSOFC); };
        void _raise_msof()                { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_MSOFS); };
        bool _is_msof()                   { return _is_msof_interrupt_enabled() && (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_MSOF)); };
        uint32_t _micro_frame_number()    {
            return (Rd_bitfield(USBHS->USBHS_DEVFNUM, (USBHS_DEVFNUM_FNUM_Msk|USBHS_DEVFNUM_MFNUM_Msk)));
        };

        // ** Manage suspend event
        void _enable_suspend_interrupt()     { (USBHS->USBHS_DEVIER = USBHS_DEVIER_SUSPES); };
        void _disable_suspend_interrupt()    { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_SUSPEC); };
        bool _is_suspend_interrupt_enabled() { return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_SUSPE)); };
        void _ack_suspend()                  { (USBHS->USBHS_DEVICR = USBHS_DEVICR_SUSPC); };
        void _raise_suspend()                { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_SUSPS); };
        bool _is_suspend()                   { return (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_SUSP)); };

        // ** USBHS device address control
        //       These macros manage the USBHS Device address.
        // enables USB device address
        void _enable_address()                 { (Set_bits(USBHS->USBHS_DEVCTRL, USBHS_DEVCTRL_ADDEN)); };
        // disables USB device address
        void _disable_address()                { (Clr_bits(USBHS->USBHS_DEVCTRL, USBHS_DEVCTRL_ADDEN)); };
        bool _is_address_enabled()             { return (Tst_bits(USBHS->USBHS_DEVCTRL, USBHS_DEVCTRL_ADDEN)); };
        // configures the USB device address
        void _configure_address(uint32_t addr) { USBHS->USBHS_DEVCTRL = (USBHS->USBHS_DEVCTRL & ~USBHS_DEVCTRL_UADD_Msk) | (addr & USBHS_DEVCTRL_UADD_Msk); };
        // gets the currently configured USB device address
        uint32_t _get_configured_address()     { return (Rd_bitfield(USBHS->USBHS_DEVCTRL, USBHS_DEVCTRL_UADD_Msk)); };

        void setAddressAvailable() { _address_available = true; };

        // ** USBHS Device endpoint configuration
        // enables the selected endpoint
        void _enable_endpoint(uint32_t ep)     { (Set_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPEN0 << (ep))); };
        // disables the selected endpoint
        void _disable_endpoint(uint32_t ep)    { (Clr_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPEN0 << (ep))); };
        // tests if the selected endpoint is enabled
        bool _is_endpoint_enabled(uint32_t ep) { return (Tst_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPEN0 << (ep))); };
        // resets the selected endpoint
        void _reset_endpoint(uint32_t ep) {
            Set_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPRST0 << (ep));
            Clr_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPRST0 << (ep));
        };
        // Tests if the selected endpoint is being reset
        bool _is_resetting_endpoint(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPT, USBHS_DEVEPT_EPRST0 << (ep)));
        };

        // Configures the selected endpoint type
        void _configure_endpoint_type(uint32_t ep, uint32_t type) {
            (Wr_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPTYPE_Msk, type));
        };
        // Gets the configured selected endpoint type
        uint32_t _get_endpoint_type(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPTYPE_Msk));
        };
        // Enables the bank autoswitch for the selected endpoint
        void _enable_endpoint_bank_autoswitch(uint32_t ep) {
            (Set_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_AUTOSW));
        };
        // Disables the bank autoswitch for the selected endpoint
        void _disable_endpoint_bank_autoswitch(uint32_t ep) {
            (Clr_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_AUTOSW));
        };
        bool _is_endpoint_bank_autoswitch_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_AUTOSW));
        };
        // Configures the selected endpoint direction
        void _configure_endpoint_direction(uint32_t ep, uint32_t dir) {
            (Wr_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPDIR, dir));
        };
        // Gets the configured selected endpoint direction
        uint32_t _get_endpoint_direction(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPDIR));
        };
        bool _is_endpoint_in(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPDIR));
        };
        bool _is_endpoint_a_tx_in(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPDIR));
        };
        // Bounds given integer size to allowed range and rounds it up to the nearest
        // available greater size, then applies register format of USBHS controller
        // for endpoint size bit-field.
        static constexpr uint32_t _format_endpoint_size(uint32_t size) {
            return (32 - clz(((uint32_t)std::min(std::max(size, 8UL), 1024UL) << 1) - 1) - 1 - 3);
        };
        // Configures the selected endpoint size
        void _configure_endpoint_size(uint32_t ep, uint32_t size) {
            (Wr_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPSIZE_Msk, _format_endpoint_size(size)));
        };
        // Gets the configured selected endpoint size
        uint32_t _get_endpoint_size(uint32_t ep) {
            return (8 << Rd_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPSIZE_Msk));
        };
        // Configures the selected endpoint number of banks
        void _configure_endpoint_bank(uint32_t ep, uint32_t bank) {
            (Wr_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPBK_Msk, bank));
        };
        // Gets the configured selected endpoint number of banks
        uint32_t _get_endpoint_bank(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPBK_Msk)+1);
        };
        // Allocates the configuration selected endpoint in DPRAM memory
        void _allocate_memory(uint32_t ep) {
            (Set_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_ALLOC));
        };
        // un-allocates the configuration selected endpoint in DPRAM memory
        void _unallocate_memory(uint32_t ep) {
            (Clr_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_ALLOC));
        };
        bool _is_memory_allocated(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_ALLOC));
        };

        #ifndef USBHS_DEVEPTCFG_EPDIR_Pos
        // Bit pos is not defined in SAM header file but we need it.
        static constexpr uint32_t USBHS_DEVEPTCFG_EPDIR_Pos = 8;
        #endif

        // Configures selected endpoint in one step
        // void _configure_endpoint(uint32_t ep, uint32_t type, uint32_t dir, uint32_t size, uint32_t bank) {
        //     Wr_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPTYPE_Msk |
        //             USBHS_DEVEPTCFG_EPDIR  |
        //             USBHS_DEVEPTCFG_EPSIZE_Msk |
        //             USBHS_DEVEPTCFG_EPBK_Msk ,
        //             (((uint32_t)(type) << USBHS_DEVEPTCFG_EPTYPE_Pos) & USBHS_DEVEPTCFG_EPTYPE_Msk) |
        //             (((uint32_t)(dir ) << USBHS_DEVEPTCFG_EPDIR_Pos ) & USBHS_DEVEPTCFG_EPDIR) |
        //             ( (uint32_t)_format_endpoint_size(size) << USBHS_DEVEPTCFG_EPSIZE_Pos) |
        //             (((uint32_t)(bank) << USBHS_DEVEPTCFG_EPBK_Pos) & USBHS_DEVEPTCFG_EPBK_Msk));
        // };
        void _configure_endpoint(uint32_t ep, uint32_t value) {
            Wr_bits(USBHS->USBHS_DEVEPTCFG[ep], USBHS_DEVEPTCFG_EPTYPE_Msk |
                    USBHS_DEVEPTCFG_EPDIR  |
                    USBHS_DEVEPTCFG_EPSIZE_Msk |
                    USBHS_DEVEPTCFG_EPBK_Msk ,
                    value);
        };
        // Tests if current endpoint is configured
        bool _endpoint_configured(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_CFGOK));
        };
        // Returns the control direction
        // void _control_direction()     { // removing undefined EP_CONTROL, using 0
        //     (Rd_bitfield(USBHS->USBHS_DEVEPTISR[EP_CONTROL], USBHS_DEVEPTISR_CTRLDIR));
        // };

        // Resets the data toggle sequence
        // void _reset_data_toggle(uint32_t ep)   { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_RSTDTS); };
        // // Tests if the data toggle sequence is being reset
        // bool _data_toggle_reset(uint32_t ep)   {
        //     return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_RSTDT));
        // };
        // // Returns data toggle
        // void _data_toggle(uint32_t ep)         {
        //     (Rd_bitfield(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_DTSEQ_Msk));
        // };

        // *** USBHS Device control endpoint
        //     These macros control the endpoints.

        // ** USBHS Device control endpoint interrupts
        //     These macros control the endpoints interrupts.
        // Enables the selected endpoint interrupt
        void _enable_endpoint_interrupt(uint32_t ep)  { (USBHS->USBHS_DEVIER = USBHS_DEVIER_PEP_0 << (ep)); };
        // Disables the selected endpoint interrupt
        void _disable_endpoint_interrupt(uint32_t ep) { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_PEP_0 << (ep)); };
        // Tests if the selected endpoint interrupt is enabled
        bool _is_endpoint_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_PEP_0 << (ep)));
        };
        // Tests if an interrupt is triggered by the selected endpoint
        bool _is_endpoint_interrupt(uint32_t ep)         {
            return (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_PEP_0 << (ep)));
        };
        // Returns the lowest endpoint number generating an endpoint interrupt or MAX_PEP_NB if none
        uint32_t _get_interrupt_endpoint_number() {
            return (ctz(((USBHS->USBHS_DEVISR >> USBHS_DEVISR_PEP_Pos) &
                                                           (USBHS->USBHS_DEVIMR >> USBHS_DEVIMR_PEP_Pos)) |
                                                           (1UL << MAX_PEP_NB()))
                                                       );
        };

        static constexpr uint32_t USBHS_DEVISR_PEP_Pos = 12;
        static constexpr uint32_t USBHS_DEVIMR_PEP_Pos = 12;

        // *** USBHS Device control endpoint errors
        //       These macros control the endpoint errors.
        // Enables the STALL handshake
        void _enable_stall_handshake(uint32_t ep)      { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_STALLRQS); };
        // Disables the STALL handshake
        void _disable_stall_handshake(uint32_t ep)     { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_STALLRQC); };
        // Tests if STALL handshake request is running
        bool _is_endpoint_stall_requested(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_STALLRQ));
        };
        // Tests if STALL sent
        bool _is_stall(uint32_t ep)                    {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_STALLEDI));
        };
        // ACKs STALL sent
        void _ack_stall(uint32_t ep)                   { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_STALLEDIC); };
        // Raises STALL sent
        void _raise_stall(uint32_t ep)                 { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_STALLEDIS); };
        // Enables STALL sent interrupt
        void _enable_stall_interrupt(uint32_t ep)      { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_STALLEDES); };
        // Disables STALL sent interrupt
        void _disable_stall_interrupt(uint32_t ep)     { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_STALLEDEC); };
        // Tests if STALL sent interrupt is enabled
        bool _is_stall_interrupt_enabled(uint32_t ep)  {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_STALLEDE));
        };

        void _request_stall_handshake(uint32_t ep) {
            setup_state = STALL_REQ;
            _enable_stall_handshake(ep);
        }

        // Tests if NAK OUT received
        bool _is_nak_out(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_NAKOUTI));
        };
        // ACKs NAK OUT received
        void _ack_nak_out(uint32_t ep)                  { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_NAKOUTIC); };
        // Raises NAK OUT received
        void _raise_nak_out(uint32_t ep)                { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_NAKOUTIS); };
        // Enables NAK OUT interrupt
        void _enable_nak_out_interrupt(uint32_t ep)     { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_NAKOUTES); };
        // Disables NAK OUT interrupt
        void _disable_nak_out_interrupt(uint32_t ep)    { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_NAKOUTEC); };
        // Tests if NAK OUT interrupt is enabled
        bool _is_nak_out_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_NAKOUTE));
        };

        // Tests if NAK IN received
        bool _is_nak_in(uint32_t ep)                    {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_NAKINI));
        };
        // ACKs NAK IN received
        void _ack_nak_in(uint32_t ep)                   { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_NAKINIC); };
        // Raises NAK IN received
        void _raise_nak_in(uint32_t ep)                 { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_NAKINIS); };
        // Enables NAK IN interrupt
        void _enable_nak_in_interrupt(uint32_t ep)      { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_NAKINES); };
        // Disables NAK IN interrupt
        void _disable_nak_in_interrupt(uint32_t ep)     { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_NAKINEC); };
        // Tests if NAK IN interrupt is enabled
        bool _is_nak_in_interrupt_enabled(uint32_t ep)  {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_NAKINE));
        };

        // ACKs endpoint isochronous overflow interrupt
        void _ack_overflow_interrupt(uint32_t ep)        { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_OVERFIC); };
        // Raises endpoint isochronous overflow interrupt
        void _raise_overflow_interrupt(uint32_t ep)      { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_OVERFIS); };
        // Tests if an overflow occurs
        bool _is_overflow(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_OVERFI));
        };
        // Enables overflow interrupt
        void _enable_overflow_interrupt(uint32_t ep)     { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_OVERFES); };
        // Disables overflow interrupt
        void _disable_overflow_interrupt(uint32_t ep)    { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_OVERFEC); };
        // Tests if overflow interrupt is enabled
        bool _is_overflow_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_OVERFE));
        };

        // ACKs endpoint isochronous underflow interrupt
        void _ack_underflow_interrupt(uint32_t ep)        { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_UNDERFIC); };
        // Raises endpoint isochronous underflow interrupt
        void _raise_underflow_interrupt(uint32_t ep)      { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_UNDERFIS); };
        // Tests if an underflow occurs
        bool _is_underflow(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_UNDERFI));
        };
        // Enables underflow interrupt
        void _enable_underflow_interrupt(uint32_t ep)     { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_UNDERFES); };
        // Disables underflow interrupt
        void _disable_underflow_interrupt(uint32_t ep)    { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_UNDERFEC); };
        // Tests if underflow interrupt is enabled
        bool _is_underflow_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_UNDERFE));
        };

          // ** Tests if CRC ERROR ISO OUT detected
        bool _is_crc_error(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_CRCERRI));
        };
        // ACKs CRC ERROR ISO OUT detected
        void _ack_crc_error(uint32_t ep)               { (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_CRCERRIC); };
        // Raises CRC ERROR ISO OUT detected
        void _raise_crc_error(uint32_t ep)             { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_CRCERRIS); };
        // Enables CRC ERROR ISO OUT detected interrupt
        void _enable_crc_error_interrupt(uint32_t ep)  { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_CRCERRES); };
        // Disables CRC ERROR ISO OUT detected interrupt
        void _disable_crc_error_interrupt(uint32_t ep) { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_CRCERREC); };
        // Tests if CRC ERROR ISO OUT detected interrupt is enabled
        bool _is_crc_error_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_CRCERRE));
        };

        // ** @name USBHS Device control endpoint transfer
        // ** These macros control the endpoint transfer.
        // ** @{

          // ** Tests if endpoint read allowed
        bool _is_read_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_RWALL));
        };
          // ** Tests if endpoint write allowed
        bool _is_write_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_RWALL));
        };

          // ** Returns the byte count
        uint32_t _byte_count(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_BYCT_Msk));
        };
          // ** Clears FIFOCON bit
        void _ack_fifocon(uint32_t ep) { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_FIFOCONC); };
          // ** Tests if FIFOCON bit set
        bool _is_fifocon(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_FIFOCON));
        };

        uint32_t get_byte_count(uint32_t ep) { return _byte_count(ep); };

          // ** Returns the number of busy banks
        uint32_t _nb_busy_bank(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_NBUSYBK_Msk));
        };
          // ** Returns the number of the current bank
        uint32_t _current_bank(uint32_t ep) {
            return (Rd_bitfield(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_CURRBK_Msk));
        };
          // ** Kills last bank
        void _kill_last_in_bank(uint32_t ep)      { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_KILLBKS); };
        bool _is_kill_last(uint32_t ep)           {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_KILLBK));
        };
          // ** Tests if last bank killed
        bool _is_last_in_bank_killed(uint32_t ep) {
            return (!Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_KILLBK));
        };
          // ** Forces all banks full (OUT) or free (IN) interrupt
        void _force_bank_interrupt(uint32_t ep)   { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_NBUSYBKS); };
          // ** Unforces all banks full (OUT) or free (IN) interrupt
        void _unforce_bank_interrupt(uint32_t ep) { (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_NBUSYBKS); };
          // ** Enables all banks full (OUT) or free (IN) interrupt
        void _enable_bank_interrupt(uint32_t ep)  { (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_NBUSYBKES); };
          // ** Disables all banks full (OUT) or free (IN) interrupt
        void _disable_bank_interrupt(uint32_t ep) { (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_NBUSYBKEC); };
          // ** Tests if all banks full (OUT) or free (IN) interrupt enabled
        bool _is_bank_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_NBUSYBKE));
        };

          // ** Tests if SHORT PACKET received
        bool _is_short_packet(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_SHORTPACKET));
        };
          // ** ACKs SHORT PACKET received
        void _ack_short_packet(uint32_t ep)               {
            (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_SHORTPACKETC);
        };
          // ** Raises SHORT PACKET received
        void _raise_short_packet(uint32_t ep)             {
            (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_SHORTPACKETS);
        };
          // ** Enables SHORT PACKET received interrupt
        void _enable_short_packet_interrupt(uint32_t ep)  {
            (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_SHORTPACKETES);
        };
          // ** Disables SHORT PACKET received interrupt
        void _disable_short_packet_interrupt(uint32_t ep) {
            (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_SHORTPACKETEC);
        };
          // ** Tests if SHORT PACKET received interrupt is enabled
        bool _is_short_packet_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_SHORTPACKETE));
        };

          // ** Tests if SETUP received
        bool _is_setup_received(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_RXSTPI));
        };
          // ** ACKs SETUP received
        void _ack_setup_received(uint32_t ep)               {
            (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_RXSTPIC);
        };
          // ** Raises SETUP received
        void _raise_setup_received(uint32_t ep)             {
            (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_RXSTPIS);
        };
          // ** Enables SETUP received interrupt
        void _enable_setup_received_interrupt(uint32_t ep)  {
            (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_RXSTPES);
        };
          // ** Disables SETUP received interrupt
        void _disable_setup_received_interrupt(uint32_t ep) {
            (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_RXSTPEC);
        };
          // ** Tests if SETUP received interrupt is enabled
        bool _is_setup_received_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_RXSTPE));
        };

        void ackSetupRX() {
            _ack_setup_received(0);
        }

          // ** Tests if OUT received
        bool _is_out_received(uint32_t ep)                   {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_RXOUTI));
        };
          // ** ACKs OUT received
        void _ack_out_received(uint32_t ep)               {
            (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_RXOUTIC);
        };
        void ackRXOut(uint32_t ep) { _ack_out_received(ep); };

          // ** Raises OUT received
        void _raise_out_received(uint32_t ep)             {
            (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_RXOUTIS);
        };
          // ** Enables OUT received interrupt
        void _enable_out_received_interrupt(uint32_t ep)  {
            (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_RXOUTES);
        };
          // ** Disables OUT received interrupt
        void _disable_out_received_interrupt(uint32_t ep) {
            (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_RXOUTEC);
        };
          // ** Tests if OUT received interrupt is enabled
        bool _is_out_received_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_RXOUTE));
        };

          // ** Tests if IN sending
        bool _is_in_send(uint32_t ep)                      {
            return (Tst_bits(USBHS->USBHS_DEVEPTISR[ep], USBHS_DEVEPTISR_TXINI));
        };
          // ** ACKs IN sending
        void _ack_in_send(uint32_t ep)               {
            (USBHS->USBHS_DEVEPTICR[ep] = USBHS_DEVEPTICR_TXINIC);
        };
          // ** Raises IN sending
        void _raise_in_send(uint32_t ep)             {
            (USBHS->USBHS_DEVEPTIFR[ep] = USBHS_DEVEPTIFR_TXINIS);
        };
          // ** Enables IN sending interrupt
        void _enable_in_send_interrupt(uint32_t ep)  {
            (USBHS->USBHS_DEVEPTIER[ep] = USBHS_DEVEPTIER_TXINES);
        };
          // ** Disables IN sending interrupt
        void _disable_in_send_interrupt(uint32_t ep) {
            (USBHS->USBHS_DEVEPTIDR[ep] = USBHS_DEVEPTIDR_TXINEC);
        };
          // ** Tests if IN sending interrupt is enabled
        bool _is_in_send_interrupt_enabled(uint32_t ep) {
            return (Tst_bits(USBHS->USBHS_DEVEPTIMR[ep], USBHS_DEVEPTIMR_TXINE));
        };

        void enableTXSentInterrupt(uint32_t ep) { _enable_in_send_interrupt(ep); };
        void disableTXSentInterrupt(uint32_t ep) { _disable_in_send_interrupt(ep); };

        // ** Raises the selected endpoint DMA channel interrupt
      void _raise_endpoint_dma_interrupt(uint32_t ep)    { (USBHS->USBHS_DEVIFR = USBHS_DEVIFR_DMA_1 << ((ep) - 1)); };
        // ** Raises the selected endpoint DMA channel interrupt
      void _clear_endpoint_dma_interrupt(uint32_t ep)    { (USBHS->USBHS_DEVICR = USBHS_DEVISR_DMA_1 << ((ep) - 1)); };
        // ** Tests if an interrupt is triggered by the selected endpoint DMA channel
      bool _is_endpoint_dma_interrupt(uint32_t ep) {
          return (Tst_bits(USBHS->USBHS_DEVISR, USBHS_DEVISR_DMA_1 << ((ep) - 1)));
      };
        // ** Enables the selected endpoint DMA channel interrupt
      void _enable_endpoint_dma_interrupt(uint32_t ep)   { (USBHS->USBHS_DEVIER = USBHS_DEVIER_DMA_1 << ((ep) - 1)); };
        // ** Disables the selected endpoint DMA channel interrupt
      void _disable_endpoint_dma_interrupt(uint32_t ep)  { (USBHS->USBHS_DEVIDR = USBHS_DEVIDR_DMA_1 << ((ep) - 1)); };
        // ** Tests if the selected endpoint DMA channel interrupt is enabled
      bool _is_endpoint_dma_interrupt_enabled(uint32_t ep) {
          return (Tst_bits(USBHS->USBHS_DEVIMR, USBHS_DEVIMR_DMA_1 << ((ep) - 1)));
      };


//        #ifndef USBHS_RAM_ADDR
//        #define USBHS_RAM_ADDR        0xA0100000u
//        #endif

        // ** Get 64-, 32-, 16- or 8-bit access to FIFO data register of selected endpoint.
        // ** @param ep Endpoint of which to access FIFO data register
        // ** @param scale Data scale in bits: 64, 32, 16 or 8
        // ** @return Volatile 64-, 32-, 16- or 8-bit data pointer to FIFO data register
        // ** @warning It is up to the user of this macro to make sure that all accesses
        // ** are aligned with their natural boundaries except 64-bit accesses which
        // ** require only 32-bit alignment.
        // ** @warning It is up to the user of this macro to make sure that used HSB
        // ** addresses are identical to the DPRAM internal pointer modulo 32 bits.
        // #define udd_get_endpoint_fifo_access(ep, scale) (((volatile TPASTE2(U, scale) (*)[0x8000 / ((scale) / 8)])USBHS_RAM_ADDR)[(ep)])

        // Now, what that crazy thing above does is make an array of volatile 32K buffers, starting at USBHS_RAM_ADDR.
        // We'll do that in a more readable format now. Note that we only support byte-level access. This is because we
        // only plan on using it that way.

        typedef volatile uint8_t USBHS_FIFO_t[0x8000];
        static constexpr uint32_t USBHS_RAM_ADDR_c = USBHS_RAM_ADDR;
        // static constexpr USBHS_FIFO_t USBHS_FIFO[10] = (USBHS_FIFO_t *)USBHS_RAM_ADDR_c;

        static constexpr uint8_t *_dev_fifo(const uint32_t ep) {
            return (uint8_t *)(USBHS_RAM_ADDR_c + (ep * 0x8000));
        }

        // Intitalization (and attach, detach internal)

        void _init() {
            // Disable interrupts until end-of-scope
            SamCommon::InterruptDisabler disabler;


            SamCommon::enablePeripheralClock(ID_USBHS);
            SamCommon::sync();

            // FORCE disable the USB hardware:
            USBHS->USBHS_CTRL &= ~USBHS_CTRL_USBE;
            SamCommon::sync();

            // ID pin not used then force device mode
            USBHS->USBHS_CTRL = USBHS_CTRL_UIMOD_DEVICE;
            SamCommon::sync();

            // pmc_enable_upll_clock();
            PMC->CKGR_UCKR = CKGR_UCKR_UPLLCOUNT(3) | CKGR_UCKR_UPLLEN;
            /* Wait UTMI PLL Lock Status */
            while (!(PMC->PMC_SR & PMC_SR_LOCKU));

            // pmc_switch_udpck_to_upllck(CONFIG_USBCLK_DIV - 1);
            PMC->PMC_USB = PMC_USB_USBS | PMC_USB_USBDIV(0);

            // pmc_enable_udpck();
            PMC->PMC_SCER = PMC_SCER_USBCLK;

            // Configure interrupts. USB gets next-to-lowest priority.
            NVIC_SetPriority((IRQn_Type) ID_USBHS, 1UL);
            NVIC_EnableIRQ((IRQn_Type) ID_USBHS);

            // Always authorize asynchrone USB interrupts to exit of sleep mode
            // For SAM USB wake up device except BACKUP mode
            // pmc_set_fast_startup_input(PMC_FSMR_USBAL);
            PMC->PMC_FSMR |= PMC_FSMR_USBAL;

            // Enable USB hardware
            //  Enable USB macro
            USBHS->USBHS_CTRL |= USBHS_CTRL_USBE;

            // Enable High Speed
            //  Disable "Forced" Low Speed first..
            USBHS->USBHS_DEVCTRL &= ~USBHS_DEVCTRL_LS;

            //  Then enable High Speed
            /* USBHS_DEVCTRL_SPDCONF_NORMAL means:
             * "The peripheral starts in full-speed mode and performs a high-speed reset to switch to the high-speed mode if
             *  the host is high-speed capable."
             */
            USBHS->USBHS_DEVCTRL = (USBHS->USBHS_DEVCTRL & ~USBHS_DEVCTRL_SPDCONF_Msk) |
//                                    USBHS_DEVCTRL_SPDCONF_NORMAL;  // this line for 480 MBps operation
                                    USBHS_DEVCTRL_SPDCONF_FORCED_FS; // this line to limit to 12 MBps


            // _unfreeze_clock();

            // Handle any additional setup here
            _inited = 1UL;
            config_number = 0UL;

            _setup_buffer = {nullptr, 0, nullptr, 0};
            setup_state = SETUP;

            // _freeze_clock();

            _detach();
        };

        void _attach() {
            SamCommon::InterruptDisabler disabler;

            _unfreeze_clock();

            // Authorize attach if Vbus is present
            if (!isConnected()) {
                return;
            }

            USBHS->USBHS_DEVCTRL &= ~USBHS_DEVCTRL_DETACH;

            // Enable USB line events
            _enable_reset_interrupt();
            _enable_suspend_interrupt();
            _enable_wake_up_interrupt();
            _disable_sof_interrupt();
            _disable_msof_interrupt();

            // Reset following interupts flag
            //_ack_reset();
            _ack_sof();
            _ack_msof();

            // The first suspend interrupt must be forced
            // The first suspend interrupt is not detected else raise it
//            _raise_suspend();

            _ack_wake_up();
//            _freeze_clock();
        };

        bool attach() {
            if (_inited) {
                _attach();
                return true;
            }
#if IN_DEBUGGER == 1
            __asm__("BKPT"); // attach_before_init1
#endif
            return false;
        };

        void _detach() {
            _unfreeze_clock();

            // Detach device from the bus
            USBHS->USBHS_DEVCTRL |= USBHS_DEVCTRL_DETACH;

            _freeze_clock();
        };
        bool detach() {
            if (_inited) {
                _detach();
                return true;
            }
            return false;
        };

        static const EndpointBufferSettings_t _enforce_enpoint_limits(const uint8_t endpoint, EndpointBufferSettings_t config) {
            if (endpoint > 9)
                return kEndpointBufferNull;

            if (endpoint == 0) {
                if ((config & kEnpointBufferSizeMask) > kEnpointBufferSizeUpTo64)
                    config = (config & ~kEnpointBufferSizeMask) | kEnpointBufferSizeUpTo64;

                config = (config & ~kEndpointBufferBlocksMask) | kEndpointBufferBlocks1;
            } else {
                // Enpoint 1 config - max 1024b buffer, with three blocks
                // Enpoint 2 config - max 1024b buffer, with three blocks

                if ((config & kEnpointBufferSizeMask) > kEnpointBufferSizeUpTo1024)
                    config = (config & ~kEnpointBufferSizeMask) | kEnpointBufferSizeUpTo1024;

                if (endpoint < 3) {
                    if ((config & kEndpointBufferBlocksMask) > kEndpointBufferBlocksUpTo3)
                        config = (config & ~kEndpointBufferBlocksMask) | kEndpointBufferBlocksUpTo3;
                } else {
                    if ((config & kEndpointBufferBlocksMask) > kEndpointBufferBlocksUpTo2)
                        config = (config & ~kEndpointBufferBlocksMask) | kEndpointBufferBlocksUpTo2;
                }
            }

            if (config & kEndpointTypeInterrupt) {
                if (config & kEndpointBufferBlocks1) {
                    config |= USBHS_DEVEPTCFG_EPBK_1_BANK;
                } else if (config & kEndpointBufferBlocksUpTo2) {
                    config |= USBHS_DEVEPTCFG_EPBK_2_BANK;
                } else if (config & kEndpointBufferBlocksUpTo3) {
                    config |= USBHS_DEVEPTCFG_EPBK_3_BANK;
                }
            }

            return config;
        };

        void _init_endpoint(uint32_t endpoint, const uint32_t configuration) {
            endpoint = endpoint & 0xF; // EP range is 0..9, hence mask is 0xF.

            volatile uint32_t configuration_fixed = _enforce_enpoint_limits(endpoint, configuration);

            // Enable autoswitch
            if (endpoint > 0) {
                configuration_fixed |= USBHS_DEVEPTCFG_AUTOSW;
            }

            if (configuration_fixed == kEndpointBufferNull) {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // configuration not valid
#endif
            }

            // Configure EP
            // If we get here, and it's a null endpoint, this will disable it.
            _configure_endpoint(endpoint, configuration_fixed);
            // Enable EP
            _allocate_memory(endpoint);
            _enable_endpoint(endpoint);

            if (!_endpoint_configured(endpoint)) {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // endpoint not configured
#endif
            }
        };

        void reset() {
//            SamCommon::InterruptDisabler disabler;

            // this is called from the reset USB request
            _ack_reset();

            _configure_address(0);
            _enable_address();
            _address_available = false;

            // catch the case where we are disconnected and reconnected, and we had open tranfers
            if (_dma_used_by_endpoint) {
                for (uint32_t ep = 0; ep < 10; ep++) {
                    if (_dma_used_by_endpoint & (1 << ep)) {
                        _dma_used_by_endpoint &= ~(1 << ep);

                        _devdma(ep)->command = USB_DMA_Descriptor::stop_now;
                        _devdma(ep)->buffer_address = nullptr;
                        _devdma(ep)->buffer_length = 0;

                        proxy->handleTransferDone(ep);
                    }
                }
            }

            _init_endpoint(0, proxy->getEndpointConfig(0, /* otherSpeed = */ false));

            _enable_setup_received_interrupt(0);
            _enable_out_received_interrupt(0);
            _enable_endpoint_interrupt(0);
        };

        void initSetup() {
            // In case of abort of IN Data Phase:
            // No need to abort IN transfer (rise TXINI),
            // because it is automatically done by hardware when a Setup packet is received.
            // But the interrupt must be disabled to not generate interrupt TXINI after SETUP reception.

            disableTXSentInterrupt(0);
            // Ensure that an OUT ZLP gets cleared out ... just in case.
            ackRXOut(0);
            setup_state = SETUP;

            _setup_buffer = {nullptr, 0, nullptr, 0};
        };

        bool checkAndHandleSOF() {
            bool got_one = false;
            if (_is_sof()) { _ack_sof(); got_one = true; }
            if (_is_msof()) { _ack_msof(); got_one = true; }
            return got_one;
        };

        bool checkAndHandleControl() {
            if (!_is_endpoint_interrupt(0)) { return false; }

            _disable_nak_in_interrupt(0);
            _disable_nak_out_interrupt(0);

            if (_is_setup_received(0)) {
                if (false == proxy->handleSetupPacket()) {
                    _request_stall_handshake(0);
                    ackSetupRX();
                    // this was the right *kind* of request, so return true
                    return true;
                }

                // setup packet was handled.
                if (proxy->getSetup().isADeviceToHostRequest()) {
                    setup_state = DATA_IN;
                    _handleControlTX(); // go ahead and start filling the TX buffer
                } else {
                    if (0 == proxy->getSetup().length()) {
                        _send_zlp_on_in();
                        return true;
                    }
                    //
                    // if (0 == get_byte_count(0)) {
                    //     _send_zlp_on_in();
                    //     return true;
                    // }

                    setup_state = DATA_OUT;

                    // Turn on NAK detection, after clearing the NAK status just in case
                    _ack_nak_in(0);
                    _enable_nak_in_interrupt(0);
                }
            }
            else if (_is_in_send(0) && _is_in_send_interrupt_enabled(0)) {
                _handleControlTX();
            }
            else if (_is_out_received(0)) {
                // Of note: this is checked after everything above has a chance to read the OUT packet...
                _handleControlRX();
            }
            else if (_is_nak_out(0)) {
                _ack_nak_out(0);
                // udd_ctrl_overflow()
                if (!_is_in_send(0)) {

                    // We do nothing if DATA_IN = setup_state, since the OUT endpoint
                    // is already freed and OUT ZLP accepted.

                    if (HANDSHAKE_WAIT_IN_ZLP == setup_state) {
                        // We are waiting on an IN handshake, but the host
                        // asked for extra OUT data. Stall it.
                        _enable_stall_handshake(0);
                    }
                }
            }
            else if (_is_nak_in(0)) {
                _ack_nak_in(0);
                // udd_ctrl_underflow();
                if (!_is_out_received(0)) {
                    if (DATA_OUT == setup_state) {
                        // Host is asking to stop the OUT transaction
                        _send_zlp_on_in();
                    }
                    else if (HANDSHAKE_WAIT_OUT_ZLP == setup_state) {
                        // We are waiting on an OUT handshake, but the host
                        // asked for extra IN data. Stall it.
                        _enable_stall_handshake(0);
                    }
                }
            }
            else {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // endpoint interrupt went unhandled
#endif
            }

            return true;
        };

        void readSetupPacket(Setup_t &setup) {
            _readFromControl((char *)USBControlBuffer, 8);
            setup.set((char *)USBControlBuffer);
            ackSetupRX();
        }

        bool checkAndHandleReset() {
            if (!_is_reset()) { return false; }
            reset();
            initSetup();

            return true;
        };

        virtual bool isConnected() { return false; };

        bool checkAndHandleWakeupSuspend() {
//            SamCommon::InterruptDisabler disabler;

            if (_is_wake_up_interrupt_enabled() && _is_wake_up()) {
                _ack_wake_up();
                _unfreeze_clock();
                _disable_wake_up_interrupt();
                _enable_suspend_interrupt();
                return true;
            }
            if (_is_suspend_interrupt_enabled() && _is_suspend()) {
                _ack_suspend();
                _unfreeze_clock();
                _disable_suspend_interrupt();
                _enable_wake_up_interrupt();
//                _freeze_clock();
                return true;
            }

            return false;
        }
// TO IMPLEMEMNT

    #if 0
        static int16_t availableToRead(const uint8_t endpoint) {
            return _getEndpointBufferCount(endpoint);
        }

        static int16_t readByte(const uint8_t endpoint) {
            return _readByteFromEndpoint(endpoint);
        };

        static int16_t read(const uint8_t endpoint, char* buffer, int16_t length) {
            if (!config_number || length < 0)
                return -1;
            //
            //            LockEP lock(uint32_t ep);
            return _readFromEndpoint(endpoint, buffer, length);
        };

        /* Data is const. The pointer to data is not. */
        static int16_t write(const uint8_t endpoint, const char* buffer, int16_t length) {
            if (!config_number || length < 0)
                return -1;

            return _sendToEndpoint(endpoint, buffer, length);
        };
    #endif

        // This function reads the data out immediately
        int16_t _readFromControl(char* buffer, int16_t length) {
            if (length < 0)
                return -1;

            int16_t to_read = length;
            volatile uint8_t *src = _dev_fifo(0);
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

        // This function sets the _setup_buffer to write to the control channel as IN packets come in.
        void writeToControl(char* buffer_0, uint16_t length_0, char* buffer_1 = nullptr, uint16_t length_1 = 0) {
            _setup_buffer = {buffer_0, length_0, buffer_1, length_1};
        };

        void _send_zlp_on_in() {
            setup_state = HANDSHAKE_WAIT_IN_ZLP;
            // send the ZLP (by ack'ing the IN packet)
            _ack_in_send(0);
            _enable_in_send_interrupt(0);

            // Turn on NAK detection, after clearing the NAK status just in case
            _ack_nak_out(0);
            _enable_nak_out_interrupt(0);
        };

        void _send_zlp_on_out() {
            setup_state = HANDSHAKE_WAIT_OUT_ZLP;

            // Turn on NAK detection, after clearing the NAK status just in case
            _ack_nak_in(0);
            _enable_nak_in_interrupt(0);
        };

        void _handleControlTX() {
            _disable_in_send_interrupt(0);

            if (HANDSHAKE_WAIT_IN_ZLP == setup_state) {
                if (_address_available) {
                    _configure_address(proxy->getSetup().valueLow());
                    _enable_address();
                    _address_available = false;
                }

                // We were waiting for an IN to end the Setup Phase, so now we are done.
                // Either the address was set, or cleared, now we'll enable it.
                initSetup();

                return;
            }

            // Make sure we don't send more than setup.maxLength!
            // If the string is longer, then the host will have to ask again,
            //  with a bigger setup.maxLength, and probably will.

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

            volatile uint8_t *dst = _dev_fifo(0);

            // quick check to make sure we didn't get an OUT ZLP, aborting this transmission
            if (_is_out_received(0)) {
                setup_state = HANDSHAKE_WAIT_OUT_ZLP;
                return;
            }

            while (to_send) {
                if (_setup_buffer.length_0 == 0) {
                    _setup_buffer.length_0 = _setup_buffer.length_1; _setup_buffer.length_1 = 0;
                    _setup_buffer.buf_addr_0 = _setup_buffer.buf_addr_1; _setup_buffer.buf_addr_1 = nullptr;
                }
                *dst++ = *_setup_buffer.buf_addr_0;
                _setup_buffer.buf_addr_0++;
                _setup_buffer.length_0--;
                to_send--;
            }

            _ack_in_send(0);
            _enable_in_send_interrupt(0);
        };

        void _handleControlRX() {
            if (DATA_OUT != setup_state) {
                if ((HANDSHAKE_WAIT_OUT_ZLP == setup_state) ||
                    (DATA_IN == setup_state))
                {
                    // End of SETUP request:
                    // - Data IN Phase aborted,
                    // - or last Data IN Phase hidden by ZLP OUT sending quiclky,
                    // - or ZLP OUT received normaly.
                } else {
                    // Protocol error, STALL
                    _request_stall_handshake(0);
                }
                // reset SETUP phase data
                initSetup();
                return;
            }

            // All is good, we have data to read into the buffer
            if (_setup_buffer.length_0) {
                uint16_t amt_sent = (uint16_t)std::min(proxy->getSetup().length(), _setup_buffer.length_0);
                _readFromControl(_setup_buffer.buf_addr_0, amt_sent);
                _setup_buffer.buf_addr_0 += _setup_buffer.length_0;
                _setup_buffer.length_0 -= amt_sent;
                if (_control_read_completed_callback) {
                    _control_read_completed_callback();
                }

                // if we didn't send a full packet OR we the SETUP length truncated the transmission
                // we need to send a ZLP
                if ((amt_sent != _get_endpoint_size(0)) ||
                    (_setup_buffer.length_0))
                {
                    _ack_out_received(0);
                    _send_zlp_on_in();
                    return;
                }
            }

            // Free buffer of control endpoint to authorize next reception
            _ack_out_received(0);
            // Turn on NAK detection, after clearing the NAK status just in case
            _ack_nak_in(0);
            _enable_nak_in_interrupt(0);
        };

        void _completeTransfer(const uint8_t ep) {
            if (_is_endpoint_a_tx_in(ep)) {
                // case 2 or 3
                _disable_in_send_interrupt(ep); // D
            } else {
                // case 5 or 6
                _disable_out_received_interrupt(ep); // D
            }
            // C
            _disable_endpoint_dma_interrupt(ep);
            _dma_used_by_endpoint &= ~(1<<ep);
            proxy->handleTransferDone(ep);
        }

        bool checkAndHandleEndpoint() {
            bool handled = false;
            /*
             * Here we handle the endpoint interupts, essentialy coming from a DMA operation.
             *
             *  We have to take one or more of three possible actions:
             *    A) Clear TXINI/RXINI flag to ack the interrupt
             *    B) Clear FIFICON to send/empty the bank (must happen after A)
             *    C) Finish the DMA, clearing the appropriate bit in _dma_used_by_endpoint, disabling the DMA interupt,
             *       and calling proxy->handleTransferDone(ep).
             *    D) Disable TXIN/RXOUT interrupt.
             *
             *  It appears we only do A with B, and C with D, so they are AB or CD.
             *
             *  When we will see/react to an item will be marked with either:
             *    - for an endpoint TXINI/RXINI interrupt
             *    + for a DMA interrupt
             *    * for either or a mix of both
             *
             *  Example: AB(-) is handled in response to an endpoint interrupt.
             *
             * Possible cases:
             * TXIN - we get interrupts when the packet is filled (unless the interrupts were turned on early):
             *   1) The packet is empty, but DMA still has buffered data - CD(-)
             *      We DO NOT clear TXINI here, so we get another interrupt.
             *      This should NOT happen when interrupts are enabled at the right time.
             *      This has been seen after first initial connection after bootup.
             *   2) The packet is not full, but the DMA buffer is empty - actions AB(-)CD(*)
             *      Action B here will send the truncated packet.
             *   3) The packet is full, but DMA still has buffered data - actions AB(-)
             *   4) The packet is full, and the DMA buffer is empty (both on the same byte) - actions AB(-)CD(+)
             *      ASSUMES that the TXIN interrupt will happen at the same time or before the DMA interrupt.
             *
             * RXOUT - we get interrupts when a packet is EMPTIED (as much as it's going to) by DMA:
             *   5) The packet is completely read, and DMA still has buffer space available for all of it - actions AB(-)
             *   6) The packet has been partially read, but DMA buffer is full (not active) - actions CD(-)
             *      DO NOT do action B! That'll lose the remaining packet data.
             *   7) The packet is completely read and the DMA buffer is full - action AB(-)CD(+)
             */
            for (uint32_t ep = 1; ep <= _get_endpoint_max_nbr(); ep++)
            {
                bool transfer_completed = false;

                if (_is_endpoint_interrupt_enabled(ep))
                {
                    if (_is_endpoint_a_tx_in(ep))
                    {
                        // check to see if we are done sending this packet
                        // cases 1, 3, or 4
                        if (_is_in_send_interrupt_enabled(ep) &&
                            _is_in_send(ep) // This bit is set for isochronous, bulk and interrupt IN endpoints, at the same time as USBHS_DEVEPTIMRx.FIFOCON when the current bank is free.
                            )
                        {
                            auto byte_count = get_byte_count(ep);
                            auto dma_bytes_left = _devdma_buffer_count(ep);
                            if ((_get_endpoint_size(ep) == byte_count) || // case 3 or 4
                                (0 == dma_bytes_left)                     // case 2 or 4
                                )
                            {
                                _ack_in_send(ep); // A
                                _ack_fifocon(ep); // B - This bit is cleared (by writing a one to USBHS_DEVEPTIDRx.FIFOCONC bit) to send the FIFO data and to switch to the next bank.
                            }

                            if (0 == dma_bytes_left) { // case 2 or 4
                                transfer_completed = true; // C+D
                            }

                            if (0 == byte_count && dma_bytes_left > 0) { // case 1
                                // goose it!
                                transfer_completed = true; // C+D
                            }

                            handled = true;
                        }
                    } // if is a tx in endpoint

                    // otherwise we know it's a RXOUT packet, check for cases 5 - 7
                    else {
                        if (_is_out_received_interrupt_enabled(ep) &&
                            _is_out_received(ep) // This bit is set for isochronous, bulk and, interrupt OUT endpoints, at the same time as USBHS_DEVEPTIMRx.FIFOCON when the current bank is full.
                            )
                        {
                            // cases 5, 6, or 7
                            if (0 == get_byte_count(ep)) {
                                // cases 5 or 7
                                _ack_out_received(ep); // A
                                // B - This bit is cleared (by writing a one to USBHS_DEVEPTIDRx.FIFOCONC bit) to free the current bank and to switch to the next bank.
                                _ack_fifocon(ep);
                            } else {
                                // case 6
                                if (0 == _devdma_buffer_count(ep)) {
                                    transfer_completed = true; // C+D
                                }
                            }

                            handled = true;
                        }
                    } // if is a rx out endpoint
                } // is an endpoint interrupt


                if (_is_endpoint_dma_interrupt_enabled(ep) &&
                    _is_endpoint_dma_interrupt(ep))
                {
                    uint32_t ep_status = _devdma_status(ep);

                    if (ep_status & USBHS_DEVDMASTATUS_DESC_LDST && !transfer_completed) {
                        if (_is_endpoint_a_tx_in(ep)) {
                            _enable_in_send_interrupt(ep);
                        }
                    }

                    if (ep_status & USBHS_DEVDMASTATUS_END_BF_ST)
                    {
                        // case 2, 4, or 7

//                        if (_is_in_send(ep) && (0 != get_byte_count(ep))) {
//                            // case 2
//                            _ack_in_send(ep); // A
//                            _ack_fifocon(ep); // B - This bit is cleared (by writing a one to USBHS_DEVEPTIDRx.FIFOCONC bit) to send the FIFO data and to switch to the next bank.
//                        }

                        // cases 2, 4, or 7
                        transfer_completed = true;
                    }

                    handled = true;
                } // is a dma interrupt

                if (transfer_completed) {
                    _completeTransfer(ep);
                }
            } // for (ep ...)

            return handled;
        };

        uint32_t _dma_used_by_endpoint;
        bool transfer(const uint8_t ep, USB_DMA_Descriptor& desc) {
            if (!config_number) {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // endpoint not configured
#endif
                return false;
            }

            desc.command = USB_DMA_Descriptor::run_and_stop;
            // DON'T interrupt when the descriptor is loaded
            desc.descriptor_loaded_interrupt_enable = false;
            if (_is_endpoint_a_tx_in(ep)) {
                // if the endpoint is a TX IN:
                // validate the packet at DMA Buffer End (BUFF_COUNT reaches 0)
                desc.end_buffer_enable = true;
                // we use the descriptor loaded to turn on the other iterrupts
                desc.descriptor_loaded_interrupt_enable = true;
            }
            // interrupt when the DMA transfer ends because the buffer ran out
            desc.end_buffer_interrupt_enable = true;

            _dma_used_by_endpoint |= 1 << ep;

            // IMPORTANT: UOTGHS_DEVDMA[0] is endpoint 1!!
            _devdma(ep)->next_descriptor = &desc;
            _devdma(ep)->command = USB_DMA_Descriptor::load_next_desc;

            if (_is_endpoint_a_tx_in(ep)) {
                _enable_in_send_interrupt(ep);
                _enable_short_packet_interrupt(ep); // this allows the DMA to send a partial packet (badly named function)
            } else {
                _enable_out_received_interrupt(ep);
            }

            _enable_endpoint_interrupt(ep);
            _enable_endpoint_dma_interrupt(ep);

            return true;
        };

        char * getTransferPositon(const uint8_t endpoint) {
            return (char *)(_devdma_address(endpoint));
        }

        void flush(const uint8_t endpoint) {
//            _flushEndpoint(endpoint);
        };

        void flushRead(const uint8_t endpoint) {
//            _flushReadEndpoint(endpoint);
        }

        void enableRXInterrupt(const uint8_t endpoint) {
            _enable_out_received_interrupt(endpoint);
        };

        void disableRXInterrupt(const uint8_t endpoint) {
            _disable_out_received_interrupt(endpoint);
        };

        // Request the speed that the device is communicating at. It is unclear at what point this becomes valid,
        // but it's assumed that this would be decided *before* he configuration and descriptors are sent, and
        // the endpoints (other than 0) are configured.
        static const USBDeviceSpeed_t getDeviceSpeed() {
            switch (USBHS->USBHS_SR & USBHS_SR_SPEED_Msk) {
                case USBHS_SR_SPEED_HIGH_SPEED:
                    return kUSBDeviceHighSpeed;

                case USBHS_SR_SPEED_FULL_SPEED:
                    return kUSBDeviceFullSpeed;

                case USBHS_SR_SPEED_LOW_SPEED:
                    return kUSBDeviceLowSpeed;

                    // This shouldn't be possible, but "3" is a reserved value...
                default:
                    return kUSBDeviceLowSpeed;
            }
        }

        uint16_t getEndpointSizeFromHardware(const uint8_t &endpoint, const bool otherSpeed) {
            if (endpoint == 0) {
                if (getDeviceSpeed() == kUSBDeviceLowSpeed) {
                    return 8;
                }
                return 64;
            }

            // Indicate that we didn't set one...
            return 0;
        };

        EndpointBufferSettings_t getEndpointConfigFromHardware(const uint8_t endpoint) {
            if (endpoint == 0)
            {
                return getBufferSizeFlags(getEndpointSizeFromHardware(endpoint, false)) | kEndpointBufferBlocks1 | kEndpointBufferTypeControl;
            }
            return kEndpointBufferNull;
        };

        bool _vbus_changed = true;
        bool handleVbusChange(const bool force = false) {
            _vbus_changed = true;
            return true;
        }


        bool checkAndHandleVbusChange() {
            if (!_vbus_changed) { return false; }
            _vbus_changed = false;

            if (!_inited) {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // vbus_changed_before_init
#endif
                return false;
            }

            if (!isConnected()) {

                // catch the case where we are disconnected and reconnected, and we had open tranfers
                if (_dma_used_by_endpoint) {
                    for (uint32_t ep = 0; ep < 10; ep++) {
                        if (_dma_used_by_endpoint & (1 << ep)) {
                            _dma_used_by_endpoint &= ~(1 << ep);

                            _devdma(ep)->command = USB_DMA_Descriptor::stop_now;
                            _devdma(ep)->buffer_address = nullptr;
                            _devdma(ep)->buffer_length = 0;

                            proxy->handleTransferDone(ep);
                        }
                    }
                }

                _detach();
            }

            if (isConnected()) {
                _attach();
            }

            proxy->handleConnectionStateChanged();

            return true;
        };


    }; //class USBDeviceHardware


    template<typename pin_type>
    struct USBDeviceHardwareVBus : USBDeviceHardware {
        pin_type vbus_pin;

        USBDeviceHardwareVBus(USBDevice_t * const _proxy) :
            USBDeviceHardware(_proxy),
            vbus_pin {kNormal, [&]{ this->handleVbusChange(); }}
        {};

        bool isConnected() override {
            return (bool)vbus_pin;
        };

    };

}

#endif
//SAMUSB_ONCE
