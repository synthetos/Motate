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

    // Error codes
    enum TimerErrorCodes {
        kFrequencyUnattainable = -1,
        kInvalidMode = -2,
    };

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
     *   Timer 0-5 map to TC0-TC5 (basic timer/counters)
     *   Timer 6-7 are TC6-TC7 on larger chips
     *
     **************************************************/

    constexpr IRQn_Type tcIRQn(uint8_t timerNum) {
        // TC0-TC5 have contiguous IRQ numbers starting at TC0_IRQn (107)
        return static_cast<IRQn_Type>(TC0_IRQn + timerNum);
    }

    /**************************************************
     *
     * TC Instance Helper - maps timer number to TC base address
     *
     **************************************************/

    constexpr Tc* tcInstance(uint8_t timerNum) {
        switch(timerNum) {
            case 0: return TC0;
            case 1: return TC1;
            case 2: return TC2;
            case 3: return TC3;
            case 4: return TC4;
            case 5: return TC5;
#ifdef TC6
            case 6: return TC6;
#endif
#ifdef TC7
            case 7: return TC7;
#endif
            default: return nullptr;
        }
    }

    /**************************************************
     *
     * GCLK Channel Helper - maps TC number to GCLK peripheral channel
     *
     * SAMD51 GCLK peripheral channel IDs for TC:
     *   TC0, TC1 -> GCLK_TC0_TC1 (9)
     *   TC2, TC3 -> GCLK_TC2_TC3 (26)
     *   TC4, TC5 -> GCLK_TC4_TC5 (30)
     *
     **************************************************/

    constexpr uint8_t tcGclkId(uint8_t timerNum) {
        switch(timerNum) {
            case 0: case 1: return TC0_GCLK_ID;  // 9
            case 2: case 3: return TC2_GCLK_ID;  // 26
            case 4: case 5: return TC4_GCLK_ID;  // 30
#ifdef TC6_GCLK_ID
            case 6: case 7: return TC6_GCLK_ID;
#endif
            default: return 0xFF;
        }
    }

    /**************************************************
     *
     * Timer Template - Full SAMD51 TC Implementation
     *
     * Implements 16-bit TC timers with MFRQ waveform mode
     * where CC[0] acts as the TOP value for periodic interrupts.
     *
     **************************************************/

    template <uint8_t timerNum>
    struct Timer {
        // Cached interrupt cause for ISR
        static volatile uint32_t _interrupt_cause_cached;

        // Track if timer is enabled
        static bool _enabled;

        // Store the prescaler divisor for frequency calculations
        static uint16_t _prescaler_divisor;

        // Store the current top value
        static uint16_t _top_value;

        Timer() { init(); }

        Timer(const TimerMode mode, const uint32_t freq) {
            init();
            setModeAndFrequency(mode, freq);
        }

        void init() {
            _enablePeripheralClock();
        }

        /**
         * Enable peripheral clocks for this TC
         * - Enable APB clock via MCLK
         * - Enable and route GCLK to TC
         */
        static void _enablePeripheralClock() {
            // Enable APB clock - TC0-5 are on different APB buses
            switch(timerNum) {
                case 0: MCLK->APBAMASK.bit.TC0_ = 1; break;
                case 1: MCLK->APBAMASK.bit.TC1_ = 1; break;
                case 2: MCLK->APBBMASK.bit.TC2_ = 1; break;
                case 3: MCLK->APBBMASK.bit.TC3_ = 1; break;
                case 4: MCLK->APBCMASK.bit.TC4_ = 1; break;
                case 5: MCLK->APBCMASK.bit.TC5_ = 1; break;
#ifdef TC6
                case 6: MCLK->APBDMASK.bit.TC6_ = 1; break;
                case 7: MCLK->APBDMASK.bit.TC7_ = 1; break;
#endif
            }

            // Enable GCLK for this TC pair
            // Route GCLK0 (120MHz main clock) to the TC
            uint8_t gclk_id = tcGclkId(timerNum);
            if (gclk_id != 0xFF) {
                GCLK->PCHCTRL[gclk_id].reg = GCLK_PCHCTRL_GEN_GCLK0 | GCLK_PCHCTRL_CHEN;
                while (GCLK->PCHCTRL[gclk_id].bit.CHEN == 0);  // Wait for enable
            }
        }

        static void _disablePeripheralClock() {
            // Disable GCLK for this TC pair
            uint8_t gclk_id = tcGclkId(timerNum);
            if (gclk_id != 0xFF) {
                GCLK->PCHCTRL[gclk_id].reg = 0;
            }

            // Disable APB clock
            switch(timerNum) {
                case 0: MCLK->APBAMASK.bit.TC0_ = 0; break;
                case 1: MCLK->APBAMASK.bit.TC1_ = 0; break;
                case 2: MCLK->APBBMASK.bit.TC2_ = 0; break;
                case 3: MCLK->APBBMASK.bit.TC3_ = 0; break;
                case 4: MCLK->APBCMASK.bit.TC4_ = 0; break;
                case 5: MCLK->APBCMASK.bit.TC5_ = 0; break;
#ifdef TC6
                case 6: MCLK->APBDMASK.bit.TC6_ = 0; break;
                case 7: MCLK->APBDMASK.bit.TC7_ = 0; break;
#endif
            }
        }

        /**
         * Get TC instance pointer
         */
        static Tc* tc() {
            return tcInstance(timerNum);
        }

        /**
         * Wait for synchronization on a specific register
         */
        static void _sync_wait(uint32_t mask) {
            while (tc()->COUNT16.SYNCBUSY.reg & mask);
        }

        /**
         * Software reset the TC
         */
        void _reset() {
            Tc* t = tc();
            t->COUNT16.CTRLA.bit.SWRST = 1;
            _sync_wait(TC_SYNCBUSY_SWRST);
        }

        /**
         * Set mode and frequency
         *
         * Uses MFRQ waveform mode where CC[0] acts as TOP.
         * Counter counts 0 -> CC[0], then wraps and generates MC0 interrupt.
         *
         * Returns: Actual frequency achieved, or negative error code
         */
        int32_t setModeAndFrequency(const TimerMode mode, uint32_t freq) {
            Tc* t = tc();

            // Disable TC first
            if (t->COUNT16.CTRLA.bit.ENABLE) {
                t->COUNT16.CTRLA.bit.ENABLE = 0;
                _sync_wait(TC_SYNCBUSY_ENABLE);
            }

            // Software reset
            _reset();

            // Get master clock frequency (120 MHz on SAMD51)
            uint32_t masterClock = SystemCoreClock;

            // Find appropriate prescaler
            // Prescalers available: 1, 2, 4, 8, 16, 64, 256, 1024
            // Need: (masterClock / prescaler / freq) <= 0xFFFF (65535)

            struct PrescalerInfo {
                uint16_t divisor;
                uint8_t setting;
            };

            static const PrescalerInfo prescalers[] = {
                {1,    TC_CTRLA_PRESCALER_DIV1_Val},
                {2,    TC_CTRLA_PRESCALER_DIV2_Val},
                {4,    TC_CTRLA_PRESCALER_DIV4_Val},
                {8,    TC_CTRLA_PRESCALER_DIV8_Val},
                {16,   TC_CTRLA_PRESCALER_DIV16_Val},
                {64,   TC_CTRLA_PRESCALER_DIV64_Val},
                {256,  TC_CTRLA_PRESCALER_DIV256_Val},
                {1024, TC_CTRLA_PRESCALER_DIV1024_Val},
            };

            uint8_t prescaler_setting = TC_CTRLA_PRESCALER_DIV1_Val;
            _prescaler_divisor = 1;

            for (const auto& ps : prescalers) {
                uint32_t top = masterClock / (ps.divisor * freq);
                if (top > 0 && top <= 0xFFFF) {
                    prescaler_setting = ps.setting;
                    _prescaler_divisor = ps.divisor;
                    break;
                }
            }

            // Calculate top value (CC[0])
            uint32_t top = masterClock / (_prescaler_divisor * freq);
            if (top < 1) top = 1;
            if (top > 0xFFFF) top = 0xFFFF;
            _top_value = (uint16_t)top;

            // Configure TC:
            // - 16-bit mode (default)
            // - Selected prescaler
            // Note: SAMD51 has waveform gen mode in separate WAVE register (not in CTRLA like SAM3/4)
            t->COUNT16.CTRLA.reg = TC_CTRLA_MODE_COUNT16 |
                                   TC_CTRLA_PRESCALER(prescaler_setting);

            // Configure waveform mode: MFRQ = CC[0] as TOP, counter resets on match
            t->COUNT16.WAVE.reg = TC_WAVE_WAVEGEN_MFRQ;

            // Set compare value (this is our TOP)
            t->COUNT16.CC[0].reg = _top_value - 1;  // -1 because counter goes 0 to (CC-1) then wraps
            _sync_wait(TC_SYNCBUSY_CC0);

            // Return actual frequency achieved
            return masterClock / (_prescaler_divisor * _top_value);
        }

        /**
         * Start the timer
         */
        void start() {
            Tc* t = tc();
            t->COUNT16.CTRLA.bit.ENABLE = 1;
            _sync_wait(TC_SYNCBUSY_ENABLE);
            _enabled = true;
        }

        /**
         * Stop the timer
         */
        void stop() {
            Tc* t = tc();
            t->COUNT16.CTRLA.bit.ENABLE = 0;
            _sync_wait(TC_SYNCBUSY_ENABLE);
            _enabled = false;
        }

        /**
         * Get the TOP value (CC[0] in MFRQ mode)
         */
        uint32_t getTopValue() const {
            return tc()->COUNT16.CC[0].reg;
        }

        /**
         * Get current counter value
         */
        uint32_t getValue() {
            Tc* t = tc();
            // Request read sync
            t->COUNT16.CTRLBSET.bit.CMD = TC_CTRLBSET_CMD_READSYNC_Val;
            _sync_wait(TC_SYNCBUSY_CTRLB);
            return t->COUNT16.COUNT.reg;
        }

        /**
         * Set duty cycle for a channel (for PWM mode, not used in MFRQ)
         */
        void setDutyCycleForChannel(const uint8_t channel, const float ratio) {
            if (channel == 0) {
                setExactDutyCycleForChannel(0, (uint32_t)(_top_value * ratio));
            } else if (channel == 1) {
                setExactDutyCycleForChannel(1, (uint32_t)(_top_value * ratio));
            }
        }

        float getDutyCycleForChannel(const uint8_t channel) {
            if (_top_value == 0) return 0.0f;
            return (float)getExactDutyCycleForChannel(channel) / (float)_top_value;
        }

        void setExactDutyCycleForChannel(const uint8_t channel, const uint32_t duty) {
            if (channel == 0) {
                tc()->COUNT16.CC[0].reg = duty;
                _sync_wait(TC_SYNCBUSY_CC0);
            } else if (channel == 1) {
                tc()->COUNT16.CC[1].reg = duty;
                _sync_wait(TC_SYNCBUSY_CC1);
            }
        }

        uint32_t getExactDutyCycleForChannel(const uint8_t channel) {
            if (channel == 0) return tc()->COUNT16.CC[0].reg;
            if (channel == 1) return tc()->COUNT16.CC[1].reg;
            return 0;
        }

        void setOutputOptions(const uint8_t channel, const uint32_t options) {
            // Not implemented for basic TC mode
            (void)channel;
            (void)options;
        }

        void startPWMOutput(const uint8_t channel) {
            (void)channel;
            start();
        }

        void stopPWMOutput(const uint8_t channel) {
            (void)channel;
            stop();
        }

        /**
         * Set interrupts
         *
         * In MFRQ mode, MC0 (match on CC[0]) is the "overflow" since that's when
         * the counter wraps. We map kInterruptOnOverflow to MC0.
         */
        void setInterrupts(const uint32_t interrupts, const int16_t channel = -1) {
            (void)channel;  // We only use MC0 in MFRQ mode

            Tc* t = tc();

            if (interrupts != kInterruptsOff) {
                // Determine NVIC priority
                uint32_t priority = 2;  // Default medium-high
                if (interrupts & kInterruptPriorityHighest) {
                    priority = 0;
                } else if (interrupts & kInterruptPriorityHigh) {
                    priority = 1;
                } else if (interrupts & kInterruptPriorityMedium) {
                    priority = 2;
                } else if (interrupts & kInterruptPriorityLow) {
                    priority = 3;
                } else if (interrupts & kInterruptPriorityLowest) {
                    priority = 4;
                }

                // Clear any pending interrupts
                t->COUNT16.INTFLAG.reg = TC_INTFLAG_MASK;

                // Enable MC0 interrupt for both Overflow and Match
                // (In MFRQ mode, they're the same thing - when counter hits CC[0])
                if (interrupts & (kInterruptOnOverflow | kInterruptOnMatch)) {
                    t->COUNT16.INTENSET.reg = TC_INTENSET_MC0;
                }

                // Software trigger only enables NVIC but no TC interrupt source
                // The interrupt will be triggered via setInterruptPending()

                // Configure and enable NVIC
                IRQn_Type irq = tcIRQn(timerNum);
                NVIC_SetPriority(irq, priority);
                NVIC_ClearPendingIRQ(irq);
                NVIC_EnableIRQ(irq);
            } else {
                // Disable all interrupts
                t->COUNT16.INTENCLR.reg = TC_INTENCLR_MASK;
                NVIC_DisableIRQ(tcIRQn(timerNum));
            }
        }

        /**
         * Trigger a software interrupt
         */
        void setInterruptPending() {
            NVIC_SetPendingIRQ(tcIRQn(timerNum));
        }

        /**
         * Get and clear interrupt cause
         *
         * Returns the cached interrupt flags from the ISR.
         * Maps TC_INTFLAG_MC0 to kInterruptOnOverflow (since in MFRQ mode,
         * MC0 match is when the counter wraps).
         */
        static TimerChannelInterruptOptions getInterruptCause(int16_t &channel) {
            channel = -1;
            uint32_t flags = _interrupt_cause_cached;

            // In MFRQ mode, MC0 is our "overflow" (counter wrapped at CC[0])
            if (flags & TC_INTFLAG_MC0) {
                return kInterruptOnOverflow;
            }

            // OVF in TC is when counter goes past 0xFFFF, which shouldn't happen in MFRQ
            if (flags & TC_INTFLAG_OVF) {
                return kInterruptOnOverflow;
            }

            return kInterruptUnknown;
        }

        // Placeholder for user code - must be specialized
        static void interrupt();
    };

    // Static member definitions
    template <uint8_t timerNum>
    volatile uint32_t Timer<timerNum>::_interrupt_cause_cached = 0;

    template <uint8_t timerNum>
    bool Timer<timerNum>::_enabled = false;

    template <uint8_t timerNum>
    uint16_t Timer<timerNum>::_prescaler_divisor = 1;

    template <uint8_t timerNum>
    uint16_t Timer<timerNum>::_top_value = 0xFFFF;

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
     * Stub implementation for now
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
     * SAMD51 has TC0-TC7 - alias to TimerChannel
     *
     **************************************************/

    template <uint8_t tcNum, uint8_t channelNum>
    struct TCChannel : TimerChannel<tcNum, channelNum> {
        static constexpr bool is_real = true;

        TCChannel(const TimerMode mode = kTimerUpToMatch, const uint32_t freq = 1000)
            : TimerChannel<tcNum, channelNum>(mode, freq) {}
    };

    /**************************************************
     *
     * System Timer (SysTick)
     *
     * For general timing, timeout, and the delay() function
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
