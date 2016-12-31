/*
 utility/MotateUSB.h - Library for the Motate system
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

#ifndef MOTATEUSB_ONCE
#define MOTATEUSB_ONCE

#include "MotateUSBHelpers.h"
#include <type_traits>
#include <cinttypes>
#include <cstring> // for size_t

namespace Motate {

#pragma mark struct USBSettings_t

    extern uint8_t USBControlBuffer[512];

    struct USBSettings_t {
        const uint16_t vendorID;
        const uint16_t productID;
        const float productVersion;
        const uint8_t  attributes;
        const uint16_t powerConsumption;
    };

    extern const USBSettings_t USBSettings;

    typedef uint32_t EndpointBufferSettings_t;

#pragma mark struct USBDevice_t

    // We'll define an "abstract" base class for USBDevice<...> so we can have a type for a pointer
    // to a USBDevice<...>.

    struct USBDevice_t
    {
        // virtual bool sendDescriptorOrConfig() const = 0;
        // virtual void sendDescriptor() const = 0;
        // virtual void sendQualifierDescriptor() const = 0;
        // virtual void sendConfig(const bool other) const = 0;
        // virtual bool handleNonstandardRequest(const Setup_t &setup) = 0;

        virtual bool handleDataAvailable(const uint8_t &endpointNum, const size_t &length) = 0;
        virtual bool handleTransferDone(const uint8_t &endpointNum) = 0;

        virtual Setup_t& getSetup() = 0;
        virtual bool handleSetupPacket() = 0;

        virtual const EndpointBufferSettings_t getEndpointConfig(const uint8_t endpoint, const bool otherSpeed) = 0;
        // virtual const uint8_t getEndpointCount(uint8_t &firstEnpointNum) const = 0;
        // virtual uint16_t getEndpointSize(const uint8_t &endpointNum, const bool otherSpeed) const = 0;

        virtual void handleConnectionStateChanged() = 0;
    }; // USBDevice_t

}; // end Motate

#ifdef __AVR_XMEGA__

#include <Atmel_avr/AvrXUSB.h>

#else

#ifdef __AVR__
#include <Atmel_avr/AvrUSB.h>
#endif

#endif

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
#include <SamUSB.h>
#endif

#if defined(__SAM4E8E__) || defined(__SAM4E16E__) || defined(__SAM4E8C__) || defined(__SAM4E16C__)
#include <SamUSB.h>
#endif

#if defined(__SAMS70N19__) || defined(__SAMS70N20__) || defined(__SAMS70N21__)
#include <SamUSB.h>
#endif

namespace Motate {

#pragma mark struct USBDevice<interfaceTypes...>

    // You will need to have something like this in your project (in a .cpp file):
    //
    // const USBSettings_t Motate::USBSettings = {
    // 	/* vendorID         = */ 0x1d50,
    // 	/* productID        = */ 0x606d,
    // 	/* productVersion   = */ 0.1,
    // 	/* attributes       = */ kUSBConfigAttributeSelfPowered,
    // 	/* powerConsumption = */ 500
    // };

    // USBDevice is our primary controller class, and "owns" the interfaces.
    // USBDevice actually talks to the hardware, and marshalls data to/from the interfaces.
    // There should only be one USBDevice per hardware USB device -- there will almost always be just one.
    template<typename... interfaceTypes>
    struct USBDevice :
        USBDevice_t,
        USBDeviceHardware,
        USBMixins<USBDevice<interfaceTypes...>, interfaceTypes...>
    {
    public:
        // Shortcut typedefs
        typedef USBDevice<interfaceTypes...> _this_type;

        typedef USBDeviceHardware _hardware_type;

        typedef USBMixins<_this_type, interfaceTypes...> _mixins_type;
        typedef USBDefaultDescriptor<interfaceTypes...> _descriptor_type;
        typedef USBDescriptorConfiguration_t<interfaceTypes...> _config_type;
        typedef USBDefaultQualifier<interfaceTypes...> _qualifier_type;

        // Keep track of the endpoint usage
        static const uint8_t _interface_0_first_endpoint  = 1; // TODO: Verify with mixin control endpoint usage.
        static const uint8_t _interface_0_first_interface = 0; // TODO: Verify this!
        static const uint8_t _total_endpoints_used        = _mixins_type::total_endpoints_used;

        Setup_t setup;
        uint8_t _set_interface = 0;

        union {
            uint8_t _features_raw[2];
            struct {
                uint8_t _empty_0;
                uint8_t _remote_wakeup : 1,
                        _self_powered  : 1,
                        _empty_1 : 6;
            };
        };

        // Init
        USBDevice() :
            _hardware_type{this},
            _mixins_type{*this, _interface_0_first_endpoint, _interface_0_first_interface}
        {
            resetSetupStatus();
        };

        void resetSetupStatus() {
            _hardware_type::initSetup();

            config_number = 0;
            _set_interface = 0;

            _features_raw[0] = 0;
            _features_raw[1] = 0;
        }

        Setup_t& getSetup() override { return setup; };

        bool handleSetupPacket() override {
            // NOTE: we don't _hardware_type::ackSetupRX() in here, it happens when we return!
            // If we return false, it happens after a stall request. Otherwise it's done on it's own.

            if (SETUP != _hardware_type::setup_state) {
                // re-init setup data
                _hardware_type::initSetup();
            }

            if (8 != _hardware_type::get_byte_count(0)) {
                return false; // Error data number doesn't correspond to SETUP packet
            }

            _hardware_type::readSetupPacket(setup);

            if (!setup.isAStandardRequestType())
            {
                return handleNonstandardRequest();
            };

            // Standard Requests
            // See http://www.beyondlogic.org/usbnutshell/usb6.shtml#StandardDeviceRequests
            // and http://wiki.osdev.org/Universal_Serial_Bus#Standard_Requests
            if (setup.isAGetStatusRequest())
            {
                if( setup.isADeviceRequest() )
                {
                    if (2 != setup.length()) {
                        return false;
                    }
                    // Send the device status - Bus powered, and cannot remote wakeup the host
                    _hardware_type::writeToControl((char *)&_features_raw, 2);
                    return true;
                }
                // if( setup.isAnEndpointRequest() )
                else
                {
                    return false;
                }
            }
            else if ( setup.isAClearFeatureRequest() )
            {
                // Check which is the selected feature
                if( setup.featureToSetOrClear() == Setup_t::kSetupDeviceRemoteWakeup )
                {
                    return false;
                }
                else if( setup.featureToSetOrClear() == Setup_t::kSetupEndpointHalt )
                {
                    // TODO: support halting endpoints
                    return false;
                }

            }
            else if (setup.isASetFeatureRequest())
            {
                // Check which is the selected feature
                if( setup.featureToSetOrClear() == Setup_t::kSetupDeviceRemoteWakeup )
                {
                    return false;
                }
                if( setup.featureToSetOrClear() == Setup_t::kSetupEndpointHalt )
                {
                    // TODO: support halting endpoints
                    return false;
                }
                if( setup.featureToSetOrClear() == Setup_t::kSetupTestMode )
                {
                    // 7.1.20 Test Mode Support, 9.4.9 SetFeature
                    // TODO: support test mode
                    return false;
                }
            }
            else if (setup.isASetAddressRequest())
            {
                if (setup.length() == 0) {
                    _hardware_type::setAddressAvailable();
                    return true;
                }
            }
            else if (setup.isAGetDescriptorRequest())
            {
                return sendDescriptorOrConfig();
            }
            else if (setup.isASetDescriptorRequest())
            {
                return false; // ???
            }
            else if (setup.isAGetConfigurationRequest())
            {
                if (1 != setup.length()) {
                    return false;
                }
                _hardware_type::writeToControl((char *)&config_number, 1);
                return true;
            }
            else if (setup.isASetConfigurationRequest())
            {
                if (setup.isADeviceRequest())
                {
                    // config_number should be set to 1 for high-speed, and 2 for full-speed
                    config_number = setup.valueLow();

                    uint8_t first_endpoint, total_endpoints;
                    total_endpoints = getEndpointCount(first_endpoint);
                    for (uint8_t ep = first_endpoint; ep < total_endpoints; ep++) {
                        _init_endpoint(ep, getEndpointConfig(ep, /* otherSpeed = */ config_number == 2));
                        //endpointSizes[ep] = getEndpointSize(ep, /* otherSpeed = */ config_number == 2);
                    }

                    /* OLD CODE
                     // Enable interrupt for CDC reception from host (OUT packet)
                     udd_enable_out_received_interrupt(CDC_RX);
                     udd_enable_endpoint_interrupt(CDC_RX);
                     */

                     return true;
                }
            }
            else if (setup.isAGetInterfaceRequest())
            {
                if (1 != setup.length()) {
                    return false;
                }
                _hardware_type::writeToControl((char *)&_set_interface, 1);
                return true;
            }
            else if (setup.isASetInterfaceRequest())
            {
                _set_interface = setup.valueLow();
                return true;
            } else {
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // unknown setup type
#endif
            }

            // if we get here, there's no response, and we send a STALL
            return false;
        };

        // This prepares a string to be sent (later, from the interrupt)
        void sendString(const uint8_t stringNum) {
            int8_t length = 0;
            const char *string;
            if (0 == stringNum) {
                // Language ID
                string = (const char *)getUSBLanguageString(length);
            }
            else if ((kManufacturerStringId == stringNum) && getUSBVendorString) {
                string = (const char *)getUSBVendorString(length);
            } else if ((kProductStringId == stringNum) && getUSBProductString) {
                string = (const char *)getUSBProductString(length);
            } else if ((kSerialNumberId == stringNum) && getUSBSerialNumberString) {
                string = (const char *)getUSBSerialNumberString(length);
            } else {
                return; // This is wrong, but works...?
            }

            // Note: length is in bytes, not char16_t characters!!

            USBDescriptorStringHeader_t *string_header = new (&USBControlBuffer) USBDescriptorStringHeader_t(length);

            int16_t header_size = sizeof(USBDescriptorStringHeader_t);

            _hardware_type::writeToControl((char *)(string_header),
                                           header_size,
                                           (char *)string,
                                           length );
        };

        bool sendDescriptorOrConfig() {
            const uint8_t type = setup.valueHigh();
            if (type == kConfigurationDescriptor) {
                sendConfig(/*other = */ setup.valueLow() == 2);
                return true;
            }
            else if (type == kOtherDescriptor) {
                sendConfig(/*other = */ true);
                return true;
            }
            else if (type == kDeviceDescriptor) {
                sendDescriptor();
                return true;
            }
            else if (type == kDeviceQualifierDescriptor) {
                sendQualifierDescriptor();
                return true;
            }
            else if (type == kStringDescriptor) {
                sendString(setup.valueLow());
                return true;
            }
            else
            {
                return _mixins_type::sendSpecialDescriptorOrConfig(setup);
            }
            return false;
        };

        void sendDescriptor() {
            _descriptor_type *descriptor = new (&USBControlBuffer) _descriptor_type(USBSettings.vendorID, USBSettings.productID, USBFloatToBCD(USBSettings.productVersion), _hardware_type::getDeviceSpeed());
            char *buffer = (char *)(descriptor);
            _hardware_type::writeToControl(buffer, descriptor->Header.Size);
        };

        void sendQualifierDescriptor() {
            _qualifier_type *qualifier = new (&USBControlBuffer) _qualifier_type();
            char *buffer = (char *)(qualifier);
            _hardware_type::writeToControl(buffer, qualifier->Header.Size);
        };

        void sendConfig(const bool other) {
            _config_type *config = new (&USBControlBuffer) _config_type(USBSettings.attributes, USBSettings.powerConsumption, _hardware_type::getDeviceSpeed(), other);
            char *buffer = (char *)(config);
            _hardware_type::writeToControl(buffer, config->TotalConfigurationSize);
        };

        void handleConnectionStateChanged() override {
            _mixins_type::handleConnectionStateChangedInMixin(isConnected());
        };

        bool handleNonstandardRequest() {
            return _mixins_type::handleNonstandardRequestInMixin(setup);
        };

        bool handleDataAvailable(const uint8_t &endpointNum, const size_t &length) override {
            return _mixins_type::handleDataAvailableInMixin(endpointNum, length);
        };

        bool handleTransferDone(const uint8_t &endpointNum) override {
            return _mixins_type::handleTransferDoneInMixin(endpointNum);
        };

        const EndpointBufferSettings_t getEndpointConfig(const uint8_t endpoint, const bool otherSpeed) override {
            EndpointBufferSettings_t ebs = _hardware_type::getEndpointConfigFromHardware(endpoint);

            if (ebs == kEndpointBufferNull) {
                ebs = _mixins_type::getEndpointConfigFromMixin(endpoint, _hardware_type::getDeviceSpeed(), otherSpeed);
            }
            return ebs;
        };

        const uint8_t getEndpointCount(uint8_t &firstEnpointNum) const {
            firstEnpointNum = _interface_0_first_endpoint;
            return _total_endpoints_used + _interface_0_first_endpoint;
        };

        constexpr uint16_t getEndpointSize(const uint8_t &endpointNum, const bool otherSpeed) {
            uint16_t size = _hardware_type::getEndpointSizeFromHardware(endpointNum, otherSpeed);
            if (size == 0) {
                size = _mixins_type::getEndpointSizeFromMixin(endpointNum, _hardware_type::getDeviceSpeed(), otherSpeed);
            }
            return size;
        };
    }; // USBDevice<...>


    template<>
    struct USBDevice <> {

    };


