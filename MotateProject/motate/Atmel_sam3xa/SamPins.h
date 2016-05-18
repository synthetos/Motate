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
        kStartHigh      = 1<<1,
        kStartLow       = 1<<1,

        // For use on PWM pins only!
        kPWMPinInverted    = 1<<7,
    };

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
    struct Port32 {
        static const uint8_t letter = 0; // NULL stub!

        static Pio* portPtr() {
            return 0;
        };
        static const uint32_t pmcId() {
            return 0;
        };

        void setModes(const uintPort_t value, const uintPort_t mask = 0xffffffff) {
            // stub
        };
        void setOptions(const uint16_t options, const uintPort_t mask) {
            // stub
        };
        void getModes() {
            // stub
        };
        void getOptions() {
            // stub
        };
        void set(const uintPort_t value) {
            // stub
        };
        void clear(const uintPort_t value) {
            // stub
        };
        void write(const uintPort_t value) {
            // stub
        };
        void write(const uintPort_t value, const uintPort_t mask) {
            // stub
        };
        uintPort_t getInputValues(const uintPort_t mask = 0xffffffff) {
            // stub
            return 0;
        };
        uintPort_t getOutputValues(const uintPort_t mask = 0xffffffff) {
            // stub
            return 0;
        };
    };

    template<int16_t pinNum>
    struct Pin {
        static const int8_t number = -1;
        static const uint8_t portLetter = 0;
        static const uint32_t mask = 0;

        Pin() {};
        Pin(const PinMode type, const PinOptions options = kNormal) {};
        void operator=(const bool value) {};
        operator bool() { return 0; };

        void init(const PinMode type, const uint16_t options = kNormal, const bool fromConstructor=false) {};
        void setMode(const PinMode type, const bool fromConstructor=false) {};
        PinMode getMode() { return kUnchanged; };
        void setOptions(const uint16_t options, const bool fromConstructor=false) {};
        uint16_t getOptions() { return kNormal; };
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
        ReversePinLookup(const PinMode type, const PinOptions options = kNormal) : Pin<-1>(type, options) {};
    };

    template<int16_t pinNum>
    struct InputPin : Pin<pinNum> {
        InputPin() : Pin<pinNum>(kInput) {};
        InputPin(const PinOptions options) : Pin<pinNum>(kInput, options) {};
        void init(const PinOptions options = kNormal  ) {Pin<pinNum>::init(kInput, options);};
        uint32_t get() {
            return Pin<pinNum>::getInputValue();
        };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. These are intentionally not defined. */
        void init(const PinMode type, const PinOptions options = kNormal);
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        void write(const bool);
    };

    template<int16_t pinNum>
    struct OutputPin : Pin<pinNum> {
        OutputPin() : Pin<pinNum>(kOutput) {};
        OutputPin(const PinOptions options) : Pin<pinNum>(kOutput, options) {};
        void init(const PinOptions options = kNormal) {Pin<pinNum>::init(kOutput, options);};
        uint32_t get() {
            return Pin<pinNum>::getOutputValue();
        };
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. */
        void init(const PinMode type, const PinOptions options = kNormal); /* Intentially not defined. */
    };



    // All of the pins on the SAM can be an interrupt pin
    // but we create these objects to share the interface with other architectures.
    template<int16_t pinNum>
    struct IRQPin : Pin<pinNum> {
        IRQPin() : Pin<pinNum>(kInput) {};
        IRQPin(const PinOptions options) : Pin<pinNum>(kInput, options) {};
        void init(const PinOptions options = kNormal  ) {Pin<pinNum>::init(kInput, options);};

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

    // Some pins are ADC pins.
    template<int16_t pinNum>
    struct ADCPin : Pin<-1> {
        ADCPin() : Pin<-1>() {};
        ADCPin(const PinOptions options) : Pin<-1>() {};

        uint32_t getRaw() {
            return 0;
        };
        uint32_t getValue() {
            return 0;
        };
        operator int16_t() {
            return getValue();
        };
        operator float() {
            return 0.0;
        };
        static const uint16_t getTop() { return 4095; };

        static const bool is_real = false;
        static void interrupt() __attribute__ (( weak )); // Allow setting an interrupt on a invalid ADC pin -- will never be called
    };

    template<int16_t adcNum>
    struct ReverseADCPin : ADCPin<-1> {
        ReverseADCPin() : ADCPin<-1>() {};
        ReverseADCPin(const PinOptions options) : ADCPin<-1>() {};
    };

    #define _MAKE_MOTATE_ADC_PIN(registerChar, registerPin, adcNum) \
    template<> \
    struct ADCPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin>, ADC_Module { \
        static const uint32_t adcMask = 1 << adcNum; \
        static const uint32_t adcNumber = adcNum; \
        static const uint16_t getTop() { return 4095; }; \
        ADCPin() : ReversePinLookup<registerChar, registerPin>(kInput), ADC_Module() { init(); }; \
        ADCPin(const PinOptions options) : ReversePinLookup<registerChar, registerPin>(kInput, options), ADC_Module() { init(); }; \
        void init() { \
            /* Enable the pin */ \
            ADC->ADC_CHER = adcMask; \
            /* Enable the conversion TAG */ \
            ADC->ADC_EMR = ADC_EMR_TAG ; \
        }; \
        uint32_t getRaw() { \
            return ADC->ADC_CDR[adcNum]; \
        }; \
        uint32_t getValue() { \
            if ((ADC->ADC_CHSR & adcMask) != adcMask) { \
                ADC->ADC_CR = ADC_CR_START; /* start the sample */ \
                while ((ADC->ADC_ISR & ADC_ISR_DRDY) != ADC_ISR_DRDY) {;} /* Wait... */ \
            } \
            return getRaw(); \
        }; \
        operator int16_t() { \
            return getValue(); \
        }; \
        operator float() { \
            return (float)getValue() / getTop(); \
        }; \
        static const bool is_real = true; \
        void setInterrupts(const uint32_t interrupts) { \
            if (interrupts != kPinInterruptsOff) { \
                /* Set interrupt priority */ \
                if (interrupts & kPinInterruptPriorityMask) { \
                    if (interrupts & kPinInterruptPriorityHighest) { \
                        NVIC_SetPriority(ADC_IRQn, 0); \
                    } \
                    else if (interrupts & kPinInterruptPriorityHigh) { \
                        NVIC_SetPriority(ADC_IRQn, 3); \
                    } \
                    else if (interrupts & kPinInterruptPriorityMedium) { \
                        NVIC_SetPriority(ADC_IRQn, 7); \
                    } \
                    else if (interrupts & kPinInterruptPriorityLow) { \
                        NVIC_SetPriority(ADC_IRQn, 11); \
                    } \
                    else if (interrupts & kPinInterruptPriorityLowest) { \
                        NVIC_SetPriority(ADC_IRQn, 15); \
                    } \
                } \
                /* Enable the IRQ */ \
                NVIC_EnableIRQ(ADC_IRQn); \
                /* Enable the interrupt */ \
                ADC->ADC_IER = adcMask; \
                /* Enable the pin */ \
                ADC->ADC_CHER = adcMask; \
            } else { \
                /* Disable the pin */ \
                ADC->ADC_CHDR = adcMask; \
                /* Disable the interrupt */ \
                ADC->ADC_IDR = adcMask; \
                /* Disable the interrupt - if all channels are disabled */ \
                if (ADC->ADC_CHSR == 0) { \
                    NVIC_DisableIRQ(ADC_IRQn); \
                } \
            } \
        }; \
        static void interrupt(); \
    }; \
    template<> \
    struct ReverseADCPin<adcNum> : ADCPin<ReversePinLookup<registerChar, registerPin>::number> { \
        ReverseADCPin() : ADCPin<ReversePinLookup<registerChar, registerPin>::number>() {}; \
        ReverseADCPin(const PinOptions options) : ADCPin<ReversePinLookup<registerChar, registerPin>::number>(options) {}; \
    };

    template<int16_t pinNum>
    constexpr const bool IsADCPin() { return ADCPin<pinNum>::is_real; };

    template<uint8_t portChar, int16_t portPin>
    using LookupADCPin = ADCPin< ReversePinLookup<portChar, portPin>::number >;

    template<int16_t adcNum>
    using LookupADCPinByADC = ADCPin< ReverseADCPin< adcNum >::number >;

    // TODO: Make the Pin<> use the appropriate Port<>, reducing duplication when there's no penalty

#define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerChar, registerPin)\
    template<>\
    struct Pin<pinNum> {\
        private: /* Lock the copy contructor.*/\
        Pin(const Pin<pinNum>&){};\
        public:\
        static const int16_t number = pinNum;\
        static const uint8_t portLetter = (uint8_t) registerChar;\
        static const uint32_t mask = (1u << registerPin);\
        \
        Pin() {};\
        Pin(const PinMode type, const PinOptions options = kNormal) {\
            init(type, options, /*fromConstructor=*/true);\
        };\
        void operator=(const bool value) { write(value); };\
        operator bool() { return (get() != 0); };\
        \
        void init(const PinMode type, const uint16_t options = kNormal, const bool fromConstructor=false) {\
            setMode(type, fromConstructor);\
            setOptions(options, fromConstructor);\
        };\
        void setMode(const PinMode type, const bool fromConstructor=false) {\
            switch (type) {\
                case kOutput:\
                    (*PIO ## registerLetter).PIO_OER = mask ;\
                    (*PIO ## registerLetter).PIO_PER = mask ;\
                    /* if all pins are output, disable PIO Controller clocking, reduce power consumption */\
                    if (!fromConstructor) {\
                        if ( (*PIO ## registerLetter).PIO_OSR == 0xffffffff )\
                        {\
                            Port32<registerChar>::disablePeripheralClock();\
                        }\
                    }\
                break;\
                case kInput:\
                    Port32<registerChar>::enablePeripheralClock();\
                    (*PIO ## registerLetter).PIO_ODR = mask ;\
                    (*PIO ## registerLetter).PIO_PER = mask ;\
                    break;\
                case kPeripheralA:\
                    (*PIO ## registerLetter).PIO_ABSR &= ~mask ;\
                    (*PIO ## registerLetter).PIO_PDR = mask ;\
                    break;\
                case kPeripheralB:\
                    (*PIO ## registerLetter).PIO_ABSR |= mask ;\
                    (*PIO ## registerLetter).PIO_PDR = mask ;\
                    break;\
                default:\
                    break;\
            }\
        };\
        PinMode getMode() {\
            return ((*PIO ## registerLetter).PIO_OSR & mask) ? kOutput : kInput;\
        };\
        void setOptions(const uint16_t options, const bool fromConstructor=false) {\
            if (kStartLow & options)\
            {\
                (*PIO ## registerLetter).PIO_SODR = mask ;\
            } else if (kStartHigh & options)\
            {\
                (*PIO ## registerLetter).PIO_CODR = mask ;\
            }\
            if (kPullUp & options)\
            {\
                (*PIO ## registerLetter).PIO_PUER = mask ;\
            }\
            else\
            {\
                (*PIO ## registerLetter).PIO_PUDR = mask ;\
            }\
            if (kWiredAnd & options)\
            {/*kDriveLowOnly - Enable Multidrive*/\
                (*PIO ## registerLetter).PIO_MDER = mask ;\
            }\
            else\
            {\
                (*PIO ## registerLetter).PIO_MDDR = mask ;\
            }\
            if (kDeglitch & options)\
            {\
                (*PIO ## registerLetter).PIO_IFER = mask ;\
                (*PIO ## registerLetter).PIO_SCIFSR = mask ;\
            }\
            else\
            {\
                if (kDebounce & options)\
                {\
                    (*PIO ## registerLetter).PIO_IFER = mask ;\
                    (*PIO ## registerLetter).PIO_DIFSR = mask ;\
                }\
                else\
                {\
                    (*PIO ## registerLetter).PIO_IFDR = mask ;\
                }\
            }\
        };\
        uint16_t getOptions() {\
            return (((*PIO ## registerLetter).PIO_PUSR & mask) ? kPullUp : 0)\
            | (((*PIO ## registerLetter).PIO_MDSR & mask) ? kWiredAnd : 0)\
            | (((*PIO ## registerLetter).PIO_IFSR & mask) ? \
            (((*PIO ## registerLetter).PIO_IFDGSR & mask) ? kDebounce : kDeglitch) : 0);\
        };\
        void set() {\
            (*PIO ## registerLetter).PIO_SODR = mask;\
        };\
        void clear() {\
            (*PIO ## registerLetter).PIO_CODR = mask;\
        };\
        void write(const bool value) {\
            if (!value)\
                clear();\
            else\
                set();\
        };\
        void toggle()  {\
            (*PIO ## registerLetter).PIO_OWER = mask; /*Enable writing thru ODSR*/\
            (*PIO ## registerLetter).PIO_ODSR ^= mask;\
        };\
        uint32_t get() { /* WARNING: This will fail if the peripheral clock is disabled for this pin!!! Use getOutputValue() instead. */\
            return (*PIO ## registerLetter).PIO_PDSR & mask;\
        };\
        uint32_t getInputValue() {\
            return (*PIO ## registerLetter).PIO_PDSR & mask;\
        };\
        uint32_t getOutputValue() {\
            return (*PIO ## registerLetter).PIO_ODSR & mask;\
        };\
        void setInterrupts(const uint32_t interrupts) {\
            port ## registerLetter.setInterrupts(interrupts, mask);\
        };\
        bool isNull() { return false; };\
        static uint32_t maskForPort(const uint8_t otherPortLetter) {\
            return portLetter == otherPortLetter ? mask : 0x00u;\
        };\
    };\
    typedef Pin<pinNum> Pin ## pinNum;\
    static Pin ## pinNum pin ## pinNum;\
    template<>\
    struct ReversePinLookup<registerChar, registerPin> : Pin<pinNum> {\
        ReversePinLookup() {};\
        ReversePinLookup(const PinMode type, const PinOptions options = kNormal) : Pin<pinNum>(type, options) {};\
    };\
    template<> void Motate::IRQPin<pinNum>::interrupt();




    static const uint32_t kDefaultPWMFrequency = 1000;
    template<int16_t pinNum>
    struct PWMOutputPin : Pin<pinNum> {
        PWMOutputPin() : Pin<pinNum>(kOutput) {};
        PWMOutputPin(const PinOptions options, const uint32_t freq = kDefaultPWMFrequency) : Pin<pinNum>(kOutput, options) {};
        PWMOutputPin(const uint32_t freq) : Pin<pinNum>(kOutput, kNormal) {};
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
        void init(const PinMode type, const PinOptions options = kNormal);

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
        PWMLikeOutputPin(const PinOptions options, const uint32_t freq = kDefaultPWMFrequency) : Pin<pinNum>(kOutput, options) {};
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
        void init(const PinMode type, const PinOptions options = kNormal);

        /* WARNING: Covariant return types! */
        bool get();
        operator bool();
    };


#define _MAKE_MOTATE_PWM_PIN(registerChar, registerPin, timerOrPWM, peripheralAorB, invertedByDefault)\
    template<>\
    struct PWMOutputPin< ReversePinLookup<registerChar, registerPin>::number > : Pin< ReversePinLookup<registerChar, registerPin>::number >, timerOrPWM {\
        static const pin_number pinNum = ReversePinLookup<registerChar, registerPin>::number;\
        typedef timerOrPWM parentTimerType;\
        PWMOutputPin() : Pin<pinNum>(kPeripheral ## peripheralAorB), timerOrPWM(Motate::kTimerUpToMatch, kDefaultPWMFrequency) { pwmpin_init(kNormal);};\
        PWMOutputPin(const PinOptions options, const uint32_t freq = kDefaultPWMFrequency) :\
                Pin<pinNum>(kPeripheral ## peripheralAorB, options), timerOrPWM(Motate::kTimerUpToMatch, freq)\
            {pwmpin_init(options);};\
        PWMOutputPin(const uint32_t freq) :\
                Pin<pinNum>(kPeripheral ## peripheralAorB, kNormal), timerOrPWM(Motate::kTimerUpToMatch, freq)\
            {pwmpin_init(kNormal);};\
        void pwmpin_init(const PinOptions options) {\
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
        void init(const PinMode type, const PinOptions options = kNormal);\
        /* WARNING: Covariant return types! */\
        bool get();\
        operator bool();\
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



#define _MAKE_MOTATE_PORT32(registerLetter, registerChar)\
    template <>\
    struct Port32<registerChar> : SamCommon< Port32<registerChar> > {\
        static const uint8_t letter = (uint8_t) registerChar;\
        void setModes(const uintPort_t value, const uintPort_t mask) {\
            (*PIO ## registerLetter).PIO_ODR = ~value & mask ;\
            (*PIO ## registerLetter).PIO_OER = value & mask ;\
            (*PIO ## registerLetter).PIO_PER = mask ;\
            /* if all pins are output, disable PIO Controller clocking, reduce power consumption */\
            if ( (*PIO ## registerLetter).PIO_OSR == 0xffffffff )\
            {\
                disablePeripheralClock();\
            } else {\
                enablePeripheralClock();\
            }\
        };\
        void setOptions(const uint16_t options, const uintPort_t mask) {\
            if (kPullUp & options)\
            {\
                (*PIO ## registerLetter).PIO_PUER = mask ;\
            }\
            else\
            {\
                (*PIO ## registerLetter).PIO_PUDR = mask ;\
            }\
            if (kWiredAnd & options)\
            {/*kDriveLowOnly - Enable Multidrive*/\
                (*PIO ## registerLetter).PIO_MDER = mask ;\
            }\
            else\
            {\
                (*PIO ## registerLetter).PIO_MDDR = mask ;\
            }\
            if (kDeglitch & options)\
            {\
                (*PIO ## registerLetter).PIO_IFER = mask ;\
                (*PIO ## registerLetter).PIO_SCIFSR = mask ;\
            }\
            else\
            {\
                if (kDebounce & options)\
                {\
                    (*PIO ## registerLetter).PIO_IFER = mask ;\
                    (*PIO ## registerLetter).PIO_DIFSR = mask ;\
                }\
                else\
                {\
                    (*PIO ## registerLetter).PIO_IFDR = mask ;\
                }\
            }\
        };\
        void set(const uintPort_t value) {\
            (*PIO ## registerLetter).PIO_SODR = value;\
        };\
        void clear(const uintPort_t value) {\
            (*PIO ## registerLetter).PIO_CODR = value;\
        };\
        void write(const uintPort_t value) {\
            (*PIO ## registerLetter).PIO_OWER = 0xffffffff;/*Enable all registers for writing thru ODSR*/\
            (*PIO ## registerLetter).PIO_ODSR = value;\
            (*PIO ## registerLetter).PIO_OWDR = 0xffffffff;/*Disable all registers for writing thru ODSR*/\
        };\
        void write(const uintPort_t value, const uintPort_t mask) {\
            (*PIO ## registerLetter).PIO_OWER = mask;/*Enable masked registers for writing thru ODSR*/\
            (*PIO ## registerLetter).PIO_ODSR = value;\
            (*PIO ## registerLetter).PIO_OWDR = mask;/*Disable masked registers for writing thru ODSR*/\
        };\
        uintPort_t getInputValues(const uintPort_t mask) {\
            return (*PIO ## registerLetter).PIO_PDSR & mask;\
        };\
        uintPort_t getOutputValues(const uintPort_t mask) {\
            return (*PIO ## registerLetter).PIO_ODSR & mask;\
        };\
        Pio* portPtr() {\
            return (PIO ## registerLetter);\
        };\
        static const uint32_t peripheralId() {\
            return ID_PIO ## registerLetter;\
        };\
        void setInterrupts(const uint32_t interrupts, const uintPort_t mask) {\
            if (interrupts != kPinInterruptsOff) {\
                (*PIO ## registerLetter).PIO_IDR = mask;\
                \
                /*Is it an "advanced" interrupt?*/\
                if (interrupts & kPinInterruptAdvancedMask) {\
                    (*PIO ## registerLetter).PIO_AIMER = mask;\
                    /*Is it an edge interrupt?*/\
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||\
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnFallingEdge) {\
                        (*PIO ## registerLetter).PIO_ESR = mask;\
                    }\
                    else\
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel ||\
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnLowLevel) {\
                        (*PIO ## registerLetter).PIO_LSR = mask;\
                    }\
                    /*Rising Edge/High Level, or Falling Edge/LowLevel?*/\
                    if ((interrupts & kPinInterruptTypeMask) == kPinInterruptOnRisingEdge ||\
                        (interrupts & kPinInterruptTypeMask) == kPinInterruptOnHighLevel) {\
                        (*PIO ## registerLetter).PIO_REHLSR = mask;\
                    }\
                    else\
                    {\
                        (*PIO ## registerLetter).PIO_FELLSR = mask;\
                    }\
                }\
                else\
                {\
                    (*PIO ## registerLetter).PIO_AIMDR = mask;\
                }\
                \
                /* Set interrupt priority */\
                if (interrupts & kPinInterruptPriorityMask) {\
                    if (interrupts & kPinInterruptPriorityHighest) {\
                        NVIC_SetPriority(PIO ## registerLetter ## _IRQn, 0);\
                    }\
                    else if (interrupts & kPinInterruptPriorityHigh) {\
                        NVIC_SetPriority(PIO ## registerLetter ## _IRQn, 3);\
                    }\
                    else if (interrupts & kPinInterruptPriorityMedium) {\
                        NVIC_SetPriority(PIO ## registerLetter ## _IRQn, 7);\
                    }\
                    else if (interrupts & kPinInterruptPriorityLow) {\
                        NVIC_SetPriority(PIO ## registerLetter ## _IRQn, 11);\
                    }\
                    else if (interrupts & kPinInterruptPriorityLowest) {\
                        NVIC_SetPriority(PIO ## registerLetter ## _IRQn, 15);\
                    }\
                }\
                /* Enable the IRQ */\
                NVIC_EnableIRQ(PIO ## registerLetter ## _IRQn);\
                /* Enable the interrupt */\
                (*PIO ## registerLetter).PIO_IER = mask;\
            } else {\
                (*PIO ## registerLetter).PIO_IDR = mask;\
                if ((*PIO ## registerLetter).PIO_ISR == 0)\
                    NVIC_DisableIRQ(PIO ## registerLetter ## _IRQn);\
            }\
        };\
    };\
typedef Port32<registerChar> Port ## registerLetter;\
static Port ## registerLetter port ## registerLetter;

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

namespace Motate {

    // disable pinholder for Due for now -- nned to convert to 32bit
    // PinHolder - 32bit virtual ports (I've never made a template with 32 parameters before.)
    template<
    int8_t PinBit31num,
    int8_t PinBit30num = -1,
    int8_t PinBit29num = -1,
    int8_t PinBit28num = -1,
    int8_t PinBit27num = -1,
    int8_t PinBit26num = -1,
    int8_t PinBit25num = -1,
    int8_t PinBit24num = -1,
    int8_t PinBit23num = -1,
    int8_t PinBit22num = -1,
    int8_t PinBit21num = -1,
    int8_t PinBit20num = -1,
    int8_t PinBit19num = -1,
    int8_t PinBit18num = -1,
    int8_t PinBit17num = -1,
    int8_t PinBit16num = -1,
    int8_t PinBit15num = -1,
    int8_t PinBit14num = -1,
    int8_t PinBit13num = -1,
    int8_t PinBit12num = -1,
    int8_t PinBit11num = -1,
    int8_t PinBit10num = -1,
    int8_t PinBit9num  = -1,
    int8_t PinBit8num  = -1,
    int8_t PinBit7num  = -1,
    int8_t PinBit6num  = -1,
    int8_t PinBit5num  = -1,
    int8_t PinBit4num  = -1,
    int8_t PinBit3num  = -1,
    int8_t PinBit2num  = -1,
    int8_t PinBit1num  = -1,
    int8_t PinBit0num  = -1>
    class PinHolder32 {

        static Pin<PinBit31num> PinBit31;
        static Pin<PinBit30num> PinBit30;
        static Pin<PinBit29num> PinBit29;
        static Pin<PinBit28num> PinBit28;
        static Pin<PinBit27num> PinBit27;
        static Pin<PinBit26num> PinBit26;
        static Pin<PinBit25num> PinBit25;
        static Pin<PinBit24num> PinBit24;
        static Pin<PinBit23num> PinBit23;
        static Pin<PinBit22num> PinBit22;
        static Pin<PinBit21num> PinBit21;
        static Pin<PinBit20num> PinBit20;
        static Pin<PinBit19num> PinBit19;
        static Pin<PinBit18num> PinBit18;
        static Pin<PinBit17num> PinBit17;
        static Pin<PinBit16num> PinBit16;
        static Pin<PinBit15num> PinBit15;
        static Pin<PinBit14num> PinBit14;
        static Pin<PinBit13num> PinBit13;
        static Pin<PinBit12num> PinBit12;
        static Pin<PinBit11num> PinBit11;
        static Pin<PinBit10num> PinBit10;
        static Pin<PinBit9num>	PinBit9;
        static Pin<PinBit8num>	PinBit8;
        static Pin<PinBit7num>	PinBit7;
        static Pin<PinBit6num>	PinBit6;
        static Pin<PinBit5num>	PinBit5;
        static Pin<PinBit4num>	PinBit4;
        static Pin<PinBit3num>	PinBit3;
        static Pin<PinBit2num>	PinBit2;
        static Pin<PinBit1num>	PinBit1;
        static Pin<PinBit0num>	PinBit0;

#define _MOTATE_PH32_CREATE_CLEAR_AND_COPY_MASKS(aPortLetter) \
    static const uint32_t port ## aPortLetter ## ClearMask =\
        (Pin<PinBit31num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit31num>::mask : 0x00) |\
        (Pin<PinBit30num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit30num>::mask : 0x00) |\
        (Pin<PinBit29num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit29num>::mask : 0x00) |\
        (Pin<PinBit28num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit28num>::mask : 0x00) |\
        (Pin<PinBit27num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit27num>::mask : 0x00) |\
        (Pin<PinBit26num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit26num>::mask : 0x00) |\
        (Pin<PinBit25num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit25num>::mask : 0x00) |\
        (Pin<PinBit24num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit24num>::mask : 0x00) |\
        (Pin<PinBit23num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit23num>::mask : 0x00) |\
        (Pin<PinBit22num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit22num>::mask : 0x00) |\
        (Pin<PinBit21num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit21num>::mask : 0x00) |\
        (Pin<PinBit20num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit20num>::mask : 0x00) |\
        (Pin<PinBit19num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit19num>::mask : 0x00) |\
        (Pin<PinBit18num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit18num>::mask : 0x00) |\
        (Pin<PinBit17num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit17num>::mask : 0x00) |\
        (Pin<PinBit16num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit16num>::mask : 0x00) |\
        (Pin<PinBit15num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit15num>::mask : 0x00) |\
        (Pin<PinBit14num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit14num>::mask : 0x00) |\
        (Pin<PinBit13num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit13num>::mask : 0x00) |\
        (Pin<PinBit12num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit12num>::mask : 0x00) |\
        (Pin<PinBit11num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit11num>::mask : 0x00) |\
        (Pin<PinBit10num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit10num>::mask : 0x00) |\
        (Pin<PinBit9num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit9num>::mask  : 0x00) |\
        (Pin<PinBit8num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit8num>::mask  : 0x00) |\
        (Pin<PinBit7num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit7num>::mask  : 0x00) |\
        (Pin<PinBit6num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit6num>::mask  : 0x00) |\
        (Pin<PinBit5num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit5num>::mask  : 0x00) |\
        (Pin<PinBit4num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit4num>::mask  : 0x00) |\
        (Pin<PinBit3num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit3num>::mask  : 0x00) |\
        (Pin<PinBit2num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit2num>::mask  : 0x00) |\
        (Pin<PinBit1num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit1num>::mask  : 0x00) |\
        (Pin<PinBit0num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit0num>::mask  : 0x00);\
\
    static const uint32_t port ## aPortLetter ## CopyMask =\
        (Pin<PinBit31num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit31num>::mask == 0x80000000u ? Pin<PinBit31num>::mask : 0x00) |\
        (Pin<PinBit30num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit30num>::mask == 0x40000000u ? Pin<PinBit30num>::mask : 0x00) |\
        (Pin<PinBit29num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit29num>::mask == 0x20000000u ? Pin<PinBit29num>::mask : 0x00) |\
        (Pin<PinBit28num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit28num>::mask == 0x10000000u ? Pin<PinBit28num>::mask : 0x00) |\
        (Pin<PinBit27num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit27num>::mask == 0x08000000u ? Pin<PinBit27num>::mask : 0x00) |\
        (Pin<PinBit26num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit26num>::mask == 0x04000000u ? Pin<PinBit26num>::mask : 0x00) |\
        (Pin<PinBit25num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit25num>::mask == 0x02000000u ? Pin<PinBit25num>::mask : 0x00) |\
        (Pin<PinBit24num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit24num>::mask == 0x01000000u ? Pin<PinBit24num>::mask : 0x00) |\
        (Pin<PinBit23num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit23num>::mask == 0x00800000u ? Pin<PinBit23num>::mask : 0x00) |\
        (Pin<PinBit22num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit22num>::mask == 0x00400000u ? Pin<PinBit22num>::mask : 0x00) |\
        (Pin<PinBit21num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit21num>::mask == 0x00200000u ? Pin<PinBit21num>::mask : 0x00) |\
        (Pin<PinBit20num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit20num>::mask == 0x00100000u ? Pin<PinBit20num>::mask : 0x00) |\
        (Pin<PinBit19num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit19num>::mask == 0x00080000u ? Pin<PinBit19num>::mask : 0x00) |\
        (Pin<PinBit18num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit18num>::mask == 0x00040000u ? Pin<PinBit18num>::mask : 0x00) |\
        (Pin<PinBit17num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit17num>::mask == 0x00020000u ? Pin<PinBit17num>::mask : 0x00) |\
        (Pin<PinBit16num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit16num>::mask == 0x00010000u ? Pin<PinBit16num>::mask : 0x00) |\
        (Pin<PinBit15num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit15num>::mask == 0x00008000u ? Pin<PinBit15num>::mask : 0x00) |\
        (Pin<PinBit14num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit14num>::mask == 0x00004000u ? Pin<PinBit14num>::mask : 0x00) |\
        (Pin<PinBit13num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit13num>::mask == 0x00002000u ? Pin<PinBit13num>::mask : 0x00) |\
        (Pin<PinBit12num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit12num>::mask == 0x00001000u ? Pin<PinBit12num>::mask : 0x00) |\
        (Pin<PinBit11num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit11num>::mask == 0x00000800u ? Pin<PinBit11num>::mask : 0x00) |\
        (Pin<PinBit10num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit10num>::mask == 0x00000400u ? Pin<PinBit10num>::mask : 0x00) |\
        (Pin<PinBit9num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit9num>::mask  == 0x00000200u ? Pin<PinBit9num>::mask  : 0x00) |\
        (Pin<PinBit8num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit8num>::mask  == 0x00000100u ? Pin<PinBit8num>::mask  : 0x00) |\
        (Pin<PinBit7num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit7num>::mask  == 0x00000080u ? Pin<PinBit7num>::mask  : 0x00) |\
        (Pin<PinBit6num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit6num>::mask  == 0x00000040u ? Pin<PinBit6num>::mask  : 0x00) |\
        (Pin<PinBit5num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit5num>::mask  == 0x00000020u ? Pin<PinBit5num>::mask  : 0x00) |\
        (Pin<PinBit4num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit4num>::mask  == 0x00000010u ? Pin<PinBit4num>::mask  : 0x00) |\
        (Pin<PinBit3num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit3num>::mask  == 0x00000008u ? Pin<PinBit3num>::mask  : 0x00) |\
        (Pin<PinBit2num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit2num>::mask  == 0x00000004u ? Pin<PinBit2num>::mask  : 0x00) |\
        (Pin<PinBit1num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit1num>::mask  == 0x00000002u ? Pin<PinBit1num>::mask  : 0x00) |\
        (Pin<PinBit0num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit0num>::mask  == 0x00000001u ? Pin<PinBit0num>::mask  : 0x00);

        _MOTATE_PH32_CREATE_CLEAR_AND_COPY_MASKS(A);
        _MOTATE_PH32_CREATE_CLEAR_AND_COPY_MASKS(B);
#ifdef PIOC
        _MOTATE_PH32_CREATE_CLEAR_AND_COPY_MASKS(C);
#endif
#ifdef PIOD
        _MOTATE_PH32_CREATE_CLEAR_AND_COPY_MASKS(D);
#endif
    public:
        PinHolder32() {

        };

        void write(uint32_t in_value) {
            uint32_t port_value	   = 0x00; // Port<> handles reading the port and setting the masked pins
#define _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, bitNumber, bitMask) \
            if (PinBit ## bitNumber.maskForPort(Port ## portLetter::letter) &&\
                (PinBit ## bitNumber.mask != (bitMask)) && (in_value & (bitMask))) {\
                port_value |= PinBit ## bitNumber.mask;\
            }

            // Using direct 0x00000000 notation instead of 1<<x, since the compiler occasionally won't precompile that.
            // Shortcut: ruby -e '(0..31).each() { |x| print "_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, %2d, 0x%08x);\\\n" % [31-x, (1<<(31-x))]}'
#define _MOTATE_PH32_PINHOLDER_SETPORT(portLetter) \
            if (port ## portLetter ## ClearMask != 0x00) {\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 31, 0x80000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 30, 0x40000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 29, 0x20000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 28, 0x10000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 27, 0x08000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 26, 0x04000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 25, 0x02000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 24, 0x01000000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 23, 0x00800000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 22, 0x00400000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 21, 0x00200000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 20, 0x00100000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 19, 0x00080000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 18, 0x00040000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 17, 0x00020000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 16, 0x00010000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 15, 0x00008000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 14, 0x00004000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 13, 0x00002000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 12, 0x00001000u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 11, 0x00000800u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter, 10, 0x00000400u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  9, 0x00000200u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  8, 0x00000100u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  7, 0x00000080u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  6, 0x00000040u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  5, 0x00000020u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  4, 0x00000010u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  3, 0x00000008u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  2, 0x00000004u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  1, 0x00000002u);\
                _MOTATE_PH32_PINHOLDER_CHECKANDSETPIN(portLetter,  0, 0x00000001u);\
                port_value |= in_value & port ## portLetter ## CopyMask;\
                port ## portLetter.write(port_value, ~port ## portLetter ## ClearMask);\
            }

            _MOTATE_PH32_PINHOLDER_SETPORT(A);
            _MOTATE_PH32_PINHOLDER_SETPORT(B);
#ifdef PIOC
            _MOTATE_PH32_PINHOLDER_SETPORT(C);
#endif
#ifdef PIOD
            _MOTATE_PH32_PINHOLDER_SETPORT(D);
#endif
        }
    };

    // disable pinholder for Due for now -- nned to convert to 32bit
    // PinHolder - 32bit virtual ports (I've never made a template with 32 parameters before.)
    template<
    int8_t PinBit7num,
    int8_t PinBit6num  = -1,
    int8_t PinBit5num  = -1,
    int8_t PinBit4num  = -1,
    int8_t PinBit3num  = -1,
    int8_t PinBit2num  = -1,
    int8_t PinBit1num  = -1,
    int8_t PinBit0num  = -1>
    class PinHolder8 {

        static Pin<PinBit7num>	PinBit7;
        static Pin<PinBit6num>	PinBit6;
        static Pin<PinBit5num>	PinBit5;
        static Pin<PinBit4num>	PinBit4;
        static Pin<PinBit3num>	PinBit3;
        static Pin<PinBit2num>	PinBit2;
        static Pin<PinBit1num>	PinBit1;
        static Pin<PinBit0num>	PinBit0;
    public:
#define _MOTATE_PH8_CREATE_CLEAR_AND_COPY_MASKS(aPortLetter) \
    static const uint32_t port ## aPortLetter ## ClearMask =\
        (Pin<PinBit7num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit7num>::mask  : 0x00u) |\
        (Pin<PinBit6num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit6num>::mask  : 0x00u) |\
        (Pin<PinBit5num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit5num>::mask  : 0x00u) |\
        (Pin<PinBit4num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit4num>::mask  : 0x00u) |\
        (Pin<PinBit3num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit3num>::mask  : 0x00u) |\
        (Pin<PinBit2num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit2num>::mask  : 0x00u) |\
        (Pin<PinBit1num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit1num>::mask  : 0x00u) |\
        (Pin<PinBit0num>::portLetter  == Port ## aPortLetter::letter ? Pin<PinBit0num>::mask  : 0x00u);\
\
    static const uint32_t port ## aPortLetter ## CopyMask =\
        (Pin<PinBit7num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit7num>::mask  == 0x00000080u ? Pin<PinBit7num>::mask  : 0x00u) |\
        (Pin<PinBit6num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit6num>::mask  == 0x00000040u ? Pin<PinBit6num>::mask  : 0x00u) |\
        (Pin<PinBit5num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit5num>::mask  == 0x00000020u ? Pin<PinBit5num>::mask  : 0x00u) |\
        (Pin<PinBit4num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit4num>::mask  == 0x00000010u ? Pin<PinBit4num>::mask  : 0x00u) |\
        (Pin<PinBit3num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit3num>::mask  == 0x00000008u ? Pin<PinBit3num>::mask  : 0x00u) |\
        (Pin<PinBit2num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit2num>::mask  == 0x00000004u ? Pin<PinBit2num>::mask  : 0x00u) |\
        (Pin<PinBit1num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit1num>::mask  == 0x00000002u ? Pin<PinBit1num>::mask  : 0x00u) |\
        (Pin<PinBit0num>::portLetter  == Port ## aPortLetter::letter && Pin<PinBit0num>::mask  == 0x00000001u ? Pin<PinBit0num>::mask  : 0x00u);

        _MOTATE_PH8_CREATE_CLEAR_AND_COPY_MASKS(A);
        _MOTATE_PH8_CREATE_CLEAR_AND_COPY_MASKS(B);
#ifdef PIOC
        _MOTATE_PH8_CREATE_CLEAR_AND_COPY_MASKS(C);
#endif
#ifdef PIOD
        _MOTATE_PH8_CREATE_CLEAR_AND_COPY_MASKS(D);
#endif
    public:
        PinHolder8() {

        };

        void write(uint8_t in_value) {
            uint32_t port_value = 0; // Port<> handles reading the port and setting the masked pins
#define _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter, bitNumber, bitMask) \
    if (PinBit ## bitNumber.maskForPort(Port ## portLetter::letter) &&\
        (PinBit ## bitNumber.mask != (bitMask)) && ((uint32_t)in_value & (bitMask))) {\
        port_value |= PinBit ## bitNumber.mask;\
    }

            // Using direct 0x00000000 notation instead of 1<<x, since the compiler occasionally won't precompile that.
            // Shortcut: ruby -e '(0..7).each() { |x| print "_MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter, %2d, 0x%02x);\\\n" % [7-x, (1<<(7-x))]}'
#define _MOTATE_PH8_PINHOLDER_SETPORT(portLetter) \
    if (port ## portLetter ## ClearMask) {\
        port_value = 0;\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  7, 0x00000080u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  6, 0x00000040u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  5, 0x00000020u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  4, 0x00000010u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  3, 0x00000008u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  2, 0x00000004u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  1, 0x00000002u);\
        _MOTATE_PH8_PINHOLDER_CHECKANDSETPIN(portLetter,  0, 0x00000001u);\
        port_value |= (uint32_t)in_value & port ## portLetter ## CopyMask;\
        port ## portLetter.write(port_value, port ## portLetter ## ClearMask);\
    }

            _MOTATE_PH8_PINHOLDER_SETPORT(A);
            _MOTATE_PH8_PINHOLDER_SETPORT(B);
#ifdef PIOC
            _MOTATE_PH8_PINHOLDER_SETPORT(C);
#endif
#ifdef PIOD
            _MOTATE_PH8_PINHOLDER_SETPORT(D);
#endif
        }
    };
}
#endif /* end of include guard: SAMPINS_H_ONCE */
