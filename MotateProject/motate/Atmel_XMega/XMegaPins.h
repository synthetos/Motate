/*
  Atmel_XMega/XMegaPins - Library for the Motate system
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

#ifndef XMEGAPINS_H_ONCE
#define XMEGAPINS_H_ONCE

#include <avr/io.h>
#include <util/atomic.h>

#include "xmega.h"
#include "xmega_stdcpp_stubs.h"

namespace Motate {

    // **************************
    // Resume actual Motate code:

	enum PinMode {
		kUnchanged         = 0,
		kOutput            = 1,
		kInput             = 2,
	};
	
	// Note that these are out of order for the same of the compatibility masking.
	// The numbering corresponds to the PINnCTRL bit values on this architecture only.
	enum PinOptions {
		kNormal            = 0,
		kTotem             = 0, // alias
		kPullUp            = 3,

        kBusKeeper         = 1,
		kPullDown          = 2,
		kWiredOr           = 4,
		kDriveHighOnly     = 4, // alias
		kWiredOrPull       = 6,
		kDriveHighPullDown = 6, // alias

        kWiredAnd          = 5,
		kDriveLowOnly      = 5, // alias
		kWiredAndPull      = 7,
		kDriveLowPullUp    = 7, // alias

        kIOInverted        = 1<<4,

        // For use on PWM pins only!
        kPWMPinInverted    = 1<<4
	};

    enum PinInterruptOptions {
        kPinInterruptsOff                = 0,

        kPinInterruptOnChange            = PORT_ISC_BOTHEDGES_gc,

        kPinInterruptOnRisingEdge        = PORT_ISC_RISING_gc,
        kPinInterruptOnFallingEdge       = PORT_ISC_FALLING_gc,

        kPinInterruptOnLowLevel          = PORT_ISC_LEVEL_gc,
        /*kPinInterruptOnHighLevel         = UNSUPPORTED,*/

        kPinInterruptTypeMask            = PORT_ISC_gm,

        /* This turns the IRQ on, but doesn't set the timer to ever trigger it. */
        /*kPinInterruptOnSoftwareTrigger   = 1<<4,*/

        /* Set priority levels here as well: */
        kPinInterruptPriorityHighest     = 1<<5,
        kPinInterruptPriorityHigh        = 1<<5, // same as highest
        kPinInterruptPriorityMedium      = 1<<6,
        kPinInterruptPriorityLow         = 1<<7, // same as lowest
        kPinInterruptPriorityLowest      = 1<<7,

        kPinInterruptPriorityMask        = ((1<<8) - (1<<5))
    };

    typedef uint8_t uintPort_t;

    typedef const uint8_t pin_number;

	template <unsigned char portLetter>
	struct Port8 {
        static const uint8_t letter = portLetter; // NULL stub!
        static PORT_t& port_proxy;

        void setModes(const uintPort_t value, const uintPort_t mask) {
            uint8_t port_value = 0;
            if (mask != 0xff) {
                port_value = (port_proxy).DIR & mask;
            }
            (port_proxy).DIR = port_value | value;
        };
        void setOptions(const uint8_t options, const uintPort_t mask) {
            PORTCFG.MPCMASK = mask; /*Write the configuration to all the masked pins at once.*/
            /* MPCMASK is automatically cleared after any PINnCTRL write completes.*/
            switch (options) {
                case kNormal:
                    /*case kTotem:*/
                    (port_proxy).PIN0CTRL = PORT_OPC_TOTEM_gc;
                    break;
                case kBusKeeper:
                    (port_proxy).PIN0CTRL = PORT_OPC_BUSKEEPER_gc;
                    break;
                case kPullDown:
                    (port_proxy).PIN0CTRL = PORT_OPC_PULLDOWN_gc;
                    break;
                case kPullUp:
                    (port_proxy).PIN0CTRL = PORT_OPC_PULLUP_gc;
                    break;
                case kWiredOr:
                    (port_proxy).PIN0CTRL = PORT_OPC_WIREDOR_gc;
                    break;
                case kWiredAnd:
                    (port_proxy).PIN0CTRL = PORT_OPC_WIREDAND_gc;
                    break;
                case kWiredOrPull:
                    (port_proxy).PIN0CTRL = PORT_OPC_WIREDORPULL_gc;
                    break;
                case kWiredAndPull:
                    (port_proxy).PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;
                    break;
                default:
                    break;
            }
        };
        void set(const uintPort_t value) {
            (port_proxy).OUTSET = value;
        };
        void clear(const uintPort_t value) {
            (port_proxy).OUTCLR = value;
        };
        void write(const uintPort_t value) {
            (port_proxy).OUT = value;
        };
        void write(const uintPort_t value, const uintPort_t mask) {
            uintPort_t port_value = 0;
            if (mask != 0xff) {
                port_value = (port_proxy).OUT & mask;
            }
            (port_proxy).OUT = port_value | value;
        };
        uintPort_t getInputValues(const uintPort_t mask) {
            return (port_proxy).IN & (mask);
        };
        uintPort_t getOutputValues(const uintPort_t mask) {
            return (port_proxy).OUT & (mask);
        };
        void setInterrupts(const uint32_t interrupts, const uintPort_t mask) {
            if (mask != 0) {
                if ((interrupts & kPinInterruptTypeMask) == kPinInterruptsOff) {
                    (port_proxy).INT0MASK |=  mask;
                } else {
                    (port_proxy).INT0MASK &= ~mask;
                }
            }

            // We can change JUST the priority...
            if ((interrupts & kPinInterruptPriorityMask) != kPinInterruptsOff) {
                /* Set interrupt priority */
                if (interrupts & kPinInterruptPriorityMask) {
                    if (interrupts & kPinInterruptPriorityHigh) {
                        (port_proxy).INTCTRL = ((port_proxy).INTCTRL & ~(PORT_INT0LVL_gm)) | PORT_INT0LVL_HI_gc;
                    }
                    else if (interrupts & kPinInterruptPriorityMedium) {
                        (port_proxy).INTCTRL = ((port_proxy).INTCTRL & ~(PORT_INT0LVL_gm)) | PORT_INT0LVL_MED_gc;
                    }
                    else if (interrupts & kPinInterruptPriorityLow) {
                        (port_proxy).INTCTRL = ((port_proxy).INTCTRL & ~(PORT_INT0LVL_gm)) | PORT_INT0LVL_LO_gc;
                    }
                }
            } else {
                // Possible optimization -- check if we're turning the last one off and shut off the priority.
            }
        };
	};

    extern Port8<'A'> portA;
    extern Port8<'B'> portB;
    extern Port8<'C'> portC;
    extern Port8<'D'> portD;
    extern Port8<'E'> portE;
    extern Port8<'F'> portF;


    template<pin_number pinNum>
    struct Pin {
        static const uint8_t number = -1;
        static const uint8_t portLetter = 0;
        static const uint8_t portPin = 0;
        static const uint32_t mask = 0;

        Pin() {};
        Pin(const PinMode type, const uint8_t options = kNormal) {};
        void operator=(const bool value) {};
        operator bool() { return 0; };

        void init(const PinMode type, const uint16_t options = kNormal, const bool fromConstructor=false) {};
        void setMode(const PinMode type, const bool fromConstructor=false) {};
        PinMode getMode() { return kUnchanged; };
        void setOptions(const uint8_t options, const bool fromConstructor=false) {};
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
        ReversePinLookup(const PinMode type, const uint8_t options = kNormal) : Pin<-1>(type, options) {};
    };

    template<pin_number pinNum>
    struct InputPin : Pin<pinNum> {
        InputPin() : Pin<pinNum>(kInput) {};
        InputPin(const uint8_t options) : Pin<pinNum>(kInput, options) {};
        void init(const uint8_t options = kNormal  ) {Pin<pinNum>::init(kInput, options);};
        uint32_t get() {
            return Pin<pinNum>::getInputValue();
        };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. These are intentionally not defined. */
        void init(const PinMode type, const uint8_t options = kNormal);
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        void write(const bool);
    };

    template<pin_number pinNum>
    struct OutputPin : Pin<pinNum> {
        OutputPin() : Pin<pinNum>(kOutput) {};
        OutputPin(const uint8_t options) : Pin<pinNum>(kOutput, options) {};
        void init(const uint8_t options = kNormal) {Pin<pinNum>::init(kOutput, options);};
        uint32_t get() {
            return Pin<pinNum>::getOutputValue();
        };
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. */
        void init(const PinMode type, const uint8_t options = kNormal); /* Intentially not defined. */
    };

    template<uint8_t portChar, uint8_t portPin>
    struct _IRQPin {
        static void interrupt();
    };

    // All of the pins on the XMega can be an interrupt pins
    // but we create these objects to share the interface with other architectures.
    template<pin_number pinNum>
    struct IRQPin : Pin<pinNum>, _IRQPin<Pin<pinNum>::portLetter, Pin<pinNum>::portPin> {
        IRQPin() : Pin<pinNum>(kInput) {};
        IRQPin(const uint8_t options) : Pin<pinNum>(kInput, options) {};
        void init(const uint8_t options = kNormal  ) {Pin<pinNum>::init(kInput, options);};

        static const bool is_real = true;
        static const bool knows_which_pin = false;
    };

    template<pin_number pinNum>
    constexpr const bool IsIRQPin() { return IRQPin<pinNum>::is_real; };

    template<pin_number gpioPinNumber>
    using IsGPIOIRQOrNull = typename std::enable_if<true>::type;

    template<uint8_t portChar, uint8_t portPin>
    using LookupIRQPin = IRQPin< ReversePinLookup<portChar, portPin>::number >;


