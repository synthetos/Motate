/*
  MotatePins.hpp - Library for the Motate system
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

#ifndef MOTATEPINS_H_ONCE
#define MOTATEPINS_H_ONCE

#include <cinttypes>
#include <algorithm> // for std::conditional

/* After some setup, we call the processor-specific bits, then we have the
 * any-processor parts.
 *
 * The processor specific parts MUST define:
 *
 * enum PinMode : PinMode_t
 *  * kUnchanged
 *  * kOutput
 *  * kInput
 *  * <any processor specific values necessary here>
 *
 * enum PinOptions : PinOptions_t
 *  * kNormal (alias kTotem)
 *  * kPullUp
 *  * kPullDown (if the processor suports it)
 *  * kWiredAnd (alias kDriveLowOnly, if the processor suports it)
 *  * kDriveLowOnly (if supported)
 *  * kWiredAndPull (=kWiredAnd|kPullUp, alias kDriveLowPullUp)
 *  * kDeglitch (if supported)
 *  * kDebounce (if supported)
 *  * kStartHigh
 *  * kStartLow
 *  * kPWMPinInverted
 *
 * enum PinInterruptOptions : PinInterruptOptions_t (the supported of the following):
 *  * kPinInterruptsOff
 *  * kPinInterruptOnChange
 *  * kPinInterruptOnRisingEdge
 *  * kPinInterruptOnFallingEdge
 *  * kPinInterruptOnLowLevel
 *  * kPinInterruptOnHighLevel
 *  * kPinInterruptOnSoftwareTrigger
 *  And all of the following, some may be aliases:
 *  * kPinInterruptPriorityHighest
 *  * kPinInterruptPriorityHigh
 *  * kPinInterruptPriorityMedium
 *  * kPinInterruptPriorityLow
 *  * kPinInterruptPriorityLowest
 *
 * typedef uintPort_t (used for Port masking, so uint32_t for 32-bit arms)
 *
 * macro _MAKE_MOTATE_PIN
 *
 * template<int16_t pinNum> constexpr const bool IsIRQPin();
 * macro MOTATE_PIN_INTERRUPT(number) (will be placed in namespace Motate {...} )
 *
 */

namespace Motate {
    // forward declare a few things that are defined in processor-specific sections:
    typedef const uint32_t PinMode_t;
    typedef const uint32_t PinOptions_t;
    typedef const uint32_t PinInterruptOptions_t;

    // pin_number is always this:
    typedef const int16_t pin_number;


#pragma mark Pin / ReversePinLookup
    /**************************************************
     *
     * BASIC PINS: Pin / ReversePinLookup
     *
     **************************************************/


    // Pin is the external interface, but only RealPins are real, and so real pins
    // are specialized to have RelPins as their parent.
    // IOW: This exact "non-specialization" is the do-nothing non-real pin implementation.
    template<pin_number pinNum>
    struct Pin {
        static const int16_t number = -1;
        static const uint8_t portLetter = 0;
        static const uint32_t mask = 0;

        Pin() {};
        Pin(const PinMode_t type, const PinOptions_t options=0) {};
        void operator=(const bool value) {};
        operator bool() { return 0; };

        void init(const PinMode_t type, const PinOptions_t options=0, const bool fromConstructor=false) {};
        void setMode(const PinMode_t type, const bool fromConstructor=false) {};
        PinMode_t getMode() { return 0; };
        void setOptions(const PinOptions_t options, const bool fromConstructor=false) {};
        PinOptions_t getOptions() { return 0; };
        void set() {};
        void clear() {};
        void write(const bool value) {};
        void toggle() {};
        uint8_t get() { return 0; };               // We don't know the port width yet,
        uint8_t getInputValue() { return 0; };     // so we'll assume we can expand
        uint8_t getOutputValue() { return 0; };    // from uint8_t if necessary.
        static constexpr uint8_t maskForPort(const uint8_t otherPortLetter) { return 0; };
        static constexpr bool isNull() { return true; };
        static constexpr bool isIRQ() { return false; };
    };


    template<uint8_t portChar, uint8_t portPin>
    struct ReversePinLookup : Pin<-1> {
        ReversePinLookup() {};
        ReversePinLookup(const PinMode_t type, const PinOptions_t options=0) : Pin<-1>(type, options) {};
    };

} // namespace Motate

#pragma mark Processor-specific includes
/**************************************************
 *
 * PROCESSOR INCLUDES
 *
 **************************************************/

#ifdef __AVR_XMEGA__

#include <Atmel_XMega/XMegaPins.h>

#else

