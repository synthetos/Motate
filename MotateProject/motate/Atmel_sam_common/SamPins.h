/*
 utility/SamPins.h - Library for the Motate system
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

#ifndef SAMPINS_H_ONCE
#define SAMPINS_H_ONCE

// #include <chip.h>
#include "sam.h"
#include "SamCommon.h"
#include "MotateTimers.h"

#include <functional>   // for std::function
#include <type_traits>

namespace Motate {
    // Numbering is arbitrary:
    enum PinMode : PinMode_t {
        kUnchanged      = 0,
        kOutput         = 1,
        kInput          = 2,
        // These next two are NOT available on other platforms,
        // but cannot be masked out since they are required for
        // special pin functions. These should not be used in
        // end-user (sketch) code.
        kPeripheralA    = 3,
        kPeripheralB    = 4,
        kPeripheralC    = 5,
        kPeripheralD    = 6,
    };

    // Numbering is arbitrary, but bit unique for bitwise operations (unlike other architectures):
    enum PinOptions : PinOptions_t {
        kNormal         = 0,
        kTotem          = 0, // alias
        kPullUp         = 1<<1,
#if !defined(MOTATE_AVR_COMPATIBILITY)
        kWiredAnd       = 1<<2,
        kDriveLowOnly   = 1<<2, // alias
        kWiredAndPull   = kWiredAnd|kPullUp,
        kDriveLowPullUp = kDriveLowOnly|kPullUp, // alias
#endif // !MOTATE_AVR_COMPATIBILITY
#if !defined(MOTATE_AVR_COMPATIBILITY) && !defined(MOTATE_AVRX_COMPATIBILITY)
        kDeglitch       = 1<<4,
        kDebounce       = 1<<5,
#endif // !MOTATE_AVR_COMPATIBILITY && !MOTATE_SAM_COMPATIBILITY

        // Set the intialized value of the pin
        kStartHigh      = 1<<6,
        kStartLow       = 1<<7,

        // For use on PWM pins only!
        kPWMPinInverted = 1<<8,

        // For use on ADC pins only!
        kDifferentialPair = 1<<9,
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

        _pinChangeInterrupt(const _pinChangeInterrupt &) = delete; // delete the copy constructor, we only allow moves
        _pinChangeInterrupt &operator=(const _pinChangeInterrupt &) = delete; // delete the assigment operator, we only allow moves
        _pinChangeInterrupt &operator=(const _pinChangeInterrupt &&) = delete; // delete the move assigment operator, we only allow moves


        _pinChangeInterrupt(const uint32_t _mask, std::function<void(void)> &&_interrupt, _pinChangeInterrupt *&_first)
            : pc_mask{_mask}, interrupt_handler{std::move(_interrupt)}, next{nullptr}
        {
//            if (interrupt_handler) { // std::function returns false if the function isn't valid
                if (_first == nullptr) {
                    _first = this;
                    return;
                }

                _pinChangeInterrupt *i = _first;
                while (i->next != nullptr) {
                    i = i->next;
                }
                i->next = this;
//            }
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
     **************************************************/

    template <unsigned char portLetter>
    struct PortHardware {
        static const uint8_t letter = portLetter;

        // The constexpr functions we can define here, and get really great optimization.
        // These switch statements are handled by the compiler, not at runtime.
        constexpr Pio* const rawPort() const
        {
            switch (portLetter) {
                case 'A': return PIOA;
#ifdef PIOB
                case 'B': return PIOB;
#endif
#ifdef PIOC
                case 'C': return PIOC;
#endif
#ifdef PIOD
                case 'D': return PIOD;
#endif
            }
        };
        constexpr static const uint32_t peripheralId()
        {
            switch (portLetter) {
                case 'A': return ID_PIOA;
#ifdef PIOB
                case 'B': return ID_PIOB;
#endif
#ifdef PIOC
                case 'C': return ID_PIOC;
#endif
#ifdef PIOD
                case 'D': return ID_PIOD;
#endif
            }
        };
        constexpr const IRQn_Type _IRQn() const
        {
            switch (portLetter) {
                case 'A': return PIOA_IRQn;
#ifdef PIOB
                case 'B': return PIOB_IRQn;
#endif
#ifdef PIOC
                case 'C': return PIOC_IRQn;
#endif
#ifdef PIOD
                case 'D': return PIOD_IRQn;
#endif
            }
        };


        static _pinChangeInterrupt *_firstInterrupt;

        void setModes(const PinMode type, const uintPort_t mask) {
            switch (type) {
                case kOutput:
                    rawPort()->PIO_OER = mask ;
                    rawPort()->PIO_PER = mask ;
                    break;
                case kInput:
                    rawPort()->PIO_ODR = mask ;
                    rawPort()->PIO_PER = mask ;
                    break;
#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
                case kPeripheralA:
                    rawPort()->PIO_ABSR &= ~mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
                case kPeripheralB:
                    rawPort()->PIO_ABSR |= mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
#else
                /* From the datasheet (corrected typo based on the register info):
                 * The corresponding bit at level zero in PIO_ABCDSR1 and
                   the corresponding bit at level zero in PIO_ABCDSR2 means peripheral A is selected.

                 * The corresponding bit at level one in PIO_ABCDSR1 and
                   the corresponding bit at level zero in PIO_ABCDSR2 means peripheral B is selected.

                 * The corresponding bit at level zero in PIO_ABCDSR1 and
                   the corresponding bit at level one in PIO_ABCDSR2 means peripheral C is selected.

                 * The corresponding bit at level one in PIO_ABCDSR1 and
                   the corresponding bit at level one in PIO_ABCDSR2 means peripheral D is selected.


                 *   Truth Table:
                 *  Sel | SR2 | SR1
                 *    A |  0  |  0
                 *    B |  0  |  1
                 *    C |  1  |  0
                 *    D |  1  |  1
                 */
                case kPeripheralA:
                    rawPort()->PIO_ABCDSR[1] &= ~mask ;
                    rawPort()->PIO_ABCDSR[0] &= ~mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
                case kPeripheralB:
                    rawPort()->PIO_ABCDSR[1] &= ~mask ;
                    rawPort()->PIO_ABCDSR[0] |=  mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
                case kPeripheralC:
                    rawPort()->PIO_ABCDSR[1] |=  mask ;
                    rawPort()->PIO_ABCDSR[0] &= ~mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
                case kPeripheralD:
                    rawPort()->PIO_ABCDSR[1] |=  mask ;
                    rawPort()->PIO_ABCDSR[0] |=  mask ;
                    rawPort()->PIO_PDR = mask ;
                    break;
#endif

                default:
                    break;
            }
            /* if all pins are output, disable PIO Controller clocking, reduce power consumption */
            if ( rawPort()->PIO_OSR == 0xffffffff )
            {
                SamCommon::disablePeripheralClock(peripheralId());
            } else {
                SamCommon::enablePeripheralClock(peripheralId());
            }
        };
        // Returns the mode of ONE pin, and only Input or Output
        PinMode getMode(const uintPort_t mask) {
            return (rawPort()->PIO_OSR & mask) ? kOutput : kInput;
        };
        void setOptions(const PinOptions_t options, const uintPort_t mask) {
            if (kStartHigh & options)
            {
                rawPort()->PIO_SODR = mask ;
            } else if (kStartLow & options)
            {
                rawPort()->PIO_CODR = mask ;
            }
            if (kPullUp & options)
            {
                rawPort()->PIO_PUER = mask ;
            }
            else
            {
                rawPort()->PIO_PUDR = mask ;
            }
            if (kWiredAnd & options)
            {/*kDriveLowOnly - Enable Multidrive*/
                rawPort()->PIO_MDER = mask ;
            }
            else
            {
                rawPort()->PIO_MDDR = mask ;
            }
            if (kDeglitch & options)
            {
                rawPort()->PIO_IFER = mask ;
#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
                rawPort()->PIO_SCIFSR = mask ;
#else
                rawPort()->PIO_IFSCDR = mask ;
#endif
            }
            else
            {
                if (kDebounce & options)
                {
                    rawPort()->PIO_IFER = mask ;
#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
                    rawPort()->PIO_DIFSR = mask ;
#else
                    rawPort()->PIO_IFSCER = mask ;
#endif
                }
                else
                {
                    rawPort()->PIO_IFDR = mask ;
                }
            }
        };
        PinOptions_t getOptions(const uintPort_t mask) {
            return ((rawPort()->PIO_PUSR & mask) ? kPullUp : 0) |
            ((rawPort()->PIO_MDSR & mask) ? kWiredAnd : 0) |
            ((rawPort()->PIO_IFSR & mask) ?
             ((rawPort()->PIO_IFDGSR & mask) ? kDebounce : kDeglitch) : 0);
        };
        void set(const uintPort_t mask) {
            rawPort()->PIO_SODR = mask;
        };
        void clear(const uintPort_t mask) {
            rawPort()->PIO_CODR = mask;
        };
        void toggle(const uintPort_t mask) {
//            rawPort()->PIO_OWDR = 0xffffffff;/*Disable all registers for writing thru ODSR*/
//            rawPort()->PIO_OWER = mask;/*Enable masked registers for writing thru ODSR*/
//            rawPort()->PIO_ODSR = rawPort()->PIO_ODSR ^ mask;
            if (rawPort()->PIO_ODSR & mask) {
                clear(mask);
            } else {
                set(mask);
            }
        };
        void write(const uintPort_t value) {
            rawPort()->PIO_OWER = 0xffffffff;/*Enable all registers for writing thru ODSR*/
            rawPort()->PIO_ODSR = value;
        };
        void write(const uintPort_t value, const uintPort_t mask) {
            rawPort()->PIO_OWER = mask;/*Enable masked registers for writing thru ODSR*/
            rawPort()->PIO_ODSR = value;
            rawPort()->PIO_OWDR = mask;/*Disable masked registers for writing thru ODSR*/
        };
        uintPort_t getInputValues(const uintPort_t mask) {
            return rawPort()->PIO_PDSR & mask;
        };
        uintPort_t getOutputValues(const uintPort_t mask) {
            return rawPort()->PIO_ODSR & mask;
        };
        Pio* portPtr() {
            return rawPort;
        };
        void setInterrupts(const uint32_t interrupts, const uintPort_t mask) {
            if (interrupts != kPinInterruptsOff) {
                rawPort()->PIO_IDR = mask;

                /*Is it an "advanced" interrupt?*/
                if (interrupts & kPinInterruptAdvancedMask) {
                    rawPort()->PIO_AIMER = mask;
                    /*Is it an edge interrupt?*/
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnFallingEdge) {
                        rawPort()->PIO_ESR = mask;
                    }
                    else
                        if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel ||
                            (interrupts & kPinInterruptTypeMask) == kPinInterruptOnLowLevel) {
                            rawPort()->PIO_LSR = mask;
                        }
                    /*Rising Edge/High Level, or Falling Edge/LowLevel?*/
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel) {
                        rawPort()->PIO_REHLSR = mask;
                    }
                    else
                    {
                        rawPort()->PIO_FELLSR = mask;
                    }
                }
                else
                {
                    rawPort()->PIO_AIMDR = mask;
                }

                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHighest) {
                        NVIC_SetPriority(_IRQn(), 0);
                    }
                    else if (interrupts & kPinInterruptPriorityHigh) {
                        NVIC_SetPriority(_IRQn(), 1);
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        NVIC_SetPriority(_IRQn(), 2);
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        NVIC_SetPriority(_IRQn(), 3);
                    }
                    else if (interrupts & kPinInterruptPriorityLowest) {
                        NVIC_SetPriority(_IRQn(), 4);
                    }
                }
                /* Enable the IRQ */
                NVIC_EnableIRQ(_IRQn());
                /* Enable the interrupt */
                rawPort()->PIO_IER = mask;
            } else {
                rawPort()->PIO_IDR = mask;
                if (rawPort()->PIO_ISR == 0)
                    NVIC_DisableIRQ(_IRQn());
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



#pragma mark ADC_Module/ACD_Pin
    /**************************************************
     *
     * PIN CHANGE INTERRUPT SUPPORT: ADC_Module/ACD_Pin
     *
     **************************************************/

    constexpr uint32_t startup_table[] = { 0, 8, 16, 24, 64, 80, 96, 112, 512, 576, 640, 704, 768, 832, 896, 960 };

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)