#pragma mark struct USBMixin<>
    // Declare the base (Null) USBMixin template. This is the one that will be at the end of the chain.
    // NOTE: This intentionally does nothing, and must be specialized elesewhere to do important stuff...
    template < typename usb_parent_type, uint8_t position, typename interface0type >
    struct USBMixin {
        static const uint8_t endpoints_used = 0;
        static const uint8_t total_endpoints_used = 0;
        static const uint8_t interfaces_used = 0;

        USBMixin (usb_parent_type &usb_parent,
                  const uint8_t new_endpoint_offset,
                  const uint8_t first_interface_number
                  )
        {};

        // Delete the copy and move constructors
        USBMixin (const USBMixin&) = delete;
        USBMixin (USBMixin&&) = delete;

        static bool isNull() { return true; };
        const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) const {
            return kEndpointBufferNull;
        };
        void handleConnectionStateChangedInMixin(const bool connected) { ; };
        bool handleNonstandardRequestInMixin(Setup_t &setup) { return false; };
        bool sendSpecialDescriptorOrConfig(Setup_t &setup) const { return false; };
        constexpr uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) { return 8; };
    };

#pragma mark struct USBMixinWrapper : USBMixin<usb_parent_type, position, firstInterfaceType>, USBMixinWrapper<usb_parent_type, position+1, interfaceTypes...>
    // We need to be able to distinguish the first interface from the second, etc.
    // For example, two USBCDC interfaces will add two Serial objects to the USBDevice<>,
    // and we need a way to request the first vs the second.

    // In short, USBMixinWrapper is a compile-time array, and you use mixin<n> to retrieve the nth element.

    // The way this works is that you use the USBMixins type, which is the "container" and interface. It is also the
    // child of a chain of USBMixinWrappers.

    // Each USBMixinWrapper is the child of the USBMixin that it "contains", which is of type first_mixin. To call
    // functions of that we use first_mixin::function(...).

    // Each USBMixinWrapper is also the child of the next one in line, which is the type other_mixins. To call a function
    // down the chain, we use other_mixins::function(...). The last one in line is specialized to not call other_mixins
    // since it doesn't have one; it is the parent of the whole chain. Each parent up the chain is given a position of
    // position+1, with the first one (the direct parent USBMixins) being position=0.

    // Then we have the mixin<n> alias to the mixin_internal<n>::type. We use that to get a specific mixin in the
    // array, which works by ascending the inheritance tree until it finds a parent whose position == n. So, externally,
    // we can call (assuomg an USB object called usb): usb.mixins<1>.blah(...) to get the blah function of the second
    // mxin.

    // Most of the functions in these objects follow the pattern of:
    // return_type function (...) { return first_mixin::function(...) || other_mixins::function(...); }

    // This isolation of the array containment and the Mixins themselves allows us to have generic mixins that don't
    // have to handle all of this positional business. It also allows all of this code to be optimized away, and most
    // of this will be inlined and what we will end up with is the same as if we hand-crafted an unrolled loop of all
    // of the mixins in every function.

    template <typename usb_parent_type, uint8_t position, typename firstInterfaceType, typename... interfaceTypes>
    struct USBMixinWrapper :
        USBMixin<usb_parent_type, position, firstInterfaceType>,
        USBMixinWrapper<usb_parent_type, position+1, interfaceTypes...>
    {
        typedef USBMixin<usb_parent_type, position, firstInterfaceType> first_mixin;
        typedef USBMixinWrapper<usb_parent_type, position+1, interfaceTypes...> other_mixins;

        // We expect the USBMixin to define endpoints_used and interfaces_used
        static const uint8_t total_endpoints_used = first_mixin::endpoints_used + other_mixins::total_endpoints_used;
        static const uint8_t total_interfaces_used = first_mixin::interfaces_used + other_mixins::total_interfaces_used;

        USBMixinWrapper (usb_parent_type &usb_parent,
                         const uint8_t new_endpoint_offset,
                         const uint8_t first_interface_number
                         ) :
            first_mixin{usb_parent, new_endpoint_offset, first_interface_number},
            other_mixins(usb_parent, new_endpoint_offset+first_mixin::endpoints_used, first_interface_number+first_mixin::interfaces_used)
        {};


        template <const uint8_t n>
        struct mixin_internal {
            // The use of "typename" and "template" here are because the compiler cannot deduce that until after it has deduced it...
            // They are described nicely here: http://stackoverflow.com/a/613132/5293860

            typedef typename other_mixins::template mixin_internal<n>::type parent_mixin;
            typedef typename std::conditional<n == position, first_mixin, parent_mixin >::type type;
        };

        template <const uint8_t n>
        using mixin = typename mixin_internal<n>::type;

        // Delete the copy and move constructors
        USBMixinWrapper (const USBMixinWrapper&) = delete;
        USBMixinWrapper (USBMixinWrapper&&) = delete;

        const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) const {
            EndpointBufferSettings_t ebs = first_mixin::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);

            if (ebs == kEndpointBufferNull) {
                ebs = other_mixins::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
            }

            return ebs;
        };
        void handleConnectionStateChangedInMixin(const bool connected) {
            first_mixin::handleConnectionStateChangedInMixin(connected);
            other_mixins::handleConnectionStateChangedInMixin(connected);
        };
        bool handleNonstandardRequestInMixin(const Setup_t &setup) {
            return first_mixin::handleNonstandardRequestInMixin(setup) || other_mixins::handleNonstandardRequestInMixin(setup);
        };
        bool handleTransferDoneInMixin(const uint8_t &endpointNum) {
            return first_mixin::handleTransferDoneInMixin(endpointNum) || other_mixins::handleTransferDoneInMixin(endpointNum);
        }
        bool handleDataAvailableInMixin(const uint8_t &endpointNum, const size_t &length) {
            return first_mixin::handleDataAvailableInMixin(endpointNum, length) || other_mixins::handleDataAvailableInMixin(endpointNum, length);
        }
        bool sendSpecialDescriptorOrConfig(const Setup_t &setup) const {
            return first_mixin::sendSpecialDescriptorOrConfig(setup) || other_mixins::sendSpecialDescriptorOrConfig(setup);
        };
        constexpr uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
            uint16_t size = first_mixin::getEndpointSizeFromMixin(endpointNum, deviceSpeed, otherSpeed);
            if (size == 0) {
                size = other_mixins::getEndpointSizeFromMixin(endpointNum, deviceSpeed, otherSpeed);
            }
            return size;
        };
    };

