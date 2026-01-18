/*
 utility/SamdPins.h - Library for the Motate system
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

#ifndef SAMDPINS_H_ONCE
#define SAMDPINS_H_ONCE

#include "sam.h"
#include "SamdCommon.h"
#include "MotateTimers.h"

#include <functional>
#include <type_traits>

namespace Motate {

    // Numbering is arbitrary:
    enum PinMode : PinMode_t {
        kUnchanged      = 0,
        kOutput         = 1,
        kInput          = 2,
        // SAMD51 supports up to 14 peripheral functions (A-N)
        // These should not be used in end-user (sketch) code.
        kPeripheralA    = 3,
        kPeripheralB    = 4,
        kPeripheralC    = 5,
        kPeripheralD    = 6,
        kPeripheralE    = 7,
        kPeripheralF    = 8,
        kPeripheralG    = 9,
        kPeripheralH    = 10,
        kPeripheralI    = 11,
        kPeripheralJ    = 12,
        kPeripheralK    = 13,
        kPeripheralL    = 14,
        kPeripheralM    = 15,
        kPeripheralN    = 16,
    };

    // Numbering is arbitrary, but bit unique for bitwise operations:
    enum PinOptions : PinOptions_t {
        kNormal         = 0,
        kTotem          = 0, // alias
        kPullUp         = 1<<1,
        kPullDown       = 1<<2,  // SAMD51 supports pull-down
        kWiredAnd       = 1<<3,
        kDriveLowOnly   = 1<<3,  // alias
        kWiredAndPull   = kWiredAnd|kPullUp,
        kDriveLowPullUp = kDriveLowOnly|kPullUp, // alias

        // SAMD51 doesn't have hardware deglitch/debounce on PORT
        // (use EIC for that), but we keep these for API compatibility
        kDeglitch       = 1<<4,
        kDebounce       = 1<<5,

        // Set the initialized value of the pin
        kStartHigh      = 1<<6,
        kStartLow       = 1<<7,

        // For use on PWM pins only!
        kPWMPinInverted = 1<<8,

        // For use on ADC pins only!
        kDifferentialPair = 1<<9,

        // SAMD51-specific: high drive strength
        kHighDriveStrength = 1<<10,
    };

    enum PinInterruptOptions : PinInterruptOptions_t {
        kPinInterruptsOff                = 0,

        kPinInterruptOnChange            = 1,

        kPinInterruptOnRisingEdge        = 1<<1,
        kPinInterruptOnFallingEdge       = 2<<1,

        kPinInterruptOnLowLevel          = 3<<1,
        kPinInterruptOnHighLevel         = 4<<1,

        kPinInterruptAdvancedMask        = ((1<<3)-1)<<1,

        /* This turns the IRQ on, but doesn't set the timer to ever trigger it. */
        kPinInterruptOnSoftwareTrigger   = 1<<4,

        kPinInterruptTypeMask            = (1<<5)-1,

        /* Set priority levels here as well: */
        kPinInterruptPriorityHighest     = 1<<5,
        kPinInterruptPriorityHigh        = 1<<6,
        kPinInterruptPriorityMedium      = 1<<7,
        kPinInterruptPriorityLow         = 1<<8,
        kPinInterruptPriorityLowest      = 1<<9,

        kPinInterruptPriorityMask        = ((1<<10) - (1<<5))
    };

    struct _pinChangeInterrupt {
        const uint32_t pc_mask; // Pin uses "mask" so we use a different name. "pc" for pinChange
        std::function<void(void)> interrupt_handler;
        _pinChangeInterrupt *next;

        _pinChangeInterrupt(const _pinChangeInterrupt &) = delete; // delete the copy constructor
        _pinChangeInterrupt &operator=(const _pinChangeInterrupt &) = delete; // delete the assignment operator
        _pinChangeInterrupt &operator=(const _pinChangeInterrupt &&) = delete; // delete the move assignment operator

        _pinChangeInterrupt(const uint32_t _mask, std::function<void(void)> &&_interrupt, _pinChangeInterrupt *&_first)
            : pc_mask{_mask}, interrupt_handler{std::move(_interrupt)}, next{nullptr}
        {
            if (_first == nullptr) {
                _first = this;
                return;
            }

            _pinChangeInterrupt *i = _first;
            while (i->next != nullptr) {
                i = i->next;
            }
            i->next = this;
        };

        void setInterrupt(std::function<void(void)> &&_interrupt)
        {
            interrupt_handler = std::move(_interrupt);
        };

        void setInterrupt(const std::function<void(void)> &_interrupt)
        {
            interrupt_handler = _interrupt;
        };
    };

    typedef uint32_t uintPort_t;

