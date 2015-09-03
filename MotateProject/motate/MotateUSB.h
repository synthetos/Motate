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

#ifdef __AVR_XMEGA__

#include <Atmel_avr/AvrXUSB.h>

#else

#ifdef __AVR__
#include <Atmel_avr/AvrUSB.h>
#endif

#endif

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
#include <Atmel_sam3xa/SamUSB.h>
#endif

namespace Motate {

    /* ############################################ */
    /* #                                          # */
    /* #             USB Device (etc.)            # */
    /* #                                          # */
    /* ############################################ */

    struct USBSettings_t {
        const uint16_t vendorID;
        const uint16_t productID;
        const float productVersion;
        const uint8_t  attributes;
        const uint16_t powerConsumption;
    };

    extern const USBSettings_t USBSettings;

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
        USBDeviceHardware< USBDevice<interfaceTypes...> >,
        USBMixins<USBDevice<interfaceTypes...>, interfaceTypes...>
    {
    public:
        // Shortcut typedefs
        typedef USBDevice<interfaceTypes...> _this_type;

        typedef USBDeviceHardware< USBDevice<interfaceTypes...> > _hardware_type;

        typedef USBMixins<_this_type, interfaceTypes...> _mixins_type;
        typedef USBDefaultDescriptor<interfaceTypes...> _descriptor_type;
        typedef USBDescriptorConfiguration_t<interfaceTypes...> _config_type;
        typedef USBDefaultQualifier<interfaceTypes...> _qualifier_type;

        // Keep track of the endpoint usage
        static const uint8_t _interface_0_first_endpoint  = 1; // TODO: Verify with mixin control endpoint usage.
        static const uint8_t _interface_0_first_interface = 0; // TODO: Verify this!
        static const uint8_t _total_endpoints_used        = _mixins_type::total_endpoints_used;

        // Init
        USBDevice() :
            _hardware_type{},
            _mixins_type{*this, _interface_0_first_endpoint, _interface_0_first_interface}
        {
            // USBDeviceHardware should handle all of the rest of the init
            _singleton = this;
        };

        static bool sendDescriptorOrConfig(Setup_t &setup) {
            const uint8_t type = setup.valueHigh();
            if (type == kConfigurationDescriptor) {
                sendConfig(setup.length(), /*other = */ setup.valueLow() == 2);
                return true;
            }
            else if (type == kOtherDescriptor) {
                sendConfig(setup.length(), /*other = */ true);
                return true;
            }
            else if (type == kDeviceDescriptor) {
                sendDescriptor(setup.length());
                return true;
            }
            else if (type == kDeviceQualifierDescriptor) {
                sendQualifierDescriptor(setup.length());
                return true;
            }
            else if (type == kStringDescriptor) {
                _this_type::sendString(setup.valueLow(), setup.length());
                return true;
            }
            else
            {
                return _mixins_type::sendSpecialDescriptorOrConfig(setup);
            }
            return false;
        };

        static void sendDescriptor(int16_t maxLength) {
            const _descriptor_type descriptor(USBSettings.vendorID, USBSettings.productID, USBFloatToBCD(USBSettings.productVersion), _hardware_type::getDeviceSpeed());
            int16_t length = sizeof(_descriptor_type);
            int16_t to_send = maxLength < length ? maxLength : length;
            const char *buffer = (const char *)(&descriptor);
            _this_type::writeToControl(0, buffer, to_send);
        };

        static void sendQualifierDescriptor(int16_t maxLength) {
            const _qualifier_type qualifier;
            int16_t length = sizeof(_qualifier_type);
            int16_t to_send = maxLength < length ? maxLength : length;
            const char *buffer = (const char *)(&qualifier);
            _this_type::writeToControl(0, buffer, to_send);
        };

        static void sendConfig(int16_t maxLength, const bool other) {
            const _config_type config(USBSettings.attributes, USBSettings.powerConsumption, _hardware_type::getDeviceSpeed(), other);
            int16_t length = sizeof(_config_type);
            int16_t to_send = maxLength < length ? maxLength : length;
            const char *buffer = (const char *)(&config);
            _this_type::writeToControl(0, buffer, to_send);
        };

        static bool handleNonstandardRequest(Setup_t &setup) {
            return _mixins_type::handleNonstandardRequestInMixin(setup);
        };

        static const EndpointBufferSettings_t getEndpointConfig(const uint8_t endpoint, const bool otherSpeed) {
            EndpointBufferSettings_t ebs = _hardware_type::getEndpointConfigFromHardware(endpoint);

            if (ebs == kEndpointBufferNull) {
                ebs = _mixins_type::getEndpointConfigFromMixin(endpoint, _hardware_type::getDeviceSpeed(), otherSpeed);
            }
            return ebs;
        };

        static const uint8_t getEndpointCount(uint8_t &firstEnpointNum) {
            firstEnpointNum = _interface_0_first_endpoint;
            return _total_endpoints_used + _interface_0_first_endpoint;
        };

        static uint16_t getEndpointSize(const uint8_t &endpointNum, const bool otherSpeed) {
            uint16_t size = _hardware_type::getEndpointSizeFromHardware(endpointNum, otherSpeed);
            if (size == 0) {
                size = _mixins_type::getEndpointSizeFromMixin(endpointNum, _hardware_type::getDeviceSpeed(), otherSpeed);
            }
            return size;
        };


        // This could be abused...
        static _this_type *_singleton;
    }; // USBDevice


    template<>
    struct USBDevice <> {

    };