#define MOTATE_PIN_INTERRUPT(number) \
    template<> void Motate::_IRQPin<Motate::Pin<number>::portLetter, Motate::Pin<number>::portPin>::interrupt() __attribute__((signal)); \
    template<> void Motate::_IRQPin<Motate::Pin<number>::portLetter, Motate::Pin<number>::portPin>::interrupt()

    template<pin_number pinNum>
    struct UARTTxPin {
        static const bool is_real = false;
    };

    template <pin_number pinNum>
    constexpr const bool IsUARTTxPin() { return UARTTxPin<pinNum>::is_real; };

#define _MAKE_MOTATE_UART_TX_PIN(uartNum_, registerChar, registerPin)\
    template<>\
    struct UARTTxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        UARTTxPin() : ReversePinLookup<registerChar, registerPin>(kOutput) {};\
        static const uint8_t uartNum = uartNum_; \
        static const bool is_real = true;\
    };
        
    
    template<pin_number pinNum>
    struct UARTRxPin {
            static const bool is_real = false;
    };

    template <pin_number pinNum>
    constexpr const bool IsUARTRxPin() { return UARTRxPin<pinNum>::is_real; };

#define _MAKE_MOTATE_UART_RX_PIN(uartNum_, registerChar, registerPin)\
    template<>\
    struct UARTRxPin< ReversePinLookup<registerChar, registerPin>::number > : ReversePinLookup<registerChar, registerPin> {\
        UARTRxPin() : ReversePinLookup<registerChar, registerPin>(kInput) {};\
        static const uint8_t uartNum = uartNum_; \
        static const bool is_real = true;\
    };



