/*
 utility/SamdTimers.h - Library for the Motate system
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

#ifndef SAMDTIMERS_H_ONCE
#define SAMDTIMERS_H_ONCE

#include "sam.h"
#include "SamdCommon.h"

#include <functional>
#include <type_traits>

namespace Motate {

    /**************************************************
     *
     * Timer Mode Enums
     *
     * SAMD51 has TCC (Timer/Counter for Control) and TC (Timer/Counter) peripherals
     * TCC: Advanced features, up to 6 channels, deadtime, pattern generation
     * TC: Basic 8/16/32-bit counter, 2 channels
     *
     **************************************************/

    enum TimerMode {
        /* InputCapture mode is UNIMPLEMENTED */
        kTimerInputCapture         = 0,

        kTimerUp                   = 1,
        /* Alias for PWM: */
        kPWMLeftAligned            = kTimerUp,

        kTimerUpDownToMatch        = 2,
        /* Alias for PWM: */
        kPWMCenterAligned          = kTimerUpDownToMatch,

        kTimerUpToMatch            = 3,
        /* For PWM, alias kTimerUpToMatch as: */
        kPWMLeftAlignedAtBottom    = kTimerUpToMatch,
    };

    /* This is an internal enum (but the values are platform-specific):
     * This is what is passed to the internal setModeAndFrequency() function.
     */
    enum TimerChannelOutputOptions {
        kOutputDisconnected        = 0,

        kToggleOnMatch             = 1 << 0,
        kClearOnMatch              = 1 << 1,
        kSetOnMatch                = 1 << 2,

        // (Clear vs Set) vs (on Match vs on Overflow) gives you different polarities.
        kClearOnOverflow           = 1 << 3,
        kSetOnOverflow             = 1 << 4,

        // Aliases for the above.
        kPWMOn                     = kClearOnMatch | kSetOnOverflow,
        kPWMOnInverted             = kSetOnMatch | kClearOnOverflow,
    };

    /* This is an internal enum (but the values are platform-specific):
     * This is what is passed to setInterrupts() and returned from getInterruptCause()
     */
    enum TimerInterruptOptions {
        kInterruptsOff             = 0,
        /* Alias for "off" to make more sense
         when returned from getInterruptCause(). */
        kInterruptUnknown          = 0,

        /* Overflow means count went past top (or through zero in count-down). */
        kInterruptOnOverflow       = 1 << 0,
        /* Match is when the count matches the channel top value. */
        kInterruptOnMatch          = 1 << 1,

        /* This turns the IRQ on, but doesn't set the timer to ever trigger it. */
        kInterruptOnSoftwareTrigger = 1 << 4,

        /* Priority levels */
        kInterruptPriorityHighest  = 1 << 5,
        kInterruptPriorityHigh     = 1 << 6,
        kInterruptPriorityMedium   = 1 << 7,
        kInterruptPriorityLow      = 1 << 8,
        kInterruptPriorityLowest   = 1 << 9,
    };

    // Alias for compatibility with other platforms
    using TimerChannelInterruptOptions = TimerInterruptOptions;

    // Timer number type (same as SAM common)
    typedef const uint8_t timer_number;

    /**************************************************
     *
     * Delay Function
     *
     **************************************************/

    extern void delay(uint32_t ms);


    /**************************************************
     *
     * Timer IRQ Helper - maps timer number to IRQ
     *
     * SAMD51 timer numbering for g2core:
     *   Timer 0-7 map to TC0-TC7 (basic timer/counters)
     *   Timer 8+ reserved for TCC if needed
     *
     **************************************************/

    constexpr IRQn_Type tcIRQn(uint8_t timerNum) {
        // TC0-TC7 have contiguous IRQ numbers starting at TC0_IRQn (107)
        return static_cast<IRQn_Type>(TC0_IRQn + timerNum);
    }

    /**************************************************
     *
     * Timer Template (Stub for compilation)
     *
     * Full implementation will be added in later phases.
     * Currently provides software interrupt support needed by stepper.cpp
     *
     **************************************************/

    template <uint8_t timerNum>
    struct Timer {
        static uint32_t _interrupt_cause_cached;

        Timer() {}
        Timer(const TimerMode mode, const uint32_t freq) {
            // TODO: Actually configure the timer
            (void)mode;
            (void)freq;
        }

        static void _enablePeripheralClock() {}
        static void _disablePeripheralClock() {}

        void setModeAndFrequency(const TimerMode mode, const uint32_t freq) {
            (void)mode;
            (void)freq;
        }
        void start() {}
        void stop() {}
        uint32_t getTopValue() const { return 0; }

        void setDutyCycleForChannel(const uint8_t channel, const float ratio) {
            (void)channel;
            (void)ratio;
        }
        float getDutyCycleForChannel(const uint8_t channel) {
            (void)channel;
            return 0.0f;
        }
        void setExactDutyCycleForChannel(const uint8_t channel, const uint32_t duty) {
            (void)channel;
            (void)duty;
        }
        uint32_t getExactDutyCycleForChannel(const uint8_t channel) {
            (void)channel;
            return 0;
        }

        void setOutputOptions(const uint8_t channel, const uint32_t options) {
            (void)channel;
            (void)options;
        }
        void startPWMOutput(const uint8_t channel) {
            (void)channel;
        }
        void stopPWMOutput(const uint8_t channel) {
            (void)channel;
        }

        void setInterrupts(const uint32_t interrupts, const int16_t channel = -1) {
            (void)channel;
            if (interrupts != kInterruptsOff) {
                NVIC_EnableIRQ(tcIRQn(timerNum));
            } else {
                NVIC_DisableIRQ(tcIRQn(timerNum));
            }
        }

        // Trigger a software interrupt for this timer
        void setInterruptPending() {
            NVIC_SetPendingIRQ(tcIRQn(timerNum));
        }

        static TimerChannelInterruptOptions getInterruptCause(int16_t &channel) {
            channel = -1;
            // Return overflow for now - full implementation will check actual status
            return kInterruptOnOverflow;
        }

        // Placeholder for user code.
        static void interrupt();
    };

    // Static member definition template
    template <uint8_t timerNum>
    uint32_t Timer<timerNum>::_interrupt_cause_cached = 0;

    /**************************************************
     *
     * TimerChannel: Wrapper for Timer with specific channel
     *
     **************************************************/

    template<uint8_t timerNum, uint8_t channelNum>
    struct TimerChannel : Timer<timerNum> {
        TimerChannel() : Timer<timerNum>{} {}
        TimerChannel(const TimerMode mode, const uint32_t freq) : Timer<timerNum>{mode, freq} {}

        void setDutyCycle(const float ratio) {
            Timer<timerNum>::setDutyCycleForChannel(channelNum, ratio);
        }

        float getDutyCycle() {
            return Timer<timerNum>::getDutyCycleForChannel(channelNum);
        }

        void setExactDutyCycle(const uint32_t absolute) {
            Timer<timerNum>::setExactDutyCycleForChannel(channelNum, absolute);
        }

        uint32_t getExactDutyCycle() {
            return Timer<timerNum>::getExactDutyCycleForChannel(channelNum);
        }

        void setOutputOptions(const uint32_t options) {
            Timer<timerNum>::setOutputOptions(channelNum, options);
        }

        void startPWMOutput() {
            Timer<timerNum>::startPWMOutput(channelNum);
        }

        void stopPWMOutput() {
            Timer<timerNum>::stopPWMOutput(channelNum);
        }

        void setInterrupts(const uint32_t interrupts) {
            Timer<timerNum>::setInterrupts(interrupts, channelNum);
        }

        TimerChannelInterruptOptions getInterruptCause(int16_t &channel) {
            channel = 1;
            return Timer<timerNum>::getInterruptCause(channel);
        }

        TimerChannelInterruptOptions getInterruptCause() {
            int16_t channel = 1;
            return Timer<timerNum>::getInterruptCause(channel);
        }

        // Placeholder for user code.
        static void interrupt();
    };

    /**************************************************
     *
     * TCC Timer Channel (Timer/Counter for Control)
     *
     * SAMD51 has TCC0-TCC4 with multiple channels each
     *
     **************************************************/

    template <uint8_t tccNum, uint8_t channelNum>
    struct TCCChannel {
        static constexpr bool is_real = false;

        TCCChannel(const TimerMode mode = kTimerUpToMatch, const uint32_t freq = 1000) {}
        void setModeAndFrequency(const TimerMode mode, const uint32_t freq) {}
        void start() {}
        void stop() {}
        uint32_t getTopValue() const { return 0; }
        float getDutyCycle() const { return 0.0f; }
        uint32_t getExactDutyCycle() const { return 0; }
        void setDutyCycle(const float value) {}
        void setExactDutyCycle(const uint32_t duty) {}
        void setOutputOptions(const TimerChannelOutputOptions options) {}
        void startPWMOutput() {}
        void stopPWMOutput() {}
        void setInterrupts(const uint32_t interrupts) {}
        TimerInterruptOptions getInterruptCause(int16_t &channel) { return kInterruptsOff; }
    };

    /**************************************************
     *
     * TC Timer Channel (Basic Timer/Counter)
     *
     * SAMD51 has TC0-TC7
     *
     **************************************************/

    template <uint8_t tcNum, uint8_t channelNum>
    struct TCChannel {
        static constexpr bool is_real = false;

        TCChannel(const TimerMode mode = kTimerUpToMatch, const uint32_t freq = 1000) {}
        void setModeAndFrequency(const TimerMode mode, const uint32_t freq) {}
        void start() {}
        void stop() {}
        uint32_t getTopValue() const { return 0; }
        float getDutyCycle() const { return 0.0f; }
        uint32_t getExactDutyCycle() const { return 0; }
        void setDutyCycle(const float value) {}
        void setExactDutyCycle(const uint32_t duty) {}
        void setOutputOptions(const TimerChannelOutputOptions options) {}
        void startPWMOutput() {}
        void stopPWMOutput() {}
        void setInterrupts(const uint32_t interrupts) {}
        TimerInterruptOptions getInterruptCause(int16_t &channel) { return kInterruptsOff; }
    };

    /**************************************************
     *
     * System Timer (SysTick)
     *
     * For general timing, timeout, and the delay() function
     *
     **************************************************/

    /**************************************************
     *
     * SysTickEvent, Timer<SysTickTimerNum> SysTickTimer
     *
     * SysTickTimer and related:
     *  Timer<SysTickTimerNum> is the special Timer for Systick.
     *  SysTickTimer is the global singleton to access it.
     *  SysTickEvent is the class to use to register a new event to occur every Tick.
     *
     **************************************************/

    struct SysTickEvent {
        const std::function<void(void)> callback;
        SysTickEvent *next;
    };

    static const timer_number SysTickTimerNum = 0xFF;

    template<>
    struct Timer<SysTickTimerNum> {
        static volatile uint32_t _motateTickCount;
        SysTickEvent *firstEvent = nullptr;

        Timer() { init(); };
        Timer(const TimerMode mode, const uint32_t freq) { init(); };

        void init() {
            _motateTickCount = 0;
            // Configure SysTick for 1ms tick at 120MHz (SAMD51 default)
            SysTick_Config(SystemCoreClock / 1000);
        }

        // Return the current value of the counter
        uint32_t getValue() {
            return _motateTickCount;
        }

        void _increment() {
            _motateTickCount++;
        }

        void registerEvent(SysTickEvent *new_event) {
            if (firstEvent == nullptr) {
                firstEvent = new_event;
                return;
            }
            SysTickEvent *event = firstEvent;
            if (new_event == event) { return; }
            while (event->next != nullptr) {
                event = event->next;
                if (new_event == event) { return; }
            }
            event->next = new_event;
            new_event->next = nullptr;
        }

        void unregisterEvent(SysTickEvent *new_event) {
            if (firstEvent == new_event) {
                firstEvent = firstEvent->next;
                return;
            }
            SysTickEvent *event = firstEvent;
            while (event->next != nullptr) {
                if (event->next == new_event) {
                    event->next = event->next->next;
                    return;
                }
                event = event->next;
            }
        }

        void _handleEvents() {
            SysTickEvent *event = firstEvent;
            while (event != nullptr) {
                event->callback();
                event = event->next;
            }
        }

        // Placeholder for user code.
        static void interrupt() __attribute__ ((weak));
    };

    extern Timer<SysTickTimerNum> SysTickTimer;

    /**************************************************
     *
     * Timeout: Simple non-blocking (polling) timeout class.
     *
     **************************************************/

    struct Timeout {
        uint32_t start_, delay_;
        Timeout() : start_{0}, delay_{0} {};

        bool isSet() {
            return (start_ > 0);
        }

        bool isPast() {
            if (!isSet()) {
                return false;
            }
            return ((SysTickTimer.getValue() - start_) > delay_);
        };

        void set(uint32_t delay, bool only_if_lower = false) {
            if (only_if_lower && (delay_ - (SysTickTimer.getValue() - start_) <= delay)) {
                return; // we would extend the timer
            }
            start_ = SysTickTimer.getValue();
            delay_ = delay;
        };

        void clear() {
            start_ = 0;
            delay_ = 0;
        }
    };

    /**************************************************
     *
     * Watchdog Timer
     *
     **************************************************/

    struct WatchDogTimerHardware {
        static void enable(uint32_t timeout_ms = 1000) {
            // TODO: Implement WDT enable for SAMD51
            // For now, leave disabled
        }

        static void disable() {
            // SAMD51 WDT can only be disabled by reset or
            // by setting ALWAYSON=0 in fuses before enabling
            // For now, just don't enable it
            WDT->CTRLA.bit.ENABLE = 0;
            while (WDT->SYNCBUSY.bit.ENABLE) {}
        }

        static void reset() {
            WDT->CLEAR.reg = WDT_CLEAR_CLEAR_KEY;
        }
    };

    extern WatchDogTimerHardware WatchDogTimer;

} // namespace Motate

#endif /* end of include guard: SAMDTIMERS_H_ONCE */