#pragma mark struct USBMixinWrapper <usb_parent_type, position, firstInterfaceType>  : USBMixin<usb_parent_type, position, firstInterfaceType>
    template <typename usb_parent_type, uint8_t position, typename firstInterfaceType>
    struct USBMixinWrapper <usb_parent_type, position, firstInterfaceType>  : USBMixin<usb_parent_type, position, firstInterfaceType> {
        typedef USBMixin<usb_parent_type, position, firstInterfaceType> first_mixin;

        // We expect the USBMixin to define endpoints_used and interfaces_used
        static const uint8_t total_endpoints_used = first_mixin::endpoints_used;
        static const uint8_t total_interfaces_used = first_mixin::interfaces_used;

        USBMixinWrapper (usb_parent_type &usb_parent,
                         const uint8_t new_endpoint_offset,
                         const uint8_t first_interface_number
                         ) :
        first_mixin{usb_parent, new_endpoint_offset, first_interface_number}
        {};

        template <uint8_t n>
        struct mixin_internal {
            typedef std::conditional_t<n == position, first_mixin, void> type;
        };

        template <const uint8_t n>
        using mixin = typename mixin_internal<n>::type;

        // Delete the copy and move constructors
        USBMixinWrapper (const USBMixinWrapper&) = delete;
        USBMixinWrapper (USBMixinWrapper&&) = delete;


        const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) const {
            return first_mixin::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
        };
        void handleConnectionStateChangedInMixin(const bool connected) {
            first_mixin::handleConnectionStateChangedInMixin(connected);
        };
        bool handleNonstandardRequestInMixin(const Setup_t &setup) {
            return first_mixin::handleNonstandardRequestInMixin(setup);
        };
        bool handleTransferDoneInMixin(const uint8_t &endpointNum) {
            return first_mixin::handleTransferDoneInMixin(endpointNum);
        }
        bool handleDataAvailableInMixin(const uint8_t &endpointNum, const size_t &length) {
            return first_mixin::handleDataAvailableInMixin(endpointNum, length);
        }
        bool sendSpecialDescriptorOrConfig(const Setup_t &setup) const {
            return first_mixin::sendSpecialDescriptorOrConfig(setup);
        };
        constexpr uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
            return first_mixin::getEndpointSizeFromMixin(endpointNum, deviceSpeed, otherSpeed);
        };
    };