    template<typename... interfaceOtherTypes>
    ATTR_WEAK USBDevice<interfaceOtherTypes...> *USBDevice<interfaceOtherTypes...>::_singleton;


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
        static const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) {
            return kEndpointBufferNull;
        };
        static bool handleNonstandardRequestInMixin(Setup_t &setup) { return false; };
        static bool sendSpecialDescriptorOrConfig(Setup_t &setup) { return false; };
        static uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) { return 8; };
    };

#pragma mark struct USBMixinWrapper : USBMixin<usb_parent_type, position, firstInterfaceType>, USBMixinWrapper<usb_parent_type, position+1, interfaceTypes...>
    // We need to be able to distinguish the first interface from the second, etc.
    // For example, two USBCDC interfaces will add two Serial objects to the USBDevice<>,
    // and we need a way to request the first vs the second.
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
            // The use of "typename" and "Tempalte" here are because the compiler cannot deduce that until after it has deduced it...
            // They are described nicely here: http://stackoverflow.com/a/613132/5293860

            typedef typename other_mixins::template mixin_internal<n>::type parent_mixin;
            typedef typename std::conditional<n == position, first_mixin, parent_mixin >::type type;
        };

        template <const uint8_t n>
        using mixin = typename mixin_internal<n>::type;

        // Delete the copy and move constructors
        USBMixinWrapper (const USBMixinWrapper&) = delete;
        USBMixinWrapper (USBMixinWrapper&&) = delete;

        static const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) {
            EndpointBufferSettings_t ebs = first_mixin::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);

            if (ebs == kEndpointBufferNull) {
                ebs = other_mixins::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
            }

            return ebs;
        };
        static bool handleNonstandardRequestInMixin(Setup_t &setup) {
            return first_mixin::handleNonstandardRequestInMixin(setup) || other_mixins::handleNonstandardRequestInMixin(setup);
        };
        static bool sendSpecialDescriptorOrConfig(Setup_t &setup) {
            return first_mixin::sendSpecialDescriptorOrConfig(setup) || other_mixins::sendSpecialDescriptorOrConfig(setup);
        };
        static uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
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


        static const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) {
            return first_mixin::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
        };
        static bool handleNonstandardRequestInMixin(Setup_t &setup) {
            return first_mixin::handleNonstandardRequestInMixin(setup);
        };
        static bool sendSpecialDescriptorOrConfig(Setup_t &setup) {
            return first_mixin::sendSpecialDescriptorOrConfig(setup);
        };
        static uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
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

        static const EndpointBufferSettings_t getEndpointConfigFromMixin(const uint8_t endpoint, const USBDeviceSpeed_t deviceSpeed, const bool other_speed) {
            return other_mixins::getEndpointConfigFromMixin(endpoint, deviceSpeed, other_speed);
        };
        static bool handleNonstandardRequestInMixin(Setup_t &setup) {
            return other_mixins::handleNonstandardRequestInMixin(setup);
        };
        static bool sendSpecialDescriptorOrConfig(Setup_t &setup) {
            return other_mixins::sendSpecialDescriptorOrConfig(setup);
        };
        static uint16_t getEndpointSizeFromMixin(const uint8_t &endpointNum, const USBDeviceSpeed_t deviceSpeed, const bool otherSpeed) {
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
        USBDescriptorDevice_t(
                              /*    USBSpecificationBCD = */ USBFloatToBCD(2.0),
                              /*                  Class = */ kNoDeviceClass,
                              /*               SubClass = */ kNoDeviceSubclass,
                              /*               Protocol = */ kNoDeviceProtocol,

                              /*          Endpoint0Size = */ getEndpointSize(0, kEndpointTypeControl, deviceSpeed, false),

                              /*               VendorID = */ vendorID,
                              /*              ProductID = */ productID,
                              /*          ReleaseNumber = */ productVersionBCD,

                              /*   ManufacturerStrIndex = */ kManufacturerStringId,
                              /*        ProductStrIndex = */ kProductStringId,
                              /*      SerialNumStrIndex = */ kSerialNumberId,

                              /* NumberOfConfigurations = */ 1  /* !!!!!!!!!!! FIXME */
                              )
        {};
    };


    // If we have multiple intefaces, we'll usa an Interface Association Descriptor
    // Ths is used for binding multiple descriptors together when we have multiple interfaces
    template < typename interface0type, typename interface1type, typename... interfaceOtherTypes >
    struct USBDefaultDescriptor <interface0type, interface1type, interfaceOtherTypes...> : USBDescriptorDevice_t {
        USBDefaultDescriptor(const uint16_t vendorID, const uint16_t productID, const uint16_t productVersion, const USBDeviceSpeed_t deviceSpeed) :
        USBDescriptorDevice_t(
                              /*    USBSpecificationBCD = */ USBFloatToBCD(1.1),
                              /*                  Class = */ kIADDeviceClass,
                              /*               SubClass = */ kIADDeviceSubclass,
                              /*               Protocol = */ kIADDeviceProtocol,

                              /*          Endpoint0Size = */ getEndpointSize(0, kEndpointTypeControl, deviceSpeed, false),

                              /*               VendorID = */ vendorID,
                              /*              ProductID = */ productID,
                              /*          ReleaseNumber = */ productVersion,

                              /*   ManufacturerStrIndex = */ kManufacturerStringId,
                              /*        ProductStrIndex = */ kProductStringId,
                              /*      SerialNumStrIndex = */ kSerialNumberId,

                              /* NumberOfConfigurations = */ 1
                              )
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