#pragma mark PortHardware
    /**************************************************
     *
     * HARDWARE LAYER: PortHardware
     *
     * SAMD51 uses the PORT peripheral with Group[n] structure
     * Each group (A, B, C, D) has 32 pins
     *
     **************************************************/

    template <unsigned char portLetter>
    struct PortHardware {
        static const uint8_t letter = portLetter;

        // Get the port group (0-3 for A-D)
        static constexpr uint8_t portGroup() {
            return portLetter - 'A';
        }

        // Get the PortGroup pointer
        constexpr PortGroup* const rawPort() const {
            return &PORT->Group[portGroup()];
        };

        // SAMD51 PORT is always-on (APBB bus), but we need to enable it once
        static void enablePeripheralClock() {
            // PORT clock is enabled by default on APBB
            // MCLK->APBBMASK.bit.PORT_ = 1; // Already enabled at reset
        }

        static _pinChangeInterrupt *_firstInterrupt;

        void setModes(const PinMode type, const uintPort_t mask) {
            switch (type) {
                case kOutput:
                    // Set direction to output
                    rawPort()->DIRSET.reg = mask;
                    // Disable peripheral mux (use as GPIO)
                    for (uint8_t i = 0; i < 32; i++) {
                        if (mask & (1u << i)) {
                            rawPort()->PINCFG[i].bit.PMUXEN = 0;
                        }
                    }
                    break;

                case kInput:
                    // Set direction to input
                    rawPort()->DIRCLR.reg = mask;
                    // Disable peripheral mux, enable input
                    for (uint8_t i = 0; i < 32; i++) {
                        if (mask & (1u << i)) {
                            rawPort()->PINCFG[i].bit.PMUXEN = 0;
                            rawPort()->PINCFG[i].bit.INEN = 1;
                        }
                    }
                    break;

                case kPeripheralA:
                case kPeripheralB:
                case kPeripheralC:
                case kPeripheralD:
                case kPeripheralE:
                case kPeripheralF:
                case kPeripheralG:
                case kPeripheralH:
                case kPeripheralI:
                case kPeripheralJ:
                case kPeripheralK:
                case kPeripheralL:
                case kPeripheralM:
                case kPeripheralN:
                {
                    // Calculate PMUX value (0=A, 1=B, ... 13=N)
                    uint8_t pmuxVal = (type - kPeripheralA);
                    for (uint8_t i = 0; i < 32; i++) {
                        if (mask & (1u << i)) {
                            // Set PMUX - each PMUX register handles 2 pins
                            uint8_t pmuxIdx = i >> 1;  // Divide by 2
                            if (i & 1) {
                                // Odd pin - use PMUXO (bits 4-7)
                                rawPort()->PMUX[pmuxIdx].bit.PMUXO = pmuxVal;
                            } else {
                                // Even pin - use PMUXE (bits 0-3)
                                rawPort()->PMUX[pmuxIdx].bit.PMUXE = pmuxVal;
                            }
                            // Enable peripheral mux
                            rawPort()->PINCFG[i].bit.PMUXEN = 1;
                        }
                    }
                    break;
                }

                default:
                    break;
            }
        };

        // Returns the mode of ONE pin
        PinMode getMode(const uintPort_t mask) const {
            // Find which pin
            uint8_t pin = 0;
            for (uint8_t i = 0; i < 32; i++) {
                if (mask & (1u << i)) {
                    pin = i;
                    break;
                }
            }

            // Check if peripheral mux is enabled
            if (rawPort()->PINCFG[pin].bit.PMUXEN) {
                uint8_t pmuxIdx = pin >> 1;
                uint8_t pmuxVal;
                if (pin & 1) {
                    pmuxVal = rawPort()->PMUX[pmuxIdx].bit.PMUXO;
                } else {
                    pmuxVal = rawPort()->PMUX[pmuxIdx].bit.PMUXE;
                }
                return static_cast<PinMode>(kPeripheralA + pmuxVal);
            }

            // GPIO mode - check direction
            return (rawPort()->DIR.reg & mask) ? kOutput : kInput;
        };

        void setOptions(const PinOptions_t options, const uintPort_t mask) {
            for (uint8_t i = 0; i < 32; i++) {
                if (mask & (1u << i)) {
                    // Handle initial output value
                    if (kStartHigh & options) {
                        rawPort()->OUTSET.reg = (1u << i);
                    } else if (kStartLow & options) {
                        rawPort()->OUTCLR.reg = (1u << i);
                    }

                    // Handle pull-up/pull-down
                    if (kPullUp & options) {
                        rawPort()->PINCFG[i].bit.PULLEN = 1;
                        rawPort()->OUTSET.reg = (1u << i);  // Pull-up
                    } else if (kPullDown & options) {
                        rawPort()->PINCFG[i].bit.PULLEN = 1;
                        rawPort()->OUTCLR.reg = (1u << i);  // Pull-down
                    } else {
                        rawPort()->PINCFG[i].bit.PULLEN = 0;
                    }

                    // Handle drive strength
                    if (kHighDriveStrength & options) {
                        rawPort()->PINCFG[i].bit.DRVSTR = 1;
                    } else {
                        rawPort()->PINCFG[i].bit.DRVSTR = 0;
                    }

                    // Input enable (needed for reading input value)
                    rawPort()->PINCFG[i].bit.INEN = 1;
                }
            }
        };

        PinOptions_t getOptions(const uintPort_t mask) {
            uint8_t pin = 0;
            for (uint8_t i = 0; i < 32; i++) {
                if (mask & (1u << i)) {
                    pin = i;
                    break;
                }
            }

            uint32_t result = kNormal;
            if (rawPort()->PINCFG[pin].bit.PULLEN) {
                if (rawPort()->OUT.reg & mask) {
                    result |= kPullUp;
                } else {
                    result |= kPullDown;
                }
            }
            if (rawPort()->PINCFG[pin].bit.DRVSTR) {
                result |= kHighDriveStrength;
            }
            return static_cast<PinOptions_t>(result);
        };

        void set(const uintPort_t mask) {
            rawPort()->OUTSET.reg = mask;
        };

        void clear(const uintPort_t mask) {
            rawPort()->OUTCLR.reg = mask;
        };

        void toggle(const uintPort_t mask) {
            rawPort()->OUTTGL.reg = mask;
        };

        void write(const uintPort_t value) {
            rawPort()->OUT.reg = value;
        };

        void write(const uintPort_t value, const uintPort_t mask) {
            rawPort()->OUT.reg = (rawPort()->OUT.reg & ~mask) | (value & mask);
        };

        uintPort_t getInputValues(const uintPort_t mask) {
            return rawPort()->IN.reg & mask;
        };

        uintPort_t getOutputValues(const uintPort_t mask) {
            return rawPort()->OUT.reg & mask;
        };

        PortGroup* portPtr() {
            return rawPort();
        };

        void setInterrupts(const uint32_t interrupts, const uintPort_t mask) {
            // SAMD51 pin interrupts are handled via EIC (External Interrupt Controller)
            // This is more complex than SAM3X/SAMS70 PIO interrupts
            // For now, we provide a basic implementation

            // TODO: Implement EIC-based interrupts
            // This requires:
            // 1. Configure GCLK for EIC
            // 2. Enable EIC peripheral clock
            // 3. Configure EIC sense for the pin
            // 4. Enable EIC interrupt in NVIC

            if (interrupts != kPinInterruptsOff) {
                // Enable input for interrupt capability
                for (uint8_t i = 0; i < 32; i++) {
                    if (mask & (1u << i)) {
                        rawPort()->PINCFG[i].bit.INEN = 1;
                    }
                }
            }
        };

        void addInterrupt(_pinChangeInterrupt *newInt) {
            _pinChangeInterrupt *i = _firstInterrupt;
            if (i == nullptr) {
                _firstInterrupt = newInt;
                return;
            }
            while (i->next != nullptr) {
                i = i->next;
            }
            i->next = newInt;
        };
    };


    /**************************************************
     *
     * BASIC PINS: _MAKE_MOTATE_PIN
     *
     **************************************************/

