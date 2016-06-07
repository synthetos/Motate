/*
 utility/SamPins.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2016 Robert Giseburt

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

#include <type_traits>

namespace Motate {
    // Numbering is arbitrary:
    enum PinMode {
        kUnchanged      = 0,
        kOutput         = 1,
        kInput          = 2,
        // These next two are NOT available on other platforms,
        // but cannot be masked out since they are required for
        // special pin functions. These should not be used in
        // end-user (sketch) code.
        kPeripheralA    = 3,
        kPeripheralB    = 4,
    };

    // Numbering is arbitrary, but bit unique for bitwise operations (unlike other architectures):
    enum PinOptions {
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
    };
    typedef uint16_t PinOptions_n;

    enum PinInterruptOptions {
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

    typedef uint32_t uintPort_t;

    typedef const int16_t pin_number;

    template <unsigned char portLetter>
    struct Port32Parent {};

    template <unsigned char portLetter>
    struct Port32 : Port32Parent<portLetter> {
        using Port32Parent<portLetter>::port;
        using Port32Parent<portLetter>::_IRQn;
        using Port32Parent<portLetter>::disablePeripheralClock;
        using Port32Parent<portLetter>::enablePeripheralClock;
        void setModes(const uintPort_t value, const uintPort_t mask) {
            port->PIO_ODR = ~value & mask ;
            port->PIO_OER = value & mask ;
            port->PIO_PER = mask ;
            /* if all pins are output, disable PIO Controller clocking, reduce power consumption */
            if ( port->PIO_OSR == 0xffffffff )
            {
                disablePeripheralClock();
            } else {
                enablePeripheralClock();
            }
        };
        void setOptions(const PinOptions_n options, const uintPort_t mask) {
            if (kPullUp & options)
            {
                port->PIO_PUER = mask ;
            }
            else
            {
                port->PIO_PUDR = mask ;
            }
            if (kWiredAnd & options)
            {/*kDriveLowOnly - Enable Multidrive*/
                port->PIO_MDER = mask ;
            }
            else
            {
                port->PIO_MDDR = mask ;
            }
            if (kDeglitch & options)
            {
                port->PIO_IFER = mask ;
                port->PIO_SCIFSR = mask ;
            }
            else
            {
                if (kDebounce & options)
                {
                    port->PIO_IFER = mask ;
                    port->PIO_DIFSR = mask ;
                }
                else
                {
                    port->PIO_IFDR = mask ;
                }
            }
        };
        void set(const uintPort_t value) {
            port->PIO_SODR = value;
        };
        void clear(const uintPort_t value) {
            port->PIO_CODR = value;
        };
        void write(const uintPort_t value) {
            port->PIO_OWER = 0xffffffff;/*Enable all registers for writing thru ODSR*/
            port->PIO_ODSR = value;
            port->PIO_OWDR = 0xffffffff;/*Disable all registers for writing thru ODSR*/
        };
        void write(const uintPort_t value, const uintPort_t mask) {
            port->PIO_OWER = mask;/*Enable masked registers for writing thru ODSR*/
            port->PIO_ODSR = value;
            port->PIO_OWDR = mask;/*Disable masked registers for writing thru ODSR*/
        };
        uintPort_t getInputValues(const uintPort_t mask) {
            return port->PIO_PDSR & mask;
        };
        uintPort_t getOutputValues(const uintPort_t mask) {
            return port->PIO_ODSR & mask;
        };
        Pio* portPtr() {
            return &port;
        };
        void setInterrupts(const uint32_t interrupts, const uintPort_t mask) {
            if (interrupts != kPinInterruptsOff) {
                port->PIO_IDR = mask;

                /*Is it an "advanced" interrupt?*/
                if (interrupts & kPinInterruptAdvancedMask) {
                    port->PIO_AIMER = mask;
                    /*Is it an edge interrupt?*/
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnFallingEdge) {
                        port->PIO_ESR = mask;
                    }
                    else
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel ||
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnLowLevel) {
                        port->PIO_LSR = mask;
                    }
                    /*Rising Edge/High Level, or Falling Edge/LowLevel?*/
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel) {
                        port->PIO_REHLSR = mask;
                    }
                    else
                    {
                        port->PIO_FELLSR = mask;
                    }
                }
                else
                {
                    port->PIO_AIMDR = mask;
                }

                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHighest) {
                        NVIC_SetPriority(_IRQn, 0);
                    }
                    else if (interrupts & kPinInterruptPriorityHigh) {
                        NVIC_SetPriority(_IRQn, 3);
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        NVIC_SetPriority(_IRQn, 7);
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        NVIC_SetPriority(_IRQn, 11);
                    }
                    else if (interrupts & kPinInterruptPriorityLowest) {
                        NVIC_SetPriority(_IRQn, 15);
                    }
                }
                /* Enable the IRQ */
                NVIC_EnableIRQ(_IRQn);
                /* Enable the interrupt */
                port->PIO_IER = mask;
            } else {
                port->PIO_IDR = mask;
                if (port->PIO_ISR == 0)
                    NVIC_DisableIRQ(_IRQn);
            }
        };
    };