#define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerChar, registerPin)\
    template<>\
    struct Pin<pinNum> {\
    private: /* Lock the copy contructor.*/\
        Pin(const Pin<pinNum>&){};\
    public:\
        static const uint8_t number = pinNum;\
        static const uint8_t portLetter = (uint8_t) registerChar;\
        static const uint8_t portPin = (uint8_t) registerPin;\
        static const uint8_t mask = (1 << registerPin);\
            \
        Pin(){};\
        Pin(const PinMode type, const uint8_t options = kNormal) {\
            init(type, options);\
        };\
        void operator=(const bool value) { write(value); };\
        operator bool() { return (get() != 0); };\
        \
        void init(const PinMode type, const uint8_t options = kNormal) {\
            setMode(type);\
            setOptions(options);\
        };\
        void setMode(const PinMode type) {\
            switch (type) {\
                case kOutput:\
                    (PORT ## registerLetter).DIR |= mask;\
                    break;\
                case kInput:\
                    (PORT ## registerLetter).DIR &= ~mask;\
                    break;\
                default:\
                    break;\
            }\
        };\
        PinMode getMode() {\
            return ((PORT ## registerLetter).DIR |= mask) ? kOutput : kInput;\
        };\
        void setOptions(const uint8_t options) {\
            if (options & kIOInverted) {\
                (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_INVEN_bm;\
            }\
            switch (options & ~kIOInverted) {\
                case kNormal:\
                /*case kTotem:*/\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_TOTEM_gc;\
                    break;\
                case kBusKeeper:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_BUSKEEPER_gc;\
                    break;\
                case kPullDown:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_PULLDOWN_gc;\
                    break;\
                case kPullUp:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_PULLUP_gc;\
                    break;\
                case kWiredOr:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDOR_gc;\
                    break;\
                case kWiredAnd:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDAND_gc;\
                    break;\
                case kWiredOrPull:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDORPULL_gc;\
                    break;\
                case kWiredAndPull:\
                    (PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDANDPULL_gc;\
                    break;\
                default:\
                    break;\
            }\
        };\
        uint8_t getOptions() {\
            return (PORT ## registerLetter).PIN ## registerPin ## CTRL;\
        };\
        void set() {\
            (PORT ## registerLetter).OUTSET = mask;\
        };\
        void clear() {\
            (PORT ## registerLetter).OUTCLR = mask;\
        };\
        void write(bool value) {\
            if (!value)\
                clear();\
            else\
                set();\
        };\
        void toggle()  {\
            (PORT ## registerLetter).OUTTGL = mask;\
        };\
        uint8_t get() { /* WARNING: This will fail if the input buffer is disabled for this pin!!! Use getOutputValue() instead. */\
            return ((PORT ## registerLetter).IN & mask);\
        };\
        uint8_t getInputValue() {\
            return ((PORT ## registerLetter).IN & mask);\
        };\
        uint8_t getOutputValue() {\
            return ((PORT ## registerLetter).OUT & mask);\
        };\
        void setInterrupts(const uint32_t interrupts) {\
            port ## registerLetter.setInterrupts(interrupts, mask);\
        };\
        bool isNull() { return false; };\
        static uint8_t maskForPort(const uint8_t otherPortLetter) {\
            return portLetter == otherPortLetter ? mask : 0x00;\
        };\
    };\
    typedef Pin<pinNum> Pin ## pinNum;\
    typedef InputPin<pinNum> InputPin ## pinNum;\
    typedef OutputPin<pinNum> OutputPin ## pinNum;\
    template<>\
    struct ReversePinLookup<registerChar, registerPin> : Pin<pinNum> {\
        ReversePinLookup() {};\
        ReversePinLookup(const PinMode type, const PinOptions options = kNormal) : Pin<pinNum>(type, options) {};\
    };\


    static const uint32_t kDefaultPWMFrequency = 1000;
    template<int8_t pinNum>
        struct PWMOutputPin : Pin<pinNum> {
            PWMOutputPin() : Pin<pinNum>(kOutput) {};
            PWMOutputPin(const PinOptions options, const uint32_t freq = kDefaultPWMFrequency) : Pin<pinNum>(kOutput, options) {};
            PWMOutputPin(const uint32_t freq) : Pin<pinNum>(kOutput, kNormal) {};
            void setFrequency(const uint32_t freq) {};
            void operator=(const float value) { write(value); };
            void write(const float value) { Pin<pinNum>::write(value >= 0.5); };
            void writeRaw(const uint16_t duty) { Pin<pinNum>::write(duty >= 50); };
            uint16_t getTopValue() { return 100; };
            bool canPWM() { return false; };

            /*Override these to pick up new methods */

        private: /* Make these private to catch them early. */
            /* These are intentially not defined. */
            void init(const PinMode type, const PinOptions options = kNormal);

            /* WARNING: Covariant return types! */
            bool get();
            operator bool();
        };

#define _MAKE_MOTATE_PWM_PIN(registerChar, registerPin, timerOrPWM, channel, invertedByDefault)\
    template<>\
    struct PWMOutputPin< ReversePinLookup<registerChar, registerPin>::number > : Pin< ReversePinLookup<registerChar, registerPin>::number >, timerOrPWM {\
        static const pin_number pinNum = ReversePinLookup<registerChar, registerPin>::number;\
        PWMOutputPin() :\
            Pin<pinNum>(kOutput), timerOrPWM(Motate::kTimerUpToMatch, kDefaultPWMFrequency)\
            {pwmpin_init(kNormal);};\
        \
        PWMOutputPin(const PinOptions options, const uint32_t freq = kDefaultPWMFrequency) :\
            Pin<pinNum>(kOutput), timerOrPWM(Motate::kTimerUpToMatch, kDefaultPWMFrequency)\
            {pwmpin_init(options);};\
        \
        PWMOutputPin(const uint32_t freq) :\
            Pin<pinNum>(kOutput), timerOrPWM(Motate::kTimerUpToMatch, freq)\
            {pwmpin_init(kNormal);};\
        \
        void pwmpin_init(const PinOptions options) {\
            timerOrPWM::startPWMOutput(channel);\
            timerOrPWM::start();\
        };\
        \
        void setFrequency(const uint32_t freq) {\
            timerOrPWM::setModeAndFrequency(Motate::kTimerUpToMatch, freq);\
            timerOrPWM::start();\
        };\
        void operator=(const float value) { write(value); };\
        void write(const float value) {\
            uint16_t duty = getTopValue() * value;\
            timerOrPWM::setExactDutyCycleForChannel(channel, duty);\
        };\
        void writeRaw(const uint16_t duty) {\
            timerOrPWM::setExactDutyCycleForChannel(channel, duty);\
        };\
        bool canPWM() { return true; };\
        /*Override these to pick up new methods */\
    private: /* Make these private to catch them early. */\
        /* These are intentially not defined. */\
        void init(const PinMode type, const PinOptions options = kNormal);\
        /* WARNING: Covariant return types! */\
        bool get();\
        operator bool();\
    };


	typedef Pin<-1> NullPin;
	static NullPin nullPin;

} // end namespace Motate


// Note: We end the namespace before including in case the included file need to include
//   another Motate file. If it does include another Motate file, we end up with
//   Motate::Motate::* definitions and weird compiler errors.
#include "motate_pin_assignments.h"

#if 0 // PinHolder is currently unused and unmaintained
// If anyone wishes to use it, then ping us on GitHub with an issue.
namespace Motate {

	// PinHolder - virtual ports
	template<uint8_t PinBit7num, uint8_t PinBit6num, uint8_t PinBit5num = -1, uint8_t PinBit4num = -1, uint8_t PinBit3num = -1, uint8_t PinBit2num = -1, uint8_t PinBit1num = -1, uint8_t PinBit0num = -1>
	class PinHolder8 {

		static Pin<PinBit7num> PinBit7;
		static Pin<PinBit6num> PinBit6;
		static Pin<PinBit5num> PinBit5;
		static Pin<PinBit4num> PinBit4;
		static Pin<PinBit3num> PinBit3;
		static Pin<PinBit2num> PinBit2;
		static Pin<PinBit1num> PinBit1;
		static Pin<PinBit0num> PinBit0;

#define _MOTATE_CREATE_CLEAR_AND_COPY_MASKS(aPortLetter) \
		static const uint8_t port ## aPortLetter ## ClearMask =\
			(Pin<PinBit7num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit7num>::mask : 0x00) |\
			(Pin<PinBit6num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit6num>::mask : 0x00) |\
			(Pin<PinBit5num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit5num>::mask : 0x00) |\
			(Pin<PinBit4num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit4num>::mask : 0x00) |\
			(Pin<PinBit3num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit3num>::mask : 0x00) |\
			(Pin<PinBit2num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit2num>::mask : 0x00) |\
			(Pin<PinBit1num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit1num>::mask : 0x00) |\
			(Pin<PinBit0num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit0num>::mask : 0x00);\
\
		static const uint8_t port ## aPortLetter ## CopyMask =\
			(Pin<PinBit7num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit7num>::mask == (1 << 7 ) \
				? Pin<PinBit7num>::mask : 0x00) |\
			(Pin<PinBit6num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit6num>::mask == (1 << 6 )\
				? Pin<PinBit6num>::mask : 0x00) |\
			(Pin<PinBit5num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit5num>::mask == (1 << 5 )\
				? Pin<PinBit5num>::mask : 0x00) |\
			(Pin<PinBit4num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit4num>::mask == (1 << 4 )\
				? Pin<PinBit4num>::mask : 0x00) |\
			(Pin<PinBit3num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit3num>::mask == (1 << 3 )\
				? Pin<PinBit3num>::mask : 0x00) |\
			(Pin<PinBit2num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit2num>::mask == (1 << 2 )\
				? Pin<PinBit2num>::mask : 0x00) |\
			(Pin<PinBit1num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit1num>::mask == (1 << 1 )\
				? Pin<PinBit1num>::mask : 0x00) |\
			(Pin<PinBit0num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit0num>::mask == (1 << 0 )\
				? Pin<PinBit0num>::mask : 0x00);

		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(A);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(B);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(C);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(D);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(E);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(F);
				
	public:
		PinHolder8() {
			
		};
		
		void write(const uint8_t in_value, const uint8_t mask = 0xff) {
			uint8_t port_value;
#define _MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, bitNumber, bitMask) \
			if (PinBit ## bitNumber.maskForPort(port ## portLetter.letter) &&\
					(PinBit ## bitNumber.mask != (bitMask)) && (in_value & mask & (bitMask))) {\
				port_value |= PinBit ## bitNumber.mask;\
			}
			
#define _MOTATE_PINHOLDER_SETPORT(portLetter) \
			if (port ## portLetter ## ClearMask && mask) {\
				port_value = 0x00;\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 7, 0b10000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 6, 0b01000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 5, 0b00100000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 4, 0b00010000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 3, 0b00001000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 2, 0b00000100);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 1, 0b00000010);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 0, 0b00000001);\
				port_value |= in_value & port ## portLetter ## CopyMask;\
				port ## portLetter.write(port_value, ~(mask & port ## portLetter ## ClearMask));\
			}
			
			_MOTATE_PINHOLDER_SETPORT(A);
			_MOTATE_PINHOLDER_SETPORT(B);
			_MOTATE_PINHOLDER_SETPORT(C);
			_MOTATE_PINHOLDER_SETPORT(D);
			_MOTATE_PINHOLDER_SETPORT(E);
			_MOTATE_PINHOLDER_SETPORT(F);
		}
		
	};
} // namespace Motate
#endif // pinholder disabled


#endif /* end of include guard: XMEGAPINS_H_ONCE */