#define _MAKE_MOTATE_PIN(pinNum, registerChar, registerPin) \
    template<> \
    struct Pin<pinNum> : RealPin<registerChar, registerPin> { \
        static const int16_t number = pinNum; \
        static const uint8_t portLetter = (uint8_t) registerChar; \
        Pin() : RealPin<registerChar, registerPin>() {}; \
        Pin(const PinMode type, const PinOptions_t options = kNormal) : RealPin<registerChar, registerPin>(type, options) {}; \
    }; \
    template<> \
    struct ReversePinLookup<registerChar, registerPin> : Pin<pinNum> { \
        ReversePinLookup() {}; \
        ReversePinLookup(const PinMode type, const PinOptions_t options = kNormal) : Pin<pinNum>(type, options) {}; \
    };


#pragma mark IRQPin support
    /**************************************************
     *
     * PIN CHANGE INTERRUPT SUPPORT: IsIRQPin / MOTATE_PIN_INTERRUPT
     *
     **************************************************/

    template<int16_t pinNum>
    constexpr const bool IsIRQPin() { return !Pin<pinNum>::isNull(); }; // Basically return if we have a valid pin.

#define MOTATE_PIN_INTERRUPT(number) \
    template<> void Motate::IRQPin<number>::interrupt()


#pragma mark PWMOutputPin support
    /**************************************************
     *
     * PWM ("fake" analog) output pin support: _MAKE_MOTATE_PWM_PIN
     *
     **************************************************/

    #define _MAKE_MOTATE_PWM_PIN(registerChar, registerPin, timerOrPWM, peripheralAorB, invertedByDefault) \
        template<> \
        struct AvailablePWMOutputPin< ReversePinLookup<registerChar, registerPin>::number > : RealPWMOutputPin< ReversePinLookup<registerChar, registerPin>::number, timerOrPWM > { \
            typedef timerOrPWM parentTimerType; \
            static const pin_number pinNum = ReversePinLookup<registerChar, registerPin>::number; \
            AvailablePWMOutputPin() : RealPWMOutputPin<pinNum, timerOrPWM>{kPeripheral ## peripheralAorB} { pwmpin_init(invertedByDefault ? kPWMOnInverted : kPWMOn);}; \
            AvailablePWMOutputPin(const PinOptions_t options, const uint32_t freq) : RealPWMOutputPin<pinNum, timerOrPWM>{kPeripheral ## peripheralAorB, options, freq} { \
                pwmpin_init((invertedByDefault ^ ((options & kPWMPinInverted)?true:false)) ? kPWMOnInverted : kPWMOn); \
            }; \
            using RealPWMOutputPin<pinNum, timerOrPWM>::operator=; \
            /* Signal to _GetAvailablePWMOrAlike that we're here, AND a real Pin<> exists. */ \
            static constexpr bool _isAvailable() { return !ReversePinLookup<registerChar, registerPin>::isNull(); };  \
        };


#pragma mark SPI Pins support
    /**************************************************
     *
     * SPI PIN METADATA and wiring
     *
     **************************************************/

#define _MAKE_MOTATE_SPI_CS_PIN(registerChar, registerPin, spiNumber, peripheralAorB, csNum)             \
    template <>                                                                                          \
    struct SPIChipSelectPin<ReversePinLookup<registerChar, registerPin>::number>                         \
        : ReversePinLookup<registerChar, registerPin> {                                                  \
        SPIChipSelectPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral##peripheralAorB){}; \
        static constexpr bool    is_real     = true;                                                     \
        static constexpr uint8_t spiNum      = spiNumber;                                                \
        static constexpr uint8_t csNumber    = csNum;                                                    \
        static constexpr uint8_t csValue     = ~(1 << csNum);                                            \
        static constexpr bool    usesDecoder = false;                                                    \
    };

#define _MAKE_MOTATE_SPI_MISO_PIN(registerChar, registerPin, spiNumber, peripheralAorB)             \
    template <>                                                                                     \
    struct SPIMISOPin<ReversePinLookup<registerChar, registerPin>::number>                          \
        : ReversePinLookup<registerChar, registerPin> {                                             \
        SPIMISOPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral##peripheralAorB} {}; \
        static constexpr bool    is_real = true;                                                    \
        static constexpr uint8_t spiNum  = spiNumber;                                               \
    };


#define _MAKE_MOTATE_SPI_MOSI_PIN(registerChar, registerPin, spiNumber, peripheralAorB)             \
    template <>                                                                                     \
    struct SPIMOSIPin<ReversePinLookup<registerChar, registerPin>::number>                          \
        : ReversePinLookup<registerChar, registerPin> {                                             \
        SPIMOSIPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral##peripheralAorB} {}; \
        static constexpr bool    is_real = true;                                                    \
        static constexpr uint8_t spiNum  = spiNumber;                                               \
    };


#define _MAKE_MOTATE_SPI_SCK_PIN(registerChar, registerPin, spiNumber, peripheralAorB)             \
    template <>                                                                                    \
    struct SPISCKPin<ReversePinLookup<registerChar, registerPin>::number>                          \
        : ReversePinLookup<registerChar, registerPin> {                                            \
        SPISCKPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral##peripheralAorB} {}; \
        static constexpr bool    is_real = true;                                                   \
        static constexpr uint8_t spiNum  = spiNumber;                                              \
    };


#pragma mark UART / USART Pin support
    /**************************************************
     *
     * UART/USART PIN METADATA and wiring
     *
     **************************************************/

    #define _MAKE_MOTATE_UART_TX_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTTxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTTxPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB, kPullUp) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };

    #define _MAKE_MOTATE_UART_RX_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTRxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTRxPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };

    #define _MAKE_MOTATE_UART_RTS_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTRTSPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTRTSPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
            void operator=(const bool value); /*Will cause a failure if used.*/\
        };

    #define _MAKE_MOTATE_UART_CTS_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTCTSPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTCTSPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB, kPullUp) {};\
            UARTCTSPin(const PinOptions_t options, const std::function<void(void)> &&_interrupt, const uint32_t interrupt_settings = kPinInterruptOnChange|kPinInterruptPriorityMedium) : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB, options) {};\
            void setInterrupts(const uint32_t interrupts); /*Will cause a failure if used.*/\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };


} // end namespace Motate

#endif /* end of include guard: SAMDPINS_H_ONCE */