#ifdef __AVR__
#include <Atmel_avr/AvrPins.h>
#endif

#endif

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
#include <SamPins.h>
#endif

#if defined(__SAM4E8E__) || defined(__SAM4E16E__) || defined(__SAM4E8C__) || defined(__SAM4E16C__)
#include <SamPins.h>
#endif

#if defined(__SAMS70N19__) || defined(__SAMS70N20__) || defined(__SAMS70N21__)
#include <SamPins.h>
#endif

#if defined(__KL05Z__)
#include <Freescale_klxx/KL05ZPins.h>
#endif


namespace Motate {

#pragma mark RealPin
    /**************************************************
     *
     * BASIC PINS: RealPin
     *
     **************************************************/

    // These are the real pins, and are the parent of a Pin<>, with a real
    // PortHardware backing it up.
    template<uint8_t portChar, uint8_t portPin>
    struct RealPin {
    private: /* Lock the copy contructor.*/
        RealPin(const RealPin<portChar, portPin>&){};

    public:

        static constexpr uint32_t mask = (1u << portPin);
        PortHardware<portChar> port;

        RealPin() {};
        RealPin(const PinMode type, const PinOptions_t options = kNormal) {
            init(type, options, /*fromConstructor=*/true);
        };
        void operator=(const bool value) { write(value); };
        operator bool() { return (get() != 0); };

        void init(const PinMode type, const PinOptions_t options = kNormal, const bool fromConstructor=false) {
            setMode(type, fromConstructor);
            setOptions(options, fromConstructor);
        };
        void setMode(const PinMode type, const bool fromConstructor=false) {
            port.setModes(type, mask);
        };
        PinMode getMode() {
            return port.getMode();
        };
        void setOptions(const PinOptions_t options, const bool fromConstructor=false) {
            port.setOptions(options, mask);
        };
        PinOptions_t getOptions() {
            return port.getOptions(mask);
        };
        void set() {
            port.set(mask);
        };
        void clear() {
            port.clear(mask);
        };
        void write(const bool value) {
            if (!value)
                clear();
            else
                set();
        };
        void toggle()  {
            port.toggle(mask); /*Enable writing thru ODSR*/
        };
        uint32_t get() { /* WARNING: This will fail if the peripheral clock is disabled for this pin!!! Use getOutputValue() instead. */
            return port.getInputValues(mask);
        };
        uint32_t getInputValue() {
            return port.getInputValues(mask);
        };
        uint32_t getOutputValue() {
            return port.getOutputValues(mask);
        };
        static constexpr uint32_t maskForPort(const uint8_t otherPortLetter) {
            return portChar == otherPortLetter ? mask : 0x00u;
        };
        static constexpr bool isNull() { return false; };
        static constexpr bool isIRQ() { return true; };
    };


#pragma mark InputPin / OutputPin
    /**************************************************
     *
     * PIN ALIASES: InputPin / OutputPin
     *
     **************************************************/


    template<pin_number pinNum>
    struct InputPin : Pin<pinNum> {
        InputPin() : Pin<pinNum>(kInput) {};
        InputPin(const PinOptions_t options) : Pin<pinNum>(kInput, options) {};
        void init(const PinOptions_t options = kNormal  ) {Pin<pinNum>::init(kInput, options);};
        uint32_t get() {
            return Pin<pinNum>::getInputValue();
        };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. These are intentionally not defined. */
        void init(const PinMode type, const PinOptions_t options = kNormal);
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        void write(const bool);
    };

    template<pin_number pinNum>
    struct OutputPin : Pin<pinNum> {
        OutputPin() : Pin<pinNum>(kOutput) {};
        OutputPin(const PinOptions_t options) : Pin<pinNum>(kOutput, options) {};
        void init(const PinOptions_t options = kNormal) {Pin<pinNum>::init(kOutput, options);};
        uint32_t get() {
            return Pin<pinNum>::getOutputValue();
        };
        void operator=(const bool value) { Pin<pinNum>::write(value); };
        /*Override these to pick up new methods */
        operator bool() { return (get() != 0); };
    private: /* Make these private to catch them early. */
        void init(const PinMode type, const PinOptions_t options = kNormal); /* Intentially not defined. */
    };


#pragma mark IRQPin / LookupIRQPin
    /**************************************************
     *
     * PIN CHANGE INTERRUPT PINS: IRQPin / LookupIRQPin
     *
     * IRQ pins occupy resources, so we extend the Pin so all Pins don't unduly
     * occupy resources.

     * On some processors, some pins aren't IRQ-capable. Also, some IRQ pins don't
     * have real pins behind them, so we make this resourceless "fake" IRQPin
     * definition and use SFINAE to decide which to use automatically.
     *
     * REQUIRES: template<int16_t pinNum> constexpr const bool IsIRQPin()
     *
     **************************************************/