#pragma mark ADC_Module/ACD_Pin (Sam3x)

    // Internal ADC object, and a parent of the ADCPin objects.
    // Handles: Setting options for the ADC module as a whole,
    //          and initializing the ADC module once.
    struct ADC_Module {
        static const uint32_t _default_adc_clock_frequency = 20 * 1000000; // 20MHz
        static const uint32_t _default_adc_startup_time = 12;

        static const uint32_t peripheralId() { return ID_ADC; }
        static constexpr bool is_real = true;

        const float _default_vref = 3.28;
        float _vref = _default_vref;

        static bool _inited;
        static _pinChangeInterrupt *_firstInterrupt;

        void init(const uint32_t adc_clock_frequency, const uint8_t adc_startuptime) {
            if (_inited) {
                return;
            }
            _inited = true;

            SamCommon::enablePeripheralClock(peripheralId());

            uint32_t ul_prescal, ul_startup,  ul_mr_startup, ul_real_adc_clock;
            ADC->ADC_CR = ADC_CR_SWRST;

            /* Reset Mode Register. */
            ADC->ADC_MR = 0;

            /* Reset PDC transfer. */
            ADC->ADC_PTCR = (ADC_PTCR_RXTDIS | ADC_PTCR_TXTDIS);
            ADC->ADC_RCR = 0;
            ADC->ADC_RNCR = 0;
            if (SystemCoreClock % (2 * adc_clock_frequency)) {
                // Division with reminder
                ul_prescal = SystemCoreClock / (2 * adc_clock_frequency);
            } else {
                // Whole division
                ul_prescal = SystemCoreClock / (2 * adc_clock_frequency) - 1;
            }
            ul_real_adc_clock = SystemCoreClock / (2 * (ul_prescal + 1));

            // ADC clocks needed to get ul_startuptime uS
            ul_startup = (ul_real_adc_clock / 1000000) * adc_startuptime;

            // Find correct MR_STARTUP value from conversion table
            for (ul_mr_startup=0; ul_mr_startup<16; ul_mr_startup++) {
                if (startup_table[ul_mr_startup] >= ul_startup)
                    break;
            }
            if (ul_mr_startup==16)
                return /*-1*/;

            ADC->ADC_MR |=
                ADC_MR_PRESCAL(ul_prescal) |
                ((ul_mr_startup << ADC_MR_STARTUP_Pos) & ADC_MR_STARTUP_Msk);

            ADC->ADC_ISR &= ADC_ISR_DRDY;
        };

        ADC_Module() {
            init(_default_adc_clock_frequency, _default_adc_startup_time);
        };

        static void startSampling() {
            ADC->ADC_CR = ADC_CR_START; /* start the sample */;
        };

        static void startFreeRunning() {
            ADC->ADC_MR |= ADC_MR_FREERUN_ON;
        };

        void initPin(const uint32_t adcNumber, bool differential) {
            // NOTE: differential is ignored
            const uint32_t adcMask = 1<<adcNumber;

            /* Enable the pin */
            ADC->ADC_CHER = adcMask;
            /* Enable the conversion TAG */
            ADC->ADC_EMR = ADC_EMR_TAG ;
        };
        int32_t getRawPin(const uint32_t adcNumber) {
            return ADC->ADC_CDR[adcNumber];
        };
        int32_t getValuePin(const uint32_t adcNumber) {
            const uint32_t adcMask = 1<<adcNumber;
            if ((ADC->ADC_CHSR & adcMask) != adcMask) {
                ADC->ADC_CR = ADC_CR_START; /* start the sample */
                while ((ADC->ADC_ISR & ADC_ISR_DRDY) != ADC_ISR_DRDY) {;} /* Wait... */
            }
            return getRawPin(adcNumber);
        };
        int32_t getBottomPin(const uint32_t adcNumber) {
            return 0;
        };
        float getBottomVoltagePin(const uint32_t adcNumber) {
            return 0.0;
        }
        int32_t getTopPin(const uint32_t adcNumber) {
            return 4095;
        };
        float getTopVoltagePin(const uint32_t adcNumber) {
            return _vref;
        }
        void setVoltageRangePin(const uint32_t adcNumber,
                                const float vref,
                                const float min_expected,
                                const float max_expected,
                                const float ideal_steps)
        {
            _vref = vref; // all pins on the same module share the same vref
            // ignore min_expected and max_expected for now
        };

        void setInterrupts(const uint32_t interrupts, const uint32_t adcMask) {
            if (interrupts != kPinInterruptsOff) {
                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHighest) {
                        NVIC_SetPriority(ADC_IRQn, 0);
                    }
                    else if (interrupts & kPinInterruptPriorityHigh) {
                        NVIC_SetPriority(ADC_IRQn, 1);
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        NVIC_SetPriority(ADC_IRQn, 2);
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        NVIC_SetPriority(ADC_IRQn, 3);
                    }
                    else if (interrupts & kPinInterruptPriorityLowest) {
                        NVIC_SetPriority(ADC_IRQn, 4);
                    }
                }
                /* Enable the IRQ */
                NVIC_EnableIRQ(ADC_IRQn);
                /* Enable the interrupt */
                ADC->ADC_IER = adcMask;
                /* Enable the pin */
                ADC->ADC_CHER = adcMask;
            } else {
                /* Disable the pin */
                ADC->ADC_CHDR = adcMask;
                /* Disable the interrupt */
                ADC->ADC_IDR = adcMask;
                /* Disable the interrupt - if all channels are disabled */
                if (ADC->ADC_CHSR == 0) {
                    NVIC_DisableIRQ(ADC_IRQn);
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

    #define _MAKE_MOTATE_ADC_PIN(registerChar, registerPin, adcNum) \
        template<> \
        struct ADCPinParent< ReversePinLookup<registerChar, registerPin>::number > : ADC_Module { \
            static constexpr uint32_t adcMask = 1 << adcNum; \
            static constexpr uint32_t adcNumber = adcNum; \
        };

#else // not Sam3x

#pragma mark ADC_Module/ACD_Pin (SamS70)
    /**************************************************
     *
     * PIN CHANGE INTERRUPT SUPPORT: ADC_Module/ACD_Pin (SamS70)
     *
     **************************************************/

    // Internal ADC object, and a parent of the ADCPin objects.
    // Handles: Setting options for the ADC module as a whole,
    //          and initializing the ADC module once.
    template <int32_t afecNum>
    struct ADC_Module {
        static const uint32_t moduleNumber = afecNum;

        const uint32_t _default_adc_clock_frequency = 10*1000000; // 10MHz
        const uint32_t _default_adc_startup_time = 10;

        // these are highly tuned parameters, and may need to become adjustable at runtime in the future
        // see http://www.atmel.com/Images/Atmel-44093-AFE-Calibration-on-SAM-V7-SAM-E7-SAM-S7-Microcontrollers_Application-Note.pdf
        //    specifically section 2.2.2
//        const float _default_gain = 0.991073741551146;
//        const float _default_gain = 0.9903;
        const float _default_gain = 1.01;
//        const float _default_gain = 1;

//        const float _default_offset = 106;
        const float _default_offset = 11;
//        const float _default_offset = 0;
        const float _default_vref = 3.29;

        // each pin holds a copy of this object, so these a per-pin inherently
        int32_t _top_value = 4095;
        float   _pin_offset = 0;
        int32_t _pin_scale = 1;
        bool    _differential = false;
        float  _vref = _default_vref;

        constexpr Afec* const afec() const { return afecNum == 0 ? AFEC0 : AFEC1; };
        constexpr IRQn const afecIRQnum() const { return afecNum == 0 ? AFEC0_IRQn : AFEC1_IRQn; };
        constexpr uint32_t peripheralId() const { return afecNum == 0 ? ID_AFEC0 : ID_AFEC1; };
        static constexpr bool is_real = true;

        static bool _inited;
        static _pinChangeInterrupt *_firstInterrupt;

        void init(const uint32_t adc_clock_frequency, const uint8_t adc_startuptime) {
            if (_inited) {
                return;
            }
            _inited = true;

            SamCommon::enablePeripheralClock(peripheralId());

            uint32_t ul_prescal, ul_startup,  ul_mr_startup, ul_real_adc_clock;
            afec()->AFEC_CR = AFEC_CR_SWRST;

            /* Reset Mode Register. */
            afec()->AFEC_MR = 0;

            ul_prescal = (SamCommon::getPeripheralClockFreq() / adc_clock_frequency) - 1;
            ul_real_adc_clock = SamCommon::getPeripheralClockFreq() / (ul_prescal + 1);

            // ADC clocks needed to get ul_startuptime uS
            ul_startup = (ul_real_adc_clock / 1000000) * adc_startuptime;

            // Find correct MR_STARTUP value from conversion table
            for (ul_mr_startup=0; ul_mr_startup<16; ul_mr_startup++) {
                if (startup_table[ul_mr_startup] >= ul_startup) {
                    break;
                }
            }
            if (ul_mr_startup==16) {
                return /*-1*/;
            }

            afec()->AFEC_MR =
                AFEC_MR_PRESCAL(ul_prescal) |
                AFEC_MR_STARTUP(ul_mr_startup) |
                AFEC_MR_TRACKTIM(0) |
                AFEC_MR_TRANSFER(2) |
                AFEC_MR_ONE            // becuase this bit must be one, apparently
            ;
            afec()->AFEC_EMR =
                AFEC_EMR_TAG | // enable the conversion TAG
                AFEC_EMR_STM   // enable Single Trigger Mode
            ;

            afec()->AFEC_ACR =
                AFEC_ACR_IBCTL(1) |
                AFEC_ACR_PGA0EN |
                AFEC_ACR_PGA1EN
            ;

            // reset error correction list
            afec()->AFEC_CECR = 0;
            // enable it generally, however
            afec()->AFEC_COSR = AFEC_COSR_CSEL;

            setupErrorCorrection(_default_gain, _default_offset, _default_vref);
        };

        ADC_Module() {
            init(_default_adc_clock_frequency, _default_adc_startup_time);
        };

        void startSampling() {
            afec()->AFEC_CR = AFEC_CR_START; /* start the sample */;
        };

        static void startFreeRunning() {
            afec()->AFEC_MR |= AFEC_MR_FREERUN_ON;
        };

        void setupErrorCorrection(const float gain, const float offset, const float vref) {
            // Using two measurements: V1 the actual voltage, with V1_a being the actual measurement (no correction) and
            //  V1_i being the ideal and a second measurement (preferably near the other end of the range) V2, V2_a, and
            //  V2_i, where Vn_i = 2^12*(Vn/VREF). Arrange it so that V1 is a higher voltage than V2.
            // To get the gain:     gain = (V1_a-V2_a)/(V1_i-V2_i)
            // To get the offset: offset = (V1_a-2^11)-gain*(V1_i-2^11)

            afec()->AFEC_CVR = AFEC_CVR_OFFSETCORR((int16_t)-offset) | AFEC_CVR_GAINCORR((int16_t)(32768.0/gain));
        };

        void initPin(const uint32_t adcNumber, bool differential) {
            const uint32_t adcMask = 1<<adcNumber;

            // Note about differential pairs: they are sequential, starting from 0
            // 0 and 1, 2 and 3, etc.
            // So, the other one from any adcNumber is adcNumber XOR 1, or adcNumber^1

            // Enable the pin
            afec()->AFEC_CHER = adcMask;

            // Enable error correction - note the actual correction parameters are shared across this module
            // and are set in setupErrorCorrection
            afec()->AFEC_CECR |= adcMask;

            // The offset (AOFF) is a 10-bit value that will set where in the range is the mid-point.
            // The startup value is 0, and the ideal midpoint is 2^10 / 2=512.
            // See 54.8.4.2 of the SamS70 Manual (19-Jan-16 ed):
            //    C_i = (4096/V_refp)*(V_in-V_DAC)*gain+2047
            // The VDAC uses the AOFF value as an "input". That's vague.
            // For more info, see:
            // http://www.atmel.com/Images/Atmel-44093-AFE-Calibration-on-SAM-V7-SAM-E7-SAM-S7-Microcontrollers_Application-Note.pdf
            afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber);
            afec()->AFEC_COCR = AFEC_COCR_AOFF(512);

            _differential = differential;
        };
        int32_t getRawPin(const uint32_t adcNumber) {
            afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber);
            return (int32_t)afec()->AFEC_CDR; // cast to get negative value correct
        };
        int32_t getValuePin(const uint32_t adcNumber) {
            const uint32_t adcMask = 1<<adcNumber;
            if ((afec()->AFEC_CHSR & adcMask) != adcMask) {
                afec()->AFEC_CR = AFEC_CR_START; /* start the sample */
                while ((afec()->AFEC_CHSR & adcMask) != adcMask) {;} /* Wait... */
            }
            return getRawPin(adcNumber);
        };
        int32_t getBottomPin(const uint32_t adcNumber) {
            return 0;
        };
        float getBottomVoltagePin(const uint32_t adcNumber) {
            if (afec()->AFEC_DIFFR & (1<<adcNumber)) {
                return 0.0; // differential values both have the offset, so it cancels out
            }
            return _pin_offset * _vref;
        }
        int32_t getTopPin(const uint32_t adcNumber) {
            if (afec()->AFEC_DIFFR & (1<<adcNumber)) {
                // differential values are centered and the offset cancels out
                return (_top_value * _pin_scale)/2.0;
            }
            return (_pin_offset + 1.0) * _top_value * _pin_scale;
        };
        float getTopVoltagePin(const uint32_t adcNumber) {
            return _vref;
        }
        void setVoltageRangePin(const uint32_t adcNumber,
                                const float vref,
                                const float min_expected,
                                const float max_expected,
                                const float ideal_steps)
        {
            _vref = vref; // all pins on the same module share the same vref
            // ignore min_expected and max_expected for now, except to set the bits
            // we have as optins 12, 13, 14, 15, and 16bit, each costs more in sample time

            float min_lsb = (vref/(max_expected-min_expected))*ideal_steps;
            if (min_lsb > (_differential ? 2047 : 4095)) { // 2^12 = 4096
                if (min_lsb > (_differential ? 4095 : 8191)) { // 2^13 = 8192
                    if (min_lsb > (_differential ? 8191 : 16383)) { // 2^14 = 16384
                        if (min_lsb > 32767) { // 2^15 = 32,768
                            // 2^16 = 65,536
                            // use 16 bit
                            afec()->AFEC_EMR = (afec()->AFEC_EMR & ~AFEC_EMR_RES_Msk) | AFEC_EMR_RES(5);
                            _top_value = 65535;
                        } else {
                            // use 15 bit
                            afec()->AFEC_EMR = (afec()->AFEC_EMR & ~AFEC_EMR_RES_Msk) | AFEC_EMR_RES(4);
                            _top_value = 32767;
                        }
                    } else {
                        // use 14 bit
                        afec()->AFEC_EMR = (afec()->AFEC_EMR & ~AFEC_EMR_RES_Msk) | AFEC_EMR_RES(3);
                        _top_value = 16383;
                    }
                } else {
                    // use 13 bit
                    afec()->AFEC_EMR = (afec()->AFEC_EMR & ~AFEC_EMR_RES_Msk) | AFEC_EMR_RES(2);
                    _top_value = 8191;
                }
            } else {
                // use 12 bit
                afec()->AFEC_EMR = (afec()->AFEC_EMR & ~AFEC_EMR_RES_Msk) | AFEC_EMR_RES(0);
                _top_value = 4095;
            }

            if (_differential) {
                // make sure any differential pins are enabled as well
                afec()->AFEC_CHER = 1<<(adcNumber^1);
                afec()->AFEC_CECR |= (1 << adcNumber) | (1 << (adcNumber^1));
                afec()->AFEC_DIFFR |= (1 << adcNumber) | (1 << (adcNumber^1));
            }

            // handle scale, which is per-pin
            // also handles offset, which is the center of the range of size (VREF/scale)
            // and it cannot offset more that 1/(scale*2) from zero
            float scale_used = (max_expected-(_differential?0:min_expected))/vref;
            if (scale_used < 0.25) { // 4x scale
                afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber);
                afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << (adcNumber * 2))) |
                                    (2 << (adcNumber * 2)
                                   );

                int32_t offset = (1024 * min_expected)/vref + 128;
                offset = std::min(offset, 1024L);
                afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                _pin_offset = ((float)(offset-128)/1024.0);

                if (_differential) {
                    afec()->AFEC_CSELR = AFEC_CSELR_CSEL((adcNumber^1));
                    afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << ((adcNumber^1) * 2))) |
                                        (2 << ((adcNumber^1) * 2)
                                       );
                    afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                }
                _pin_scale = 4;
            }
            else if (scale_used < 0.5) { // 2x scale
                afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber);
                afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << (adcNumber * 2))) |
                                    (1 << (adcNumber * 2)
                                   );

                int32_t offset = (1024 * min_expected)/vref + 256;
                offset = std::min(offset, 1024L);
                afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                _pin_offset = ((float)(offset-256)/1024.0);

                if (_differential) {
                    afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber^1);
                    afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << ((adcNumber^1) * 2))) |
                                        (1 << ((adcNumber^1) * 2)
                                       );
                    afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                }

                _pin_scale = 2;
            }
            else { // 1x scale
                afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber);
                afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << (adcNumber * 2))) |
                                    (0 << (adcNumber * 2));

                int32_t offset = (1024 * min_expected)/vref + 512;
                offset = std::min(offset, 1024L);
                afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                _pin_offset = ((float)(offset-512)/1024.0);

                if (_differential) {
                    afec()->AFEC_CSELR = AFEC_CSELR_CSEL(adcNumber^1);
                    afec()->AFEC_CGR = (afec()->AFEC_CGR & ~(0x3 << ((adcNumber^1) * 2))) |
                                        (0 << ((adcNumber^1) * 2)
                                       );
                    afec()->AFEC_COCR = AFEC_COCR_AOFF(offset);
                }

                _pin_scale = 1;
            }
        };

        void setInterrupts(const uint32_t interrupts, const uint32_t adcMask) {
            if (interrupts != kPinInterruptsOff) {
                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHighest) {
                        NVIC_SetPriority(afecIRQnum(), 0);
                    }
                    else if (interrupts & kPinInterruptPriorityHigh) {
                        NVIC_SetPriority(afecIRQnum(), 3);
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        NVIC_SetPriority(afecIRQnum(), 7);
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        NVIC_SetPriority(afecIRQnum(), 11);
                    }
                    else if (interrupts & kPinInterruptPriorityLowest) {
                        NVIC_SetPriority(afecIRQnum(), 15);
                    }
                }
                /* Enable the IRQ */
                NVIC_EnableIRQ(afecIRQnum());
                /* Enable the interrupt */
                afec()->AFEC_IER = adcMask;
                /* Enable the pin */
                afec()->AFEC_CHER = adcMask;
            } else {
                /* Disable the pin */
                afec()->AFEC_CHDR = adcMask;
                /* Disable the interrupt */
                afec()->AFEC_IDR = adcMask;
                /* Disable the interrupt - if all channels are disabled */
                if (afec()->AFEC_CHSR == 0) {
                    NVIC_DisableIRQ(afecIRQnum());
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

    #define _MAKE_MOTATE_ADC_PIN(registerChar, registerPin, afecNum, adcNum) \
        template<> \
        struct ADCPinParent< ReversePinLookup<registerChar, registerPin>::number > : ADC_Module<afecNum> { \
            static constexpr uint32_t adcMask = 1 << adcNum; \
            static constexpr uint32_t adcNumber = adcNum; \
        };

#endif

//    template<pin_number negPinNum, pin_number posPinNum>
//    constexpr const bool IsADCDifferentialPair() {
//        return (ADCPin<adcNegNum>::moduleNumber == ADCPin<adcPosNum>::moduleNumber) &&
//        (ADCPin<adcNegNum>::adcNumber == (ADCPin<adcPosNum>::adcNumber^1));
//    };


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
     * SPI PIN METADATA and wiring: specializes SPIChipSelectPin / SPIMISOPin / SPIMOSIPin / SPISCKPin
     *
     * Provides: _MAKE_MOTATE_SPI_CS_PIN
     *           _MAKE_MOTATE_SPI_MISO_PIN
     *           _MAKE_MOTATE_SPI_MOSI_PIN
     *           _MAKE_MOTATE_SPI_SCK_PIN
     *
     **************************************************/


    #define _MAKE_MOTATE_SPI_CS_PIN(registerChar, registerPin, spiNumber, peripheralAorB, csNum) \
        template<> \
        struct SPIChipSelectPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> { \
            SPIChipSelectPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {}; \
            static constexpr bool is_real = true; \
            static constexpr uint8_t spiNum = spiNumber; \
            static constexpr uint8_t csNumber =  csNum; \
            static constexpr uint8_t csValue  = ~(1<<csNum); \
            static constexpr bool usesDecoder = false; \
        };

    #define _MAKE_MOTATE_SPI_MISO_PIN(registerChar, registerPin, spiNumber, peripheralAorB)\
        template<>\
        struct SPIMISOPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            SPIMISOPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral ## peripheralAorB} {};\
            static constexpr bool is_real = true; \
            static constexpr uint8_t spiNum = spiNumber; \
        };


    #define _MAKE_MOTATE_SPI_MOSI_PIN(registerChar, registerPin, spiNumber, peripheralAorB)\
        template<>\
        struct SPIMOSIPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            SPIMOSIPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral ## peripheralAorB} {};\
            static constexpr bool is_real = true; \
            static constexpr uint8_t spiNum = spiNumber; \
        };


    #define _MAKE_MOTATE_SPI_SCK_PIN(registerChar, registerPin, spiNumber, peripheralAorB)\
        template<>\
        struct SPISCKPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            SPISCKPin() : ReversePinLookup<registerChar, registerPin> { kPeripheral ## peripheralAorB } {};\
            static constexpr bool is_real = true; \
            static constexpr uint8_t spiNum = spiNumber; \
        };