#define _MAKE_MOTATE_PORT32(registerLetter, registerChar)\
    template <> \
    struct Port32Parent<registerChar> : SamCommon< Port32<registerChar> > {\
        static const uint8_t letter = (uint8_t) registerChar; \
        static constexpr auto port = (PIO ## registerLetter); \
        static const uint32_t peripheralId() { \
            return ID_PIO ## registerLetter; \
        }; \
        static constexpr auto _IRQn = PIO ## registerLetter ## _IRQn; \
        using SamCommon< Port32<registerChar> >::disablePeripheralClock; \
        using SamCommon< Port32<registerChar> >::enablePeripheralClock; \
    }

    template<pin_number n>
    struct PinParent {
        static const int16_t number = -1;
        static const uint8_t portLetter = 0;
        static const uint32_t mask = 0;
    };

    template<pin_number pinNum>
    struct Pin : PinParent<pinNum> {
        private: /* Lock the copy contructor.*/
        Pin(const Pin<pinNum>&){};
        using PinParent<pinNum>::port;
        using PinParent<pinNum>::mask;
        using PinParent<pinNum>::enablePeripheralClock;
        using PinParent<pinNum>::disablePeripheralClock;

        public:

        Pin() {};
        Pin(const PinMode type, const PinOptions_n options = kNormal) {
            init(type, options, /*fromConstructor=*/true);
        };
        void operator=(const bool value) { write(value); };
        operator bool() { return (get() != 0); };

        void init(const PinMode type, const PinOptions_n options = kNormal, const bool fromConstructor=false) {
            setMode(type, fromConstructor);
            setOptions(options, fromConstructor);
        };
        void setMode(const PinMode type, const bool fromConstructor=false) {
            switch (type) {
                case kOutput:
                    port->PIO_OER = mask ;
                    port->PIO_PER = mask ;
                    /* if all pins are output, disable PIO Controller clocking, reduce power consumption */
                    if (!fromConstructor) {
                        if ( port->PIO_OSR == 0xffffffff )
                        {
                            disablePeripheralClock();
                        }
                    }
                break;
                case kInput:
                    enablePeripheralClock();
                    port->PIO_ODR = mask ;
                    port->PIO_PER = mask ;
                    break;
                case kPeripheralA:
                    port->PIO_ABSR &= ~mask ;
                    port->PIO_PDR = mask ;
                    break;
                case kPeripheralB:
                    port->PIO_ABSR |= mask ;
                    port->PIO_PDR = mask ;
                    break;
                default:
                    break;
            }
        };
        PinMode getMode() {
            return (port->PIO_OSR & mask) ? kOutput : kInput;
        };
        void setOptions(const PinOptions_n options, const bool fromConstructor=false) {
            if (kStartHigh & options)
            {
                port->PIO_SODR = mask ;
            } else if (kStartLow & options)
            {
                port->PIO_CODR = mask ;
            }
            if (kPullUp & options)
            {
                port->PIO_PUER = mask ;
            }
            else
            {
                port->PIO_PUDR = mask ;
            }
            if (kWiredAnd & options)
            {/*kDriveLowOnly - Enable Multidrive*/
                port->PIO_MDER = mask ;
            }
            else
            {
                port->PIO_MDDR = mask ;
            }
            if (kDeglitch & options)
            {
                port->PIO_IFER = mask ;
                port->PIO_SCIFSR = mask ;
            }
            else
            {
                if (kDebounce & options)
                {
                    port->PIO_IFER = mask ;
                    port->PIO_DIFSR = mask ;
                }
                else
                {
                    port->PIO_IFDR = mask ;
                }
            }
        };
        PinOptions_n getOptions() {
            return ((port->PIO_PUSR & mask) ? kPullUp : 0)
            | ((port->PIO_MDSR & mask) ? kWiredAnd : 0)
            | ((port->PIO_IFSR & mask) ?
            ((port->PIO_IFDGSR & mask) ? kDebounce : kDeglitch) : 0);
        };
        void set() {
            port->PIO_SODR = mask;
        };
        void clear() {
            port->PIO_CODR = mask;
        };
        void write(const bool value) {
            if (!value)
                clear();
            else
                set();
        };
        void toggle()  {
            port->PIO_OWER = mask; /*Enable writing thru ODSR*/
            port->PIO_ODSR ^= mask;
        };
        uint32_t get() { /* WARNING: This will fail if the peripheral clock is disabled for this pin!!! Use getOutputValue() instead. */
            return port->PIO_PDSR & mask;
        };
        uint32_t getInputValue() {
            return port->PIO_PDSR & mask;
        };
        uint32_t getOutputValue() {
            return port->PIO_ODSR & mask;
        };
        void setInterrupts(const uint32_t interrupts) {
            setInterrupts(interrupts, mask);
        };
        bool isNull() { return false; };
//        static uint32_t maskForPort(const uint8_t otherPortLetter) {
//            return portLetter == otherPortLetter ? mask : 0x00u;
//        };
    };


    // Specialization for Pin<-1>
    template<>
    struct Pin<-1> {
        static const int8_t number = -1;
        static const uint8_t portLetter = 0;
        static const uint32_t mask = 0;

        Pin() {};
        Pin(const PinMode type, const PinOptions_n options = kNormal) {};
        void operator=(const bool value) {};
        operator bool() { return 0; };

        void init(const PinMode type, const PinOptions_n options = kNormal, const bool fromConstructor=false) {};
        void setMode(const PinMode type, const bool fromConstructor=false) {};
        PinMode getMode() { return kUnchanged; };
        void setOptions(const PinOptions_n options, const bool fromConstructor=false) {};
        PinOptions_n getOptions() { return kNormal; };
        void set() {};
        void clear() {};
        void write(const bool value) {};
        void toggle() {};
        uintPort_t get() { return 0; };
        uintPort_t getInputValue() { return 0; };
        uintPort_t getOutputValue() { return 0; };
        void setInterrupts(const uint32_t interrupts) {};
        static uintPort_t maskForPort(const uint8_t otherPortLetter) { return 0; };
        bool isNull() { return true; };
    };

    template<uint8_t portChar, uint8_t portPin>
    struct ReversePinLookup : Pin<-1> {
        ReversePinLookup() {};
        ReversePinLookup(const PinMode type, const PinOptions_n options = kNormal) : Pin<-1>(type, options) {};
    };


    #define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerChar, registerPin) \
        template<> \
        struct PinParent<pinNum> : Port32<registerChar> { \
                static const int16_t number = pinNum; \
                static const uint8_t portLetter = (uint8_t) registerChar; \
                static const uint32_t mask = (1u << registerPin); \
        }; \
        template<> \
        struct ReversePinLookup<registerChar, registerPin> : Pin<pinNum> { \
            ReversePinLookup() {}; \
            ReversePinLookup(const PinMode type, const PinOptions_n options = kNormal) : Pin<pinNum>(type, options) {}; \
        }; \
        template<> void Motate::IRQPin<pinNum>::interrupt();

    template<int16_t pinNum>
    struct InputPin : Pin<pinNum> {
        InputPin() : Pin<pinNum>(kInput) {};
        InputPin(const PinOptions_n options) : Pin<pinNum>(kInput, options) {};
        void init(const PinOptions_n options = kNormal  ) {Pin<pinNum>::init(kInput, options);};
        uint32_t get() {
            return Pin<pinNum>::getInputValue();
        };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. These are intentionally not defined. */
        void init(const PinMode type, const PinOptions_n options = kNormal);
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        void write(const bool);
    };

    template<int16_t pinNum>
    struct OutputPin : Pin<pinNum> {
        OutputPin() : Pin<pinNum>(kOutput) {};
        OutputPin(const PinOptions_n options) : Pin<pinNum>(kOutput, options) {};
        void init(const PinOptions_n options = kNormal) {Pin<pinNum>::init(kOutput, options);};
        uint32_t get() {
            return Pin<pinNum>::getOutputValue();
        };
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. */
        void init(const PinMode type, const PinOptions_n options = kNormal); /* Intentially not defined. */
    };

    // All of the pins on the SAM can be an interrupt pin
    // but we create these objects to share the interface with other architectures.
    template<int16_t pinNum>
    struct IRQPin : Pin<pinNum> {
        IRQPin() : Pin<pinNum>(kInput) {};
        IRQPin(const PinOptions_n options) : Pin<pinNum>(kInput, options) {};
        void init(const PinOptions_n options = kNormal  ) {Pin<pinNum>::init(kInput, options);};

        static const bool is_real = true; // Yeah, they are ALL interrupt pins.
        static void interrupt() __attribute__ (( weak ));
    };

    template<int16_t pinNum>
    constexpr const bool IsIRQPin() { return IRQPin<pinNum>::is_real; }; // Basically return if we have a valid pin.

    template<pin_number gpioPinNumber>
    using IsGPIOIRQOrNull = typename std::enable_if<true>::type;

    template<uint8_t portChar, uint8_t portPin>
    using LookupIRQPin = IRQPin< ReversePinLookup<portChar, portPin>::number >;


    struct _pinChangeInterrupt {
        const uint8_t portLetter;
        const uint32_t mask;
        void (&interrupt)();
    };

    // YAY! We get to have fun with macro concatenation!
    // See: https://gcc.gnu.org/onlinedocs/cpp/Stringification.html#Stringification
    // Short form: We need to take two passes to get the concatenation to work
    #define MOTATE_PIN_INTERRUPT_NAME_( x, y ) x##y
    #define MOTATE_PIN_INTERRUPT_NAME( x, y )\
        MOTATE_PIN_INTERRUPT_NAME_( x, y )

    // Also we use the GCC-specific __COUNTER__
    // See https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
#define MOTATE_PIN_INTERRUPT(number) \
    Motate::_pinChangeInterrupt MOTATE_PIN_INTERRUPT_NAME( _Motate_PinChange_Interrupt_Trampoline, __COUNTER__ )\
            __attribute__(( used,section(".motate.pin_change_interrupts") )) {\
        Motate::IRQPin<number>::portLetter,\
        Motate::IRQPin<number>::mask,\
        Motate::IRQPin<number>::interrupt\
    };\
    template<> void Motate::IRQPin<number>::interrupt()

    constexpr uint32_t startup_table[] = { 0, 8, 16, 24, 64, 80, 96, 112, 512, 576, 640, 704, 768, 832, 896, 960 };

    // Internal ADC object, and a parent of the ADCPin objects.
    // Handles: Setting options for the ADC module as a whole,
    //          and initializing the ADC module once.
    struct ADC_Module : SamCommon< ADC_Module > {
        static const uint32_t default_adc_clock_frequency = 20000000;
        static const uint32_t default_adc_startup_time = 12;
        static const uint32_t peripheralId() { return ID_ADC; }

        static bool inited_;

        void init(const uint32_t adc_clock_frequency, const uint8_t adc_startuptime) {
            if (inited_) {
                return;
            }
            inited_ = true;

            enablePeripheralClock();

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
            init(default_adc_clock_frequency, default_adc_startup_time);
        };

        static void startSampling() {
            ADC->ADC_CR = ADC_CR_START; /* start the sample */;
        };

        static void startFreeRunning() {
            ADC->ADC_MR |= ADC_MR_FREERUN_ON;
        };
    };

    template<pin_number pinNum>
    struct ADCPinParent {
        static const uint32_t adcMask = 0;
        static const uint32_t adcNumber = 0;
        static const uint16_t getTop() { return 4095; };
    };

    // Some pins are ADC pins.
    // template<pin_number n>
    // struct ADCPin : Pin<-1> {
    //     ADCPin() : Pin<-1>() {};
    //     ADCPin(const PinOptions_n options) : Pin<-1>() {};
    //
    //     uint32_t getRaw() {
    //         return 0;
    //     };
    //     uint32_t getValue() {
    //         return 0;
    //     };
    //     operator int16_t() {
    //         return getValue();
    //     };
    //     operator float() {
    //         return 0.0;
    //     };
    //     static const uint16_t getTop() { return 4095; };
    //
    //     static const bool is_real = false;
    //     static void interrupt() __attribute__ (( weak )); // Allow setting an interrupt on a invalid ADC pin -- will never be called
    // };


    template<pin_number pinNum>
    struct ADCPin : ADCPinParent<pinNum>, Pin<pinNum>, ADC_Module {
        using ADCPinParent<pinNum>::adcMask;
        using ADCPinParent<pinNum>::adcNumber;
        using ADCPinParent<pinNum>::getTop;

        ADCPin() : ADCPinParent<pinNum>(), Pin<pinNum>(kInput), ADC_Module() { init(); };
        ADCPin(const PinOptions_n options) : ADCPinParent<pinNum>(), Pin<pinNum>(kInput), ADC_Module() { init(); };

        void init() {
            /* Enable the pin */
            ADC->ADC_CHER = adcMask;
            /* Enable the conversion TAG */
            ADC->ADC_EMR = ADC_EMR_TAG ;
        };
        uint32_t getRaw() {
            return ADC->ADC_CDR[adcNumber];
        };
        uint32_t getValue() {
            if ((ADC->ADC_CHSR & adcMask) != adcMask) {
                ADC->ADC_CR = ADC_CR_START; /* start the sample */
                while ((ADC->ADC_ISR & ADC_ISR_DRDY) != ADC_ISR_DRDY) {;} /* Wait... */
            }
            return getRaw();
        };
        operator int16_t() {
            return getValue();
        };
        operator float() {
            return (float)getValue() / getTop();
        };
        static const bool is_real = true;
        void setInterrupts(const uint32_t interrupts) {
            if (interrupts != kPinInterruptsOff) {
                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHighest) {
                        NVIC_SetPriority(ADC_IRQn, 0);
                    }
                    else if (interrupts & kPinInterruptPriorityHigh) {
                        NVIC_SetPriority(ADC_IRQn, 3);
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        NVIC_SetPriority(ADC_IRQn, 7);
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        NVIC_SetPriority(ADC_IRQn, 11);
                    }
                    else if (interrupts & kPinInterruptPriorityLowest) {
                        NVIC_SetPriority(ADC_IRQn, 15);
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
        static void interrupt();
    };

    template<int16_t adcNum>
    struct ReverseADCPin : ADCPin<-1> {
        ReverseADCPin() : ADCPin<-1>() {};
        ReverseADCPin(const PinOptions_n options) : ADCPin<-1>() {};
    };

    #define _MAKE_MOTATE_ADC_PIN(registerChar, registerPin, adcNum) \
    template<> \
    struct ADCPinParent< ReversePinLookup<registerChar, registerPin>::number > { \
        static const uint32_t adcMask = 1 << adcNum; \
        static const uint32_t adcNumber = adcNum; \
        static const uint16_t getTop() { return 4095; }; \
    }; \
    template<> \
    struct ReverseADCPin<adcNum> : ADCPin<ReversePinLookup<registerChar, registerPin>::number> { \
        ReverseADCPin() : ADCPin<ReversePinLookup<registerChar, registerPin>::number>() {}; \
        ReverseADCPin(const PinOptions_n options) : ADCPin<ReversePinLookup<registerChar, registerPin>::number>(options) {}; \
    };

    template<int16_t pinNum>
    constexpr const bool IsADCPin() { return ADCPin<pinNum>::is_real; };

    template<uint8_t portChar, int16_t portPin>
    using LookupADCPin = ADCPin< ReversePinLookup<portChar, portPin>::number >;

    template<int16_t adcNum>
    using LookupADCPinByADC = ADCPin< ReverseADCPin< adcNum >::number >;


    template<pin_number n>
    struct PWMOutputPinParent : Pin< -1 > {
        static const pin_number pinNum = -1;
    };

    #define _MAKE_MOTATE_PWM_PIN(registerChar, registerPin, timerOrPWM, peripheralAorB, invertedByDefault) \
    template<> \
    struct PWMOutputPinParent< ReversePinLookup<registerChar, registerPin>::number > : Pin< ReversePinLookup<registerChar, registerPin>::number >, timerOrPWM { \
        static const pin_number pinNum = ReversePinLookup<registerChar, registerPin>::number; \
        typedef timerOrPWM parentTimerType; \
        PWMOutputPinParent() : Pin<pinNum>(kPeripheral ## peripheralAorB, ) {}; \
        PWMOutputPinParent(const PinOptions_n options, const uint32_t freq = kDefaultPWMFrequency) : Pin<pinNum>(kPeripheral ## peripheralAorB, , options) {}; \
        PWMOutputPinParent(const uint32_t freq) : Pin<pinNum>(kPeripheral ## peripheralAorB, , kNormal) {}; \
    }

    static const uint32_t kDefaultPWMFrequency = 1000;
    template<pin_number pinNum>
    struct PWMOutputPin : PWMOutputPinParent<pinNum> {
        PWMOutputPin() : PWMOutputPinParent<pinNum>(), timerOrPWM(Motate::kTimerUpToMatch, kDefaultPWMFrequency) { pwmpin_init(kNormal);};\
        PWMOutputPin(const PinOptions_n options, const uint32_t freq = kDefaultPWMFrequency) :\
                Pin<pinNum>(options), timerOrPWM(Motate::kTimerUpToMatch, freq)\
            {pwmpin_init(options);};\
        PWMOutputPin(const uint32_t freq) :\
                Pin<pinNum>(kNormal), timerOrPWM(Motate::kTimerUpToMatch, freq)\
            {pwmpin_init(kNormal);};\
        void pwmpin_init(const PinOptions_n options) {\
            timerOrPWM::setOutputOptions((invertedByDefault ^ ((options & kPWMPinInverted)?true:false)) ? kPWMOnInverted : kPWMOn);\
            timerOrPWM::start();\
        };\
        void setFrequency(const uint32_t freq) {\
            timerOrPWM::setModeAndFrequency(Motate::kTimerUpToMatch, freq);\
            timerOrPWM::start();\
        };\
        operator float() { return timerOrPWM::getDutyCycle(); };\
        operator uint32_t() { return (float)timerOrPWM::getExactDutyCycle(); };\
        void operator=(const float value) { write(value); };\
        void write(const float value) {\
            uint16_t duty = getTopValue() * value;\
            if (duty < 2)\
                stopPWMOutput();\
            else\
                startPWMOutput();\
            timerOrPWM::setExactDutyCycle(duty);\
        };\
        void writeRaw(const uint16_t duty) {\
            if (duty < 2)\
                stopPWMOutput();\
            else\
                startPWMOutput();\
            timerOrPWM::setExactDutyCycle(duty);\
        };\
        bool canPWM() { return true; };\
        void setInterrupts(const uint32_t interrupts) {\
            timerOrPWM::setInterrupts(interrupts);\
        };\
        TimerChannelInterruptOptions getInterruptCause() {\
            int16_t temp;\
            return timerOrPWM::getInterruptCause(temp);\
        };\
        /*Override these to pick up new methods */\
        private: /* Make these private to catch them early. */\
        /* These are intentially not defined. */\
        void init(const PinMode type, const PinOptions_n options = kNormal);\
        /* WARNING: Covariant return types! */\
        bool get();\
        operator bool();\
    };

    template<>
    struct PWMOutputPin<-1> : PWMOutputPinParent<-1> {
        PWMOutputPin() : PWMOutputPinParent<-1>(kOutput) {};
        PWMOutputPin(const PinOptions_n options, const uint32_t freq = kDefaultPWMFrequency) : PWMOutputPinParent<-1>(kOutput, options) {};
        PWMOutputPin(const uint32_t freq) : PWMOutputPinParent<1->(kOutput, kNormal) {};

        void setFrequency(const uint32_t freq) {};
        operator float() { return !!Pin<pinNum>::getOutputValue(); };
        operator uint32_t() { return (100 * (!!Pin<pinNum>::getOutputValue())); };
        void operator=(const float value) { write(value); };
        void write(const float value) { Pin<pinNum>::write(value >= 0.5); };
        void writeRaw(const uint16_t duty) { Pin<pinNum>::write(duty >= 50); };
        uint16_t getTopValue() { return 100; };
        bool canPWM() { return false; };
        void setInterrupts(const uint32_t interrupts) {
            // This is for timer interrupts, not pin interrupts.
        };

        /*Override these to pick up new methods */

    private: /* Make these private to catch them early. */
        /* These are intentially not defined. */
        void init(const PinMode type, const PinOptions_n options = kNormal);

        /* WARNING: Covariant return types! */
        bool get();
        operator bool();
    };

    // PWMLikeOutputPin is the PWMOutputPin interface on a normal output pin.
    // This is for cases where you want it to act like a non-PWM capable
    // PWMOutputPin, but there actually IS a PWMOutputPin that you explictly
    // don't want to use.
    template<int16_t pinNum>
    struct PWMLikeOutputPin : Pin<pinNum> {
        PWMLikeOutputPin() : Pin<pinNum>(kOutput) {};
        PWMLikeOutputPin(const PinOptions_n options, const uint32_t freq = kDefaultPWMFrequency) : Pin<pinNum>(kOutput, options) {};
        PWMLikeOutputPin(const uint32_t freq) : Pin<pinNum>(kOutput, kNormal) {};
        void setFrequency(const uint32_t freq) {};
        operator float() { return !!Pin<pinNum>::getOutputValue(); };
        operator uint32_t() { return (100 * (!!Pin<pinNum>::getOutputValue())); };
        void operator=(const float value) { write(value); };
        void write(const float value) { Pin<pinNum>::write(value >= 0.5); };
        void writeRaw(const uint16_t duty) { Pin<pinNum>::write(duty >= 50); };
        uint16_t getTopValue() { return 100; };
        bool canPWM() { return false; };
        void setInterrupts(const uint32_t interrupts) {
            // This is for timer interrupts, not pin interrupts.
        };

        /*Override these to pick up new methods */

    private: /* Make these private to catch them early. */
        /* These are intentially not defined. */
        void init(const PinMode type, const PinOptions_n options = kNormal);

        /* WARNING: Covariant return types! */
        bool get();
        operator bool();
    };

    template<int16_t pinNum>
    struct SPIChipSelectPin {
        static const bool is_real = std::false_type::value;
    };

    template<int16_t pinNum>
    constexpr const bool IsSPICSPin() { return SPIChipSelectPin<pinNum>::is_real; };

#define _MAKE_MOTATE_SPI_CS_PIN(registerChar, registerPin, peripheralAorB, csNum)\
    template<>\
    struct SPIChipSelectPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        SPIChipSelectPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
        static const uint8_t moduleId = 0; \
        static const bool is_real = std::true_type::value;\
        static const uint8_t csOffset = csNum;\
    };


    template<int16_t pinNum>
    struct SPIMISOPin {
        static const bool is_real = std::false_type::value;
    };

    template <int16_t pinNum>
    constexpr const bool IsSPIMISOPin() { return SPIMISOPin<pinNum>::is_real; };

#define _MAKE_MOTATE_SPI_MISO_PIN(registerChar, registerPin, peripheralAorB)\
    template<>\
    struct SPIMISOPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        SPIMISOPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
        static const uint8_t moduleId = 0; \
        static const bool is_real = std::true_type::value;\
    };


    template<int16_t pinNum>
    struct SPIMOSIPin {
        static const bool is_real = std::false_type::value;
    };

    template <int16_t pinNum>
    constexpr const bool IsSPIMOSIPin() { return SPIMOSIPin<pinNum>::is_real; };

#define _MAKE_MOTATE_SPI_MOSI_PIN(registerChar, registerPin, peripheralAorB)\
    template<>\
    struct SPIMOSIPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        SPIMOSIPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
        static const uint8_t moduleId = 0; \
        static const bool is_real = std::true_type::value;\
    };



    template<int16_t pinNum>
    struct SPISCKPin {
        static const bool is_real = std::false_type::value;
    };

    template <int16_t pinNum>
    constexpr const bool IsSPISCKPin() { return SPISCKPin<pinNum>::is_real; };

#define _MAKE_MOTATE_SPI_SCK_PIN(registerChar, registerPin, peripheralAorB)\
    template<>\
    struct SPISCKPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        SPISCKPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
        static const uint8_t moduleId = 0; \
        static const bool is_real = std::true_type::value;\
    };



    template<int8_t pinNum>
    struct UARTTxPin {
        static const bool is_real = false;
        static const uint8_t uartNum = -1; // use, we assigned -1 to a uint8_t
    };

    template <int8_t pinNum>
    constexpr const bool IsUARTTxPin() { return UARTTxPin<pinNum>::is_real; };

    #define _MAKE_MOTATE_UART_TX_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTTxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTTxPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB, kPullUp) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };


    template<int8_t pinNum>
    struct UARTRxPin {
        static const bool is_real = false;
        static const uint8_t uartNum = -1; // use, we assigned -1 to a uint8_t
    };

    template <int8_t pinNum>
    constexpr const bool IsUARTRxPin() { return UARTRxPin<pinNum>::is_real; };

    #define _MAKE_MOTATE_UART_RX_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTRxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTRxPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };



    template<int8_t pinNum>
    struct UARTRTSPin {
        static const bool is_real = false;
        static const uint8_t uartNum = -1; // use, we assigned -1 to a uint8_t
    };

    template <int8_t pinNum>
    constexpr const bool IsUARTRTSPin() { return UARTRTSPin<pinNum>::is_real; };

    #define _MAKE_MOTATE_UART_RTS_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTRTSPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTRTSPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };


    template<int8_t pinNum>
    struct UARTCTSPin {
        static const bool is_real = false;
        static const uint8_t uartNum = -1; // use, we assigned -1 to a uint8_t
    };

    template <int8_t pinNum>
    constexpr const bool IsUARTCTSPin() { return UARTCTSPin<pinNum>::is_real; };

    #define _MAKE_MOTATE_UART_CTS_PIN(registerChar, registerPin, uartNumVal, peripheralAorB)\
        template<>\
        struct UARTCTSPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
            UARTCTSPin() : ReversePinLookup<registerChar, registerPin>(kPeripheral ## peripheralAorB, kPullUp) {};\
            static const uint8_t uartNum = uartNumVal;\
            static const bool is_real = true;\
        };

    typedef Pin<-1> NullPin;
    static NullPin nullPin;


    _MAKE_MOTATE_PORT32(A, 'A');
    _MAKE_MOTATE_PORT32(B, 'B');
#ifdef PIOC
    _MAKE_MOTATE_PORT32(C, 'C');
#endif
#ifdef PIOD
    _MAKE_MOTATE_PORT32(D, 'D');
#endif

} // end namespace Motate


// Note: We end the namespace before including in case the included file need to include
//   another Motate file. If it does include another Motate file, we end up with
//   Motate::Motate::* definitions and weird compiler errors.
#include "motate_pin_assignments.h"

#endif /* end of include guard: SAMPINS_H_ONCE */