    template<pin_number pinNum, class _enabled = void>
    struct IRQPin : Pin<pinNum> {
        static_assert(Pin<pinNum>::isNull() || !IsIRQPin<pinNum>(), "Fake IRQPin should only be on a null pin or non-IRQ pin.");

        IRQPin() : Pin<pinNum>(kInput) {};
        IRQPin(const PinOptions_t options) : Pin<pinNum>(kInput, options) {};
        IRQPin(const std::function<void(void)> &&_interrupt) : Pin<pinNum>(kInput) {};
        IRQPin(const PinOptions_t options, const std::function<void(void)> &&_interrupt, const uint32_t interrupt_settings = kPinInterruptOnChange|kPinInterruptPriorityMedium) : Pin<pinNum>(kInput, options) {};

        void init(const PinOptions_t options = kNormal  ) {Pin<pinNum>::init(kInput, options);};

        static const bool is_real = false;

        void setInterrupts(const uint32_t interrupts) {};
        static void interrupt() __attribute__ (( weak, unused ));
        void setInterruptHandler(std::function<void(void)> &&handler) {};
    };

    // This is the REAL IRQPin defintition, and is used only wehn the pin is really defined.
    template<pin_number pinNum>
    struct IRQPin<pinNum, typename std::enable_if<IsIRQPin<pinNum>() && !Pin<pinNum>::isNull()>::type> : Pin<pinNum>, _pinChangeInterrupt {
        static_assert(!Pin<pinNum>::isNull(), "Cannot have a null pin be an IRQ");

        IRQPin(const IRQPin&) = delete; // delete copy
        IRQPin(IRQPin&&) = delete; // delete move

        IRQPin()
        : Pin<pinNum>(kInput),
        _pinChangeInterrupt(Pin<pinNum>::mask, interrupt, PortHardware<Pin<pinNum>::portLetter>::_firstInterrupt)
        {
            setInterrupts(kPinInterruptOnChange|kPinInterruptPriorityMedium);
        };

        IRQPin(const PinOptions_t options,
               const uint32_t interrupt_settings = kPinInterruptOnChange|kPinInterruptPriorityMedium
               )
        : Pin<pinNum>(kInput, options),
        _pinChangeInterrupt(Pin<pinNum>::mask, interrupt, PortHardware<Pin<pinNum>::portLetter>::_firstInterrupt)
        {
            setInterrupts(interrupt_settings);
        };

        IRQPin(std::function<void(void)> &&_interrupt,
               const uint32_t interrupt_settings = kPinInterruptOnChange|kPinInterruptPriorityMedium
               )
        : Pin<pinNum>(kInput),
        _pinChangeInterrupt(Pin<pinNum>::mask, std::move(_interrupt), PortHardware<Pin<pinNum>::portLetter>::_firstInterrupt)
        {
            setInterrupts(interrupt_settings);
        };

        IRQPin(const PinOptions_t options,
               std::function<void(void)> &&_interrupt,
               const uint32_t interrupt_settings = kPinInterruptOnChange|kPinInterruptPriorityMedium
               )
        : Pin<pinNum>(kInput, options),
        _pinChangeInterrupt(Pin<pinNum>::mask, std::move(_interrupt), PortHardware<Pin<pinNum>::portLetter>::_firstInterrupt)
        {
            setInterrupts(interrupt_settings);
        };

        static const bool is_real = true; // Yeah, they ALL can be interrupt pins (in hardware).

        using Pin<pinNum>::port;
        using Pin<pinNum>::mask;

        void init(const PinOptions_t options = kNormal  ) {Pin<pinNum>::init(kInput, options);};

        // This sets the interrupt *settings*
        void setInterrupts(const uint32_t interrupts) {
            port.setInterrupts(interrupts, mask);
        };

        // There are two way to setup interrupt callbacks. DON'T USE BOTH!

        // Interface option 1: create this function (use macro MOTATE_PIN_INTERRUPT)
        static void interrupt() __attribute__ (( weak ));

        // Inferface option 2: call this function with your closure or function pointer
        void setInterruptHandler(std::function<void(void)> &&handler) {
            _pinChangeInterrupt::setInterrupt(std::move(handler)); // enable interrupts and set the priority
        };
        void setInterruptHandler(const std::function<void(void)> &handler) {
            _pinChangeInterrupt::setInterrupt(handler); // enable interrupts and set the priority
        };
    };