#pragma mark TWI Pins support
    /**************************************************
     *
     * TWI PIN METADATA and wiring: specializes TWISCKPin / TWISDAPin
     *
     * Provides: _MAKE_MOTATE_TWI_SCK_PIN
     *           _MAKE_MOTATE_TWI_SDA_PIN
     *
     **************************************************/


    #define _MAKE_MOTATE_TWI_SCK_PIN(registerChar, registerPin, twiNumber, peripheralAorB)\
        template<>\
        struct TWISCKPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            TWISCKPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral ## peripheralAorB} {};\
            static constexpr bool is_real = true; \
            static constexpr uint8_t twiNum = twiNumber; \
        };


    #define _MAKE_MOTATE_TWI_SDA_PIN(registerChar, registerPin, twiNumber, peripheralAorB)\
        template<>\
        struct TWISDAPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            TWISDAPin() : ReversePinLookup<registerChar, registerPin>{kPeripheral ## peripheralAorB} {};\
            static constexpr bool is_real = true; \
            static constexpr uint8_t twiNum = twiNumber; \
        };


#pragma mark UART / USART Pin support
    /**************************************************
     *
     * UART/USART PIN METADATA and wiring: specializes UARTTxPin / UARTRxPin / UARTRTSPin / UARTCTSPin
     *
     * Provides: _MAKE_MOTATE_UART_TX_PIN
     *           _MAKE_MOTATE_UART_RX_PIN
     *           _MAKE_MOTATE_UART_RTS_PIN
     *           _MAKE_MOTATE_UART_CTS_PIN
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

#pragma mark ClockOutputPin
    /**************************************************
     *
     * Clock Output PIN METADATA and wiring: CLKOutPin
     *
     * Provides: _MAKE_MOTATE_CLOCK_OUTPUT_PIN
     *
     **************************************************/

    #define _MAKE_MOTATE_CLOCK_OUTPUT_PIN(registerChar, registerPin, clockNumber, peripheralAorB)\
        template<>\
        struct ClockOutputPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            ClockOutputPin(const uint32_t target_freq) : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {\
                uint32_t prescaler = PMC_PCK_PRES_CLK_1;\
                if ((SystemCoreClock >> 1) < target_freq) { prescaler = PMC_PCK_PRES_CLK_2; }\
                if ((SystemCoreClock >> 2) < target_freq) { prescaler = PMC_PCK_PRES_CLK_4; }\
                if ((SystemCoreClock >> 3) < target_freq) { prescaler = PMC_PCK_PRES_CLK_8; }\
                if ((SystemCoreClock >> 4) < target_freq) { prescaler = PMC_PCK_PRES_CLK_16; }\
                if ((SystemCoreClock >> 5) < target_freq) { prescaler = PMC_PCK_PRES_CLK_32; }\
                if ((SystemCoreClock >> 6) < target_freq) { prescaler = PMC_PCK_PRES_CLK_64; }\
                PMC->PMC_PCK[clockNumber] = PMC_PCK_CSS_MCK | prescaler;\
            };\
            static const bool is_real = true;\
            void operator=(const bool value); /*Will cause a failure if used.*/\
        };

} // end namespace Motate

#endif /* end of include guard: SAMPINS_H_ONCE */