#pragma mark struct USBMixins : USBMixinWrapper<usb_parent_type, 0, interfaceTypes...>
    template <typename usb_parent_type, typename... interfaceTypes>
    struct USBMixins : USBMixinWrapper<usb_parent_type, 0, interfaceTypes...> {

        typedef USBMixinWrapper<usb_parent_type, 0, interfaceTypes...> other_mixins;

        USBMixins (usb_parent_type &usb_parent,
                   const uint8_t new_endpoint_offset,
                   const uint8_t first_interface_number
                   ) :
            other_mixins{usb_parent, new_endpoint_offset, first_interface_number}
        {};

        // Delete the copy and move constructors
        USBMixins (const USBMixins&) = delete;
        USBMixins (USBMixins&&) = delete;

        const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) const {
            return other_mixins::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
        };
        void handleConnectionStateChangedInMixin(const bool connected) {
            other_mixins::handleConnectionStateChangedInMixin(connected);
        };
        bool handleNonstandardRequestInMixin(const Setup_t &setup) {
            return other_mixins::handleNonstandardRequestInMixin(setup);
        };
        bool handleTransferDoneInMixin(const uint8_t &endpointNum) {
            return other_mixins::handleTransferDoneInMixin(endpointNum);
        }
        bool handleDataAvailableInMixin(const uint8_t &endpointNum, const size_t &length) {
            return other_mixins::handleDataAvailableInMixin(endpointNum, length);
        }
        bool sendSpecialDescriptorOrConfig(const Setup_t &setup) const {
            return other_mixins::sendSpecialDescriptorOrConfig(setup);
        };
        static constexpr uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
            return other_mixins::getEndpointSizeFromMixin(endpointNum, deviceSpeed, otherSpeed);
        };
    };