    template<uint8_t portChar, uint8_t portPin>
    using LookupIRQPin = IRQPin< ReversePinLookup<portChar, portPin>::number >;


#pragma mark PWMOutputPin / RealPWMOutputPin / PWMLikeOutputPin
    /**************************************************
     *
     * PWM ("fake" analog) output pins: PWMOutputPin / RealPWMOutputPin / PWMLikeOutputPin
     *
     * PWM pins use a PWM-capable hardware Timer, and configure it to drive the output pin.
     *
     * We have an additional criteria: We need both to have an "available" PWMable pin,
     *  IOW, associated to hardware with a pin, AND have that pin assigned a numbered Pin<>.
     *
     * _GetAvailablePWMOrAlike<n> returns the correct ype to use, based on if the Pin<n>
     *   is real, AND we have an AvailablePWMOutputPin<n> (which is what _MAKE_MOTATE_PWM_PIN()
     *   should create.
     *
     **************************************************/

    static const uint32_t kDefaultPWMFrequency = 1000;

    template<pin_number pinNum, typename timerOrPWM>
    struct RealPWMOutputPin : Pin<pinNum>, timerOrPWM {

        RealPWMOutputPin(const PinMode pinMode) : Pin<pinNum>(pinMode, pinMode), timerOrPWM(kTimerUpToMatch, kDefaultPWMFrequency) {};
        RealPWMOutputPin(const PinMode pinMode, const PinOptions_t options, const uint32_t freq) : Pin<pinNum>(pinMode, options), timerOrPWM(kTimerUpToMatch, freq) {};
//        RealPWMOutputPin(const PinMode pinMode, const uint32_t freq) : Pin<pinNum>(pinMode, kNormal), timerOrPWM(kTimerUpToMatch, freq) {};
        bool canPWM() { return true; };

        void pwmpin_init(const TimerChannelOutputOptions options) {
            timerOrPWM::setOutputOptions(options);
            timerOrPWM::start();
        };
        void setFrequency(const uint32_t freq) {
            timerOrPWM::setModeAndFrequency(kTimerUpToMatch, freq);
            timerOrPWM::start();
        };
        operator float() { return timerOrPWM::getDutyCycle(); };
        operator uint32_t() { return timerOrPWM::getExactDutyCycle(); };
        void operator=(const float value) { write(value); };
        void write(const float value) {
            uint16_t duty = timerOrPWM::getTopValue() * value;
            if (duty < 2)
                timerOrPWM::stopPWMOutput();
            else
                timerOrPWM::startPWMOutput();
            timerOrPWM::setExactDutyCycle(duty);
        };
        void writeRaw(const uint16_t duty) {
            if (duty < 2)
                timerOrPWM::stopPWMOutput();
            else
                timerOrPWM::startPWMOutput();
            timerOrPWM::setExactDutyCycle(duty);
        };
        void setInterrupts(const uint32_t interrupts) {
            timerOrPWM::setInterrupts(interrupts);
        };
        auto getInterruptCause() {
            int16_t temp;
            return timerOrPWM::getInterruptCause(temp);
        };
        /*Override these to pick up new methods */
    private: /* Make these private to catch them early. */
        /* These are intentially not defined. */
        void init(const PinMode type, const PinOptions_t options = kNormal);
        /* WARNING: Covariant return types! */
        bool get();
        operator bool();
    };

    template <pin_number pinNum>
    struct AvailablePWMOutputPin {
        // empty shell to fill for actual available PWMPins
        static constexpr bool _isAvailable() { return false; }
    };

    // PWMLikeOutputPin is the PWMOutputPin interface on a normal output pin.
    // This is for cases where you want it to act like a non-PWM capable
    // PWMOutputPin, but there actually IS a PWMOutputPin that you explictly
    // don't want to use.
    template<pin_number pinNum>
    struct PWMLikeOutputPin : Pin<pinNum> {
        PWMLikeOutputPin() : Pin<pinNum>(kOutput) {};
        PWMLikeOutputPin(const PinOptions_t options, const uint32_t freq) : Pin<pinNum>(kOutput, options) {};
//        PWMLikeOutputPin(const uint32_t freq) : Pin<pinNum>(kOutput, kNormal) {};
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
        void init(const PinMode type, const PinOptions_t options = kNormal);

        /* WARNING: Covariant return types! */
        bool get();
        operator bool();
    };


    
    template <pin_number pinNum>
    using _GetAvailablePWMOrAlike = typename std::conditional<
    (AvailablePWMOutputPin<pinNum>::_isAvailable()),
        /* True: */  AvailablePWMOutputPin<pinNum>,
        /* False: */ PWMLikeOutputPin<pinNum>
    >::type;

    template<pin_number pinNum>
    struct PWMOutputPin : _GetAvailablePWMOrAlike<pinNum> {
        typedef _GetAvailablePWMOrAlike<pinNum> _pin_parent;
        PWMOutputPin() : _pin_parent() {};
        PWMOutputPin(const PinOptions_t options, const uint32_t freq) : _pin_parent(options, freq) {};
//        PWMOutputPin(const uint32_t freq) : _pin_parent(kOutput, kNormal) {};

        using _GetAvailablePWMOrAlike<pinNum>::operator=;
        using _GetAvailablePWMOrAlike<pinNum>::setFrequency;
    };


#pragma mark SPIChipSelectPin / SPIMISOPin / SPIMOSIPin / SPISCKPin
    /**************************************************
     *
     * SPI PIN METADATA and wiring: SPIChipSelectPin / SPIMISOPin / SPIMOSIPin / SPISCKPin
     *
     * SPI uses these pins to wire up pin muxing and handle tests if pins can support SPI.
     *
     * REQUIRES: _MAKE_MOTATE_SPI_CS_PIN
     *           _MAKE_MOTATE_SPI_MISO_PIN
     *           _MAKE_MOTATE_SPI_MOSI_PIN
     *           _MAKE_MOTATE_SPI_SCK_PIN
     *
     **************************************************/


    template<pin_number pinNum>
    struct SPIChipSelectPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t spiNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template<pin_number pinNum>
    constexpr bool IsSPICSPin() { return SPIChipSelectPin<pinNum>::is_real; };


    template<pin_number pinNum>
    struct SPIMISOPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t spiNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsSPIMISOPin() { return SPIMISOPin<pinNum>::is_real; };

    template<pin_number pinNum>
    struct SPIMOSIPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t spiNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsSPIMOSIPin() { return SPIMOSIPin<pinNum>::is_real; };

    template<pin_number pinNum>
    struct SPISCKPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t spiNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsSPISCKPin() { return SPISCKPin<pinNum>::is_real; };


#pragma mark UARTTxPin / UARTRxPin / UARTRTSPin / UARTCTSPin
    /**************************************************
     *
     * UART/USART PIN METADATA and wiring: UARTTxPin / UARTRxPin / UARTRTSPin / UARTCTSPin
     *
     * SPI uses these pins to wire up pin muxing and handle tests if pins can support SPI.
     *
     * REQUIRES: _MAKE_MOTATE_UART_TX_PIN
     *           _MAKE_MOTATE_UART_RX_PIN
     *           _MAKE_MOTATE_UART_RTS_PIN
     *           _MAKE_MOTATE_UART_CTS_PIN
     *
     **************************************************/


    template<pin_number pinNum>
    struct UARTTxPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t uartNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsUARTTxPin() { return UARTTxPin<pinNum>::is_real; };


    template<pin_number pinNum>
    struct UARTRxPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t uartNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsUARTRxPin() { return UARTRxPin<pinNum>::is_real; };



    template<pin_number pinNum>
    struct UARTRTSPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t uartNum = -1; // yes, we assigned -1 to a uint8_t
    };
    template <pin_number pinNum>
    constexpr bool IsUARTRTSPin() { return UARTRTSPin<pinNum>::is_real; };



    template<pin_number pinNum>
    struct UARTCTSPin {
        static constexpr bool is_real = false;
        static constexpr uint8_t uartNum = -1; // yes, we assigned -1 to a uint8_t
    };

    template <pin_number pinNum>
    constexpr bool IsUARTCTSPin() { return UARTCTSPin<pinNum>::is_real; };


#pragma mark ClockOutputPin
    /**************************************************
     *
     * Clock Output PIN METADATA and wiring: CLKOutPin
     *
     * REQUIRES: _MAKE_MOTATE_CLOCK_OUTPUT_PIN
     *
     **************************************************/


    template<pin_number pinNum>
    struct ClockOutputPin {
        static constexpr bool is_real = false;
        ClockOutputPin(const uint32_t target_freq) {};
    };
    template <pin_number pinNum>
    constexpr bool IsClockOutputPin() { return ClockOutputPin<pinNum>::is_real; };

} // namespace motate


// Note: We end the namespace before including in case the included file need to include
//   another Motate file. If it does include another Motate file, we end up with
//   Motate::Motate::* definitions and weird compiler errors.
#include "motate_pin_assignments.h"


#endif /* end of include guard: MOTATEPINS_H_ONCE */