#pragma mark struct USBDefaultDescriptor <interfaceType> : USBDescriptorDevice_t
    // Forward declaration to establish that this is a variadic template.
    template < typename... interfaceOtherTypes >
    struct USBDefaultDescriptor;

    // For only one interface, we'll use this default (unless it's explicitly overridden)
    template < typename interfaceType >
    struct USBDefaultDescriptor <interfaceType> : USBDescriptorDevice_t {
        USBDefaultDescriptor(const uint16_t vendorID, const uint16_t productID, const uint16_t productVersionBCD, const USBDeviceSpeed_t deviceSpeed) :
        USBDescriptorDevice_t{
                              /*    USBSpecificationBCD = */ USBFloatToBCD(2.0),
                              /*                  Class = */ kNoDeviceClass,
                              /*               SubClass = */ kNoDeviceSubclass,
                              /*               Protocol = */ kNoDeviceProtocol,

                              /*          Endpoint0Size = */ (uint8_t)getEndpointSize(0, kEndpointTypeControl, deviceSpeed, false),

                              /*               VendorID = */ vendorID,
                              /*              ProductID = */ productID,
                              /*          ReleaseNumber = */ (float)productVersionBCD,

                              /*   ManufacturerStrIndex = */ kManufacturerStringId,
                              /*        ProductStrIndex = */ kProductStringId,
                              /*      SerialNumStrIndex = */ kSerialNumberId,

                              /* NumberOfConfigurations = */ 1  /* !!!!!!!!!!! FIXME */
                          }
        {};
    };


    // If we have multiple intefaces, we'll usa an Interface Association Descriptor
    // Ths is used for binding multiple descriptors together when we have multiple interfaces
    template < typename interface0type, typename interface1type, typename... interfaceOtherTypes >
    struct USBDefaultDescriptor <interface0type, interface1type, interfaceOtherTypes...> : USBDescriptorDevice_t {
        USBDefaultDescriptor(const uint16_t vendorID, const uint16_t productID, const uint16_t productVersion, const USBDeviceSpeed_t deviceSpeed) :
        USBDescriptorDevice_t{
                              /*    USBSpecificationBCD = */ USBFloatToBCD(1.1),
                              /*                  Class = */ kIADDeviceClass,
                              /*               SubClass = */ kIADDeviceSubclass,
                              /*               Protocol = */ kIADDeviceProtocol,

                              /*          Endpoint0Size = */ (uint8_t)getEndpointSize(0, kEndpointTypeControl, deviceSpeed, false),

                              /*               VendorID = */ vendorID,
                              /*              ProductID = */ productID,
                              /*          ReleaseNumber = */ (float)productVersion,

                              /*   ManufacturerStrIndex = */ kManufacturerStringId,
                              /*        ProductStrIndex = */ kProductStringId,
                              /*      SerialNumStrIndex = */ kSerialNumberId,

                              /* NumberOfConfigurations = */ 1
                          }
        {};
    };

    template < typename... interfaceOtherTypes >
    struct USBDefaultQualifier : USBDescriptorDeviceQualifier_t {
        USBDefaultQualifier() :
        USBDescriptorDeviceQualifier_t() // use the defaults
        {};
    };
} // namespace Motate

#endif
// MOTATEUSB_ONCE
