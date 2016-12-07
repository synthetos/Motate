/*
 utility/SamTimers.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2016 Robert Giseburt

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

#ifndef SAMTIMERS_H_ONCE
#define SAMTIMERS_H_ONCE

#include "sam.h"
#include "SamCommon.h"
#ifndef PWM_PTCR_TXTEN
#include "SamDMA.h"
#endif
#include <functional> // for std::function and related
#include <type_traits> // for std::extent and std::alignment_of

/* Sam hardware has two types of timer: "Timers" and "PWMTimers"
 *
 * Timers:
 *
 * Sam hardware timers have three channels each. Each channel is actually an
 * independent timer, so we have a little nomenclature clash.
 *
 * Sam Timer != Motate::Timer!!!
 *
 * A Sam Timer CHANNEL is actually the portion that a Motate::Timer controls
 * direcly. Each SAM CHANNEL has two Motate:Timers (A and B).
 * (Actually, the Quadrature Decoder and Block Control can mix them up some,
 * but we ignore that.)
 * So, for the Sam, we have to maintain the same interface, and treat each
 * channel as an independent timer.
 *
 *
 * PWMTimers:
 *
 * For compatibility and transparency with Timers, we use the same TimerModes,
 * even through they actually use bitmaps for Timer registers.
 *
 * Timers have more modes than PWM Timers, and more interrupts.
 * We return kInvalidMode for the ones that don't map, except that we treat "Up"
 * and "UpToMatch" both as "LeftAligned," and "UpDown" and "UpDownToMatch"
 * as "CenterAligned."
 *
 * Consequently, you can use kPWMLeftAligned and kPWMCenterAligned as valid modes
 * on a Timer.
 */


namespace Motate {
#pragma mark Enums, typedefs, etc.
    /**************************************************
     *
     * Enums and typedefs
     *
     **************************************************/

    enum TimerMode {
        /* InputCapture mode (WAVE = 0) */
        kTimerInputCapture         = 0,
        /* InputCapture mode (WAVE = 0), counts up to RC */
        kTimerInputCaptureToMatch  = 0 | TC_CMR_CPCTRG,

        /* Waveform select, Up to 0xFFFFFFFF */
        kTimerUp            = TC_CMR_WAVE | TC_CMR_WAVSEL_UP,
        /* Waveform select, Up to TOP (RC) */
        kTimerUpToTop       = TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC,
        /* Keep the "ToMatch" naming for compatibility */
        kTimerUpToMatch     = TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC,
        /* For PWM, we'll alias kTimerUpToMatch as: */
        kPWMLeftAligned     = TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC,
        /* Waveform select, Up to 0xFFFFFFFF, then Down */
        kTimerUpDown        = TC_CMR_WAVE | TC_CMR_WAVSEL_UPDOWN,
        /* Waveform select, Up to TOP (RC), then Down */
        kTimerUpDownToTop   = TC_CMR_WAVE | TC_CMR_WAVSEL_UPDOWN_RC,
        /* Keep the "ToMatch" naming for compatibility */
        kTimerUpDownToMatch = TC_CMR_WAVE | TC_CMR_WAVSEL_UPDOWN_RC,
        /* For PWM, we'll alias kTimerUpDownToMatch as: */
        kPWMCenterAligned     = kTimerUpDownToMatch,
    };

    enum TimerSyncMode {
        kTimerSyncManually = 0,
        kTimerSyncDMA      = 1
    };

    /* We're trading acronyms for verbose CamelCase. Dubious. */
    enum TimerChannelOutputOptions {
        kOutputDisconnected = 0,

        kToggleOnMatch     = 1<<0,
        kClearOnMatch      = 1<<1,
        kSetOnMatch        = 1<<2,

        kToggleOnOverflow  = 1<<3,
        kClearOnOverflow   = 1<<4,
        kSetOnOverflow     = 1<<5,


        /* Aliases for use with PWM */
        kPWMOn             = kClearOnMatch | kSetOnOverflow,
        kPWMOnInverted     = kSetOnMatch | kClearOnOverflow,
    };

    /* We use TC_CMR_EEVT_XC0 in the above to allow TIOB to be an output.
     * The defualt is for it to be the input for ExternalEvent.
     * By setting it to XC0, we allow it to be an output.
     */

    enum TimerChannelInterruptOptions {
        kInterruptsOff              = 0,
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        kInterruptUnknown           = 0,

        kInterruptOnMatch          = 1<<1,
        /* Note: Interrupt on overflow could be a match C as well. */
        kInterruptOnOverflow        = 1<<3,

        /* This turns the IRQ on, but doesn't set the timer to ever trigger it. */
        kInterruptOnSoftwareTrigger = 1<<4,

        /* Set priority levels here as well: */
        kInterruptPriorityHighest   = 1<<5,
        kInterruptPriorityHigh      = 1<<6,
        kInterruptPriorityMedium    = 1<<7,
        kInterruptPriorityLow       = 1<<8,
        kInterruptPriorityLowest    = 1<<9,
    };

    enum TimerErrorCodes {
        kFrequencyUnattainable = -1,
        kInvalidMode = -2,
    };

    enum PWMTimerClockOptions {
        kPWMClockPrescalerOnly = 0,
        kPWMClockPrescaleAndDivA = 1,
        kPWMClockPrescaleAndDivB = 2,
    };

    typedef const uint8_t timer_number;


#pragma mark Timer<n>
    /**************************************************
     *
     * TIMERS: Timer<n>
     *
     **************************************************/

    template <uint8_t timerNum>
    struct Timer {
        // For external inspection
        static constexpr uint8_t peripheral_num = (timerNum < 3) ? 0 : (timerNum < 6) ? 1 : 2;
        static constexpr uint8_t timer_num = timerNum;

        // This type of timer can't sync
        static constexpr uint8_t can_sync = false;
        static constexpr uint8_t sync_master = false;

        // Try to catch invalid Timer usage ASAP
#ifndef TC1
        static_assert(timerNum!=3, "Timer<3> cannot be used on this processor.");
        static_assert(timerNum!=4, "Timer<4> cannot be used on this processor.");
        static_assert(timerNum!=5, "Timer<5> cannot be used on this processor.");
#endif
#ifndef TC2
        static_assert(timerNum!=6, "Timer<6> cannot be used on this processor.");
        static_assert(timerNum!=7, "Timer<7> cannot be used on this processor.");
        static_assert(timerNum!=8, "Timer<8> cannot be used on this processor.");
#endif
#ifndef TC3
        static_assert(timerNum!=9, "Timer<9> cannot be used on this processor.");
        static_assert(timerNum!=10, "Timer<10> cannot be used on this processor.");
        static_assert(timerNum!=11, "Timer<11> cannot be used on this processor.");
#endif

        // NOTE: Notice! The *pointers* are const, not the *values*.
        static constexpr Tc * const tc()
        {
            if (timerNum < 3) { return TC0; }
#ifdef TC1
            else
            if (timerNum < 6) { return TC1; }
#endif
#ifdef TC2
            else
            if (timerNum < 9) { return TC2; }
#endif
#ifdef TC3
            else { return TC3; }
#endif
        };
        static constexpr TcChannel * const tcChan()
        {
            if (timerNum < 3) { return TC0->TC_CHANNEL + timerNum; }
#ifdef TC1
            else
            if (timerNum < 6) { return TC1->TC_CHANNEL + (timerNum - 3); }
#endif
#ifdef TC2
            else
            if (timerNum < 9) { return TC2->TC_CHANNEL + (timerNum - 6); }
#endif
#ifdef TC3
            else { return TC3->TC_CHANNEL + (timerNum - 9); }
#endif
        };
        static constexpr const uint32_t peripheralId()
        {
            switch (timerNum) {
                case 0: return ID_TC0;
                case 1: return ID_TC1;
                case 2: return ID_TC2;
#ifdef TC1
                case 3: return ID_TC3;
                case 4: return ID_TC4;
                case 5: return ID_TC5;
#endif
#ifdef TC2
                case 6: return ID_TC6;
                case 7: return ID_TC7;
                case 8: return ID_TC8;
#endif
#ifdef TC3
                case 9: return ID_TC9;
                case 10: return ID_TC10;
                case 11: return ID_TC11;
#endif
            }
        };
        static constexpr const IRQn_Type tcIRQ()
        {
            switch (timerNum) {
                case 0: return TC0_IRQn;
                case 1: return TC1_IRQn;
                case 2: return TC2_IRQn;
#ifdef TC1
                case 3: return TC3_IRQn;
                case 4: return TC4_IRQn;
                case 5: return TC5_IRQn;
#endif
#ifdef TC2
                case 6: return TC6_IRQn;
                case 7: return TC7_IRQn;
                case 8: return TC8_IRQn;
#endif
#ifdef TC3
                case 9: return TC9_IRQn;
                case 10: return TC10_IRQn;
                case 11: return TC11_IRQn;
#endif
            }
        };
        static volatile uint32_t _interrupt_cause_cached;

        static const bool has_channel_interrupts = false;

        /********************************************************************
         **                          WARNING                                **
         ** WARNING: Sam channels (tcChan) DO NOT map to Motate Channels!?! **
         **                          WARNING           (u been warned)      **
         *********************************************************************/

        Timer() { init(); };
        Timer(const TimerMode mode, const uint32_t freq) {
            init();
            setModeAndFrequency(mode, freq);
        };

        void init() {
            /* Unlock this thing */
            unlock();
        }

#if !defined(TC_WPMR_WPKEY_PASSWD)
        static const uint32_t TC_WPMR_WPKEY_PASSWD = TC_WPMR_WPKEY(0x54494D);
#endif

        void unlock() const {
            tc()->TC_WPMR = TC_WPMR_WPKEY_PASSWD;
        }

        /* WHOA!! Only do this if you know what you're doing!! */
        void lock() const {
            tc()->TC_WPMR = TC_WPMR_WPEN | TC_WPMR_WPKEY_PASSWD;
        }

        // Set the mode and frequency.
        // Returns: The actual frequency that was used, or kFrequencyUnattainable
        // freq is not const since we may "change" it
        int32_t setModeAndFrequency(const TimerMode mode, uint32_t freq) {
            /* Prepare to be able to make changes: */
            /*   Disable TC clock */
            tcChan()->TC_CCR = TC_CCR_CLKDIS ;
            /*   Disable interrupts */
            tcChan()->TC_IDR = 0xFFFFFFFF ;
            /*   Clear status register */
            tcChan()->TC_SR;

            SamCommon::enablePeripheralClock(peripheralId());

            if (mode == kTimerUpDownToMatch || mode == kTimerUpDown)
                freq /= 2;

            /* Setup clock "prescaler" */
            /* Divisors: TC1: 2, TC2: 8, TC3: 32, TC4: 128, TC5: ???! */
            /* For now, we don't support TC5. */

            // Grab the base clock frequency, which is different on 4E and S70 (peripheral clock) than 3X (Master clock).
            uint32_t masterClock = SamCommon::getPeripheralClockFreq();

            // Store the divisor temporarily, to avoid looking it up again...
            uint32_t divisor = 2; // sane default of 2

#if (SAMV71 || SAMV70 || SAME70 || SAMS70)
            // The SAM*70 chips have the first prescaler option set to PCK6
#else
            // TC1 = MCK/2
            if (freq > ((masterClock / 2) / 0x10000) && freq < (masterClock / 2)) {
                /*  Set mode */
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK1;
                divisor = 2;
            } else
#endif

                // TC2 = MCK/8
            if (freq > ((masterClock / 8) / 0x10000) && freq < (masterClock / 8)) {
                /*  Set mode */
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK2;
                divisor = 8;

                // TC3 = MCK/32
            } else if (freq > ((masterClock / 32) / 0x10000) && freq < (masterClock / 32)) {
                /*  Set mode */
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK3;
                divisor = 32;

                // TC4 = MCK/128
            } else if (freq > ((masterClock / 128) / 0x10000) && freq < (masterClock / 128)) {
                /*  Set mode */
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK4;
                divisor = 128;

                // Nothing fit! Hmm...
            } else {
                // PUNT! For now, just guess TC1.
                /*  Set mode */
#if (SAMV71 || SAMV70 || SAME70 || SAMS70)
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK2;
#else
                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_WAVSEL_Msk | TC_CMR_TCCLKS_Msk)) |
                    mode | TC_CMR_TCCLKS_TIMER_CLOCK1;
#endif
                return kFrequencyUnattainable;
            }

            //TODO: Add ability to select external clocks... -RG

            // Extra mile, set the actual frequency, but only if we're going to RC.
            if (mode == kTimerInputCaptureToMatch
                || mode == kTimerUpToMatch
                || mode == kTimerUpDownToMatch) {

                int32_t newTop = masterClock/(divisor*freq);
                setTop(newTop);

                // Determine and return the new frequency.
                return masterClock/(divisor*newTop);
            }

            // Optimization -- we can't use RC for much when we're not using it,
            //  so, instead of looking up if we're using it or not, just set it to
            //  0xFFFF when we're not using it.
            setTop(0xFFFF);

            // Determine and return the new frequency.
            return masterClock/(divisor*0xFFFF);
        };

        // Set the TOP value for modes that use it.
        // WARNING: No sanity checking is done to verify that you are, indeed, in a mode that uses it.
        void setTop(const uint32_t topValue) {
            tcChan()->TC_RC = topValue;
        };

        // Here we want to get what the TOP value is. Is the mode is one that resets on RC, then RC is the TOP.
        // Otherwise, TOP is 0xFFFF. In order to see if TOP is RC, we need to look at the CPCTRG (RC Compare
        // Trigger Enable) bit of the CMR (Channel Mode Register). Note that this bit position is the same for
        // waveform or Capture mode, even though the Datasheet seems to obfuscate this fact.
        uint32_t getTopValue() {
            return tcChan()->TC_CMR & TC_CMR_CPCTRG ? tcChan()->TC_RC : 0xFFFF;
        };

        // Return the current value of the counter. This is a fleeting thing...
        uint32_t getValue() {
            return tcChan()->TC_CV;
        }

        void start() {
            tcChan()->TC_CCR = TC_CCR_CLKEN | TC_CCR_SWTRG;
        };

        void stop() {
            tcChan()->TC_CCR = TC_CCR_CLKDIS;
        };

        void stopOnMatch() {
            tcChan()->TC_CMR = TC_CMR_CPCSTOP;
        };

        // Channel-specific functions. These are Motate channels, but they happen to line-up.
        // Motate channel A = Sam channel A.
        // Motate channel B = Sam channel B.

        // Specify the duty cycle as a value from 0.0 .. 1.0;
        void setDutyCycleForChannel(const uint8_t channel, const float ratio) {
            setExactDutyCycleForChannel(channel, getTopValue() * ratio);
        };
        float getDutyCycleForChannel(const uint8_t channel) {
            return (float)getExactDutyCycleForChannel(channel) / getTopValue();
        };

        // Specify channel A/B duty cycle as a integer value from 0 .. TOP.
        // TOP in this case is either RC_RC or 0xFFFF.
        void setExactDutyCycleForChannel(const uint8_t channel, const uint32_t absolute) {
            if (channel == 0) {
                tcChan()->TC_RA = absolute;
            } else if (channel == 1) {
                tcChan()->TC_RB = absolute;
            }
        };

        uint32_t getExactDutyCycleForChannel(const uint8_t channel) {
            if (channel == 0) {
                return tcChan()->TC_RA;
            } else /*if (channel == 1)*/ {
                return tcChan()->TC_RB;
            }
        };

        void setOutputOptions(const uint8_t channel, const uint32_t options) {
            uint32_t bitfield = 0;
            if (channel == 0) {
                if (options & kToggleOnMatch) {
                    bitfield |= TC_CMR_ACPA_TOGGLE;
                }
                if (options & kClearOnMatch) {
                    bitfield |= TC_CMR_ACPA_CLEAR;
                }
                if (options & kSetOnMatch) {
                    bitfield |= TC_CMR_ACPA_SET;
                }
                if (options & kToggleOnOverflow) {
                    bitfield |= TC_CMR_ACPC_TOGGLE;
                }
                if (options & kClearOnOverflow) {
                    bitfield |= TC_CMR_ACPC_CLEAR;
                }
                if (options & kSetOnOverflow) {
                    bitfield |= TC_CMR_ACPC_SET;
                }

                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(
                                                         TC_CMR_ACPA_Msk |
                                                         TC_CMR_ACPC_Msk
                                                         )) | bitfield;
            } else if (channel == 1) {

                if (options & kToggleOnMatch) {
                    bitfield |= TC_CMR_BCPB_TOGGLE;
                }
                if (options & kClearOnMatch) {
                    bitfield |= TC_CMR_BCPB_CLEAR;
                }
                if (options & kSetOnMatch) {
                    bitfield |= TC_CMR_BCPB_SET;
                }
                if (options & kToggleOnOverflow) {
                    bitfield |= TC_CMR_BCPC_TOGGLE;
                }
                if (options & kClearOnOverflow) {
                    bitfield |= TC_CMR_BCPC_CLEAR;
                }
                if (options & kSetOnOverflow) {
                    bitfield |= TC_CMR_BCPC_SET;
                }

                tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(
                                                         TC_CMR_BCPB_Msk |
                                                         TC_CMR_BCPC_Msk
                                                         )) | bitfield | TC_CMR_EEVT_XC0;
            }
        };


        // These two start the waveform. We try to be as fast as we can.
        // ASSUMPTION: We stopped it with the corresponding function.
        // ASSUMPTION: The pin is not and was not in Toggle mode.
        void startPWMOutput(const uint8_t channel) {
            if (channel == 0) {
                if ((tcChan()->TC_CMR & TC_CMR_ACPA_Msk) == TC_CMR_ACPA_SET)
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_ACPC_Msk)) | TC_CMR_ACPC_CLEAR;
                else
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_ACPC_Msk)) | TC_CMR_ACPC_SET;
            } else if (channel == 1) {
                if ((tcChan()->TC_CMR & TC_CMR_BCPB_Msk) == TC_CMR_BCPB_SET)
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_BCPC_Msk)) | TC_CMR_BCPC_CLEAR;
                else
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_BCPC_Msk)) | TC_CMR_BCPC_SET;
            }
        };

        // These are special function for stopping output waveforms.
        // This is different from stopping the timer, which kill both channels and
        // all interrupts. This simply stops the pin output from changing, and is used
        // to set the duty cycle to 0.

        // ASSUMPTION: The pin is not in Toggle mode.
        void stopPWMOutput(const uint8_t channel) {
            if (channel == 0) {
                if ((tcChan()->TC_CMR & TC_CMR_ACPA_Msk) == TC_CMR_ACPA_SET)
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_ACPC_Msk)) | TC_CMR_ACPC_SET;
                else
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_ACPC_Msk)) | TC_CMR_ACPC_CLEAR;
            } else if (channel == 1) {
                if ((tcChan()->TC_CMR & TC_CMR_BCPB_Msk) == TC_CMR_BCPB_SET)
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_BCPC_Msk)) | TC_CMR_BCPC_SET;
                else
                    tcChan()->TC_CMR = (tcChan()->TC_CMR & ~(TC_CMR_BCPC_Msk)) | TC_CMR_BCPC_CLEAR;
            }
        };

        void setInterrupts(const uint32_t interrupts, const int16_t channel = -1) {
            if (interrupts != kInterruptsOff) {
                // We intentionally don't clear the interrupts so multiple calls
                // will add to the interrupts, not replace them.

                if (interrupts & kInterruptOnOverflow) {
                    // Check to see if we're overflowing on C. See getTopValue() description.
                    if (tcChan()->TC_CMR & TC_CMR_CPCTRG) {
                        tcChan()->TC_IER = TC_IER_CPCS; // RC Compare
                    } else {
                        tcChan()->TC_IER = TC_IER_COVFS; // Counter Overflow
                    }
                }
                if (interrupts & kInterruptOnMatch && channel == 0) {
                    tcChan()->TC_IER = TC_IER_CPAS; // RA Compare
                }
                if (interrupts & kInterruptOnMatch && channel == 1) {
                    tcChan()->TC_IER = TC_IER_CPBS; // RB Compare
                }

                /* Set interrupt priority */
                if (interrupts & kInterruptPriorityHighest) {
                    NVIC_SetPriority(tcIRQ(), 0);
                }
                else if (interrupts & kInterruptPriorityHigh) {
                    NVIC_SetPriority(tcIRQ(), 3);
                }
                else if (interrupts & kInterruptPriorityMedium) {
                    NVIC_SetPriority(tcIRQ(), 7);
                }
                else if (interrupts & kInterruptPriorityLow) {
                    NVIC_SetPriority(tcIRQ(), 11);
                }
                else if (interrupts & kInterruptPriorityLowest) {
                    NVIC_SetPriority(tcIRQ(), 15);
                }

                NVIC_EnableIRQ(tcIRQ());
            } else {
                tcChan()->TC_IDR = 0xFFFFFFFF;
                NVIC_DisableIRQ(tcIRQ());
            }
        }

        void setInterruptPending() {
            NVIC_SetPendingIRQ(tcIRQ());
        }

        /* Here for reference (most we don't use):
         TC_SR_COVFS   (TC_SR) Counter Overflow Status
         TC_SR_LOVRS   (TC_SR) Load Overrun Status
         TC_SR_CPAS    (TC_SR) RA Compare Status
         TC_SR_CPBS    (TC_SR) RB Compare Status
         TC_SR_CPCS    (TC_SR) RC Compare Status
         TC_SR_LDRAS   (TC_SR) RA Loading Status
         TC_SR_LDRBS   (TC_SR) RB Loading Status
         TC_SR_ETRGS   (TC_SR) External Trigger Status
         TC_SR_CLKSTA  (TC_SR) Clock Enabling Status
         TC_SR_MTIOA   (TC_SR) TIOA Mirror
         TC_SR_MTIOB   (TC_SR) TIOB Mirror
         */


        static TimerChannelInterruptOptions getInterruptCause(int16_t &channel) {
            uint32_t sr_ = _interrupt_cause_cached;
            channel = -1;

            // if it is either an overflow or a RC compare
            if (sr_ & (TC_SR_COVFS | TC_SR_CPCS)) {
                return kInterruptOnOverflow;
            }
            else if (sr_ & (TC_SR_CPAS)) {
                channel = 0;
                return kInterruptOnMatch;
            }
            else if (sr_ & (TC_SR_CPBS)) {
                channel = 1;
                return kInterruptOnMatch;
            }
            else if (sr_ & (TC_SR_ETRGS)) {
                return kInterruptOnMatch;
            }
            return kInterruptUnknown;
        }

        // Placeholder for user code.
        static void interrupt();
    }; // Timer<>

#pragma mark TimerChannel<n>
    /**************************************************
     *
     * TIMERCHANNELSS: TimerChannel<n>
     *
     **************************************************/

    template<uint8_t timerNum, uint8_t channelNum>
    struct TimerChannel : Timer<timerNum> {
        TimerChannel() : Timer<timerNum>{} {};
        TimerChannel(const TimerMode mode, const uint32_t freq) : Timer<timerNum>{mode, freq} {};

        void setDutyCycle(const float ratio) {
            Timer<timerNum>::setDutyCycleForChannel(channelNum, ratio);
        };

        float getDutyCycle() {
            return Timer<timerNum>::getDutyCycleForChannel(channelNum);
        };

        void setExactDutyCycle(const uint32_t absolute) {
            Timer<timerNum>::setExactDutyCycleForChannel(channelNum, absolute);
        };

        uint32_t getExactDutyCycle() {
            return Timer<timerNum>::getExactDutyCycleForChannel(channelNum);
        };


        void setOutputOptions(const uint32_t options) {
            Timer<timerNum>::setOutputOptions(channelNum, options);
        };

        void startPWMOutput() {
            Timer<timerNum>::startPWMOutput(channelNum);
        };

        void stopPWMOutput() {
            Timer<timerNum>::stopPWMOutput(channelNum);
        }

        void setInterrupts(const uint32_t interrupts) {
            Timer<timerNum>::setInterrupts(interrupts, channelNum);
        };

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


#pragma mark PWMTimer<n>
    /**************************************************
     *
     * PWMTimer: PWMTimer<n>
     *
     **************************************************/

    extern uint32_t pwm_interrupt_cause_cached_1_;
    extern uint32_t pwm_interrupt_cause_cached_2_;

    static constexpr uint32_t divisors[11] = {1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024};


    template <uint8_t timerNum>
    struct PWMTimer {

#if defined(PWM)
        static_assert(timerNum>=0 && timerNum<8, "PWMTimer<n>: n must be within 0 through 7 on this processor.");

        // For external inspection
        static constexpr uint8_t peripheral_num = 0;
        static constexpr uint8_t timer_num = timerNum;

        // NOTE: Notice! The *pointers* are const, not the *values*.
        static constexpr Pwm * const pwm() { return PWM; };
        static constexpr PwmCh_num * const pwmChan() { return PWM->PWM_CH_NUM + timerNum; };
        static constexpr const uint32_t peripheralId() { return ID_PWM; };
        static constexpr const IRQn_Type pwmIRQ() { return PWM_IRQn; };

#elif defined(PWM1)
        static_assert(timerNum>=0 && timerNum<16, "PWMTimer<n>: n must be within 0 through 16 on this processor.");

        // For external inspection
        static constexpr uint8_t peripheral_num = (timerNum<8)?0:1;
        static constexpr uint8_t timer_num = (timerNum<8) ? timerNum : (timerNum-8);

        // NOTE: Notice! The *pointers* are const, not the *values*.
        static constexpr Pwm * const pwm()
        {
            if (timerNum < 8) { return PWM0; }
            else              { return PWM1; }
        };
        static constexpr PwmCh_num * const pwmChan()
        {
            if (timerNum < 8) { return PWM0->PWM_CH_NUM + timerNum; }
            else              { return PWM1->PWM_CH_NUM + (timerNum-8); }
        };
        static constexpr const uint32_t peripheralId()
        {
            if (timerNum < 8) { return ID_PWM0; }
            else              { return ID_PWM1; }
        };
        static constexpr const IRQn_Type pwmIRQ()
        {
            if (timerNum < 8) { return PWM0_IRQn; }
            else              { return PWM1_IRQn; }
        };
#endif

#ifndef PWM_PTCR_TXTEN
        DMA<Pwm *, peripheral_num> dma_ { nullptr }; // nullptr instead of the handler
        constexpr const DMA<Pwm *, peripheral_num> *dma() { return &dma_; };
#endif

        PWMTimer() { init(); };
        PWMTimer(const TimerMode mode, const uint32_t freq) {
            init();
            setModeAndFrequency(mode, freq);
        };

        void init() {
            /* Unlock this thing */
            unlock();
#ifndef PWM_PTCR_TXTEN
            dma()->reset();
#endif
        }


        static constexpr uint8_t can_sync = true;
        static constexpr uint8_t sync_master = (timer_num == 0);

        void setSync(const bool is_sync) {
            if (is_sync) {
                pwm()->PWM_SCM |=   PWM_SCM_SYNC0 << timer_num;
            } else {
                pwm()->PWM_SCM &= ~(PWM_SCM_SYNC0 << timer_num);
            }
        };

        void setSyncMode(const TimerSyncMode m, uint32_t periods = 1) {
            pwm()->PWM_SCM = (pwm()->PWM_SCM & ~(PWM_SCM_UPDM_Msk | PWM_SCM_PTRM | PWM_SCM_PTRCS_Msk)) |
              ((m == kTimerSyncManually) ?
                PWM_SCM_UPDM_MODE1 :
//               (PWM_SCM_UPDM_MODE2 | PWM_SCM_PTRM | PWM_SCM_PTRCS(2)) // set the update to occur int he middle of the period
               (PWM_SCM_UPDM_MODE2)
              );

            if (periods < 1) { periods = 1; } // minimum of 1
            if (periods > 0xF) { periods = 0xF; } // maximum of 0xF
            pwm()->PWM_SCUPUPD = PWM_SCUPUPD_UPRUPD(periods - 1);
            pwm()->PWM_SCUC = PWM_SCUC_UPDULOCK;
        };

        bool startTransfer(uint8_t * const buffer, const uint16_t length) {
#ifdef PWM_PTCR_TXTEN
            if (pwm()->PWM_TCR == 0) {
//                stop();
//                pwm()->PWM_PTCR = PWM_PTCR_TXTDIS;
                pwm()->PWM_TPR = (uint32_t)buffer;
                pwm()->PWM_TCR = length;
            } else if (pwm()->PWM_TNCR == 0) {
                pwm()->PWM_TNPR = (uint32_t)buffer;
                pwm()->PWM_TNCR = length;
            } else {
                return false; // was unable to start a transfer
            }
            pwm()->PWM_PTCR = PWM_PTCR_TXTEN;
            start();
            return true;
#else
            dma()->startTXTransfer(buffer, length, false);
            start();
            return true;
#endif
        }
        // This form allows passing a single-dimensional array by reference, and deduces the full length
        template<typename T>
        bool startTransfer(T const &buffer) {
            static_assert(std::alignment_of<T>::value == 2, "startTransfer(buffer): buffer must be an array of two-byte values");
            return startTransfer((uint8_t *)(&buffer), std::extent<T>::value);
        }

        bool isTransferDone() {
#ifdef PWM_PTCR_TXTEN
            return ((pwm()->PWM_TCR == 0) && (pwm()->PWM_TNCR == 0));
#else
            return dma()->doneWriting();
#endif
        }

#ifndef YOU_REALLY_WANT_PWM_LOCK_AND_UNLOCK
#define YOU_REALLY_WANT_PWM_LOCK_AND_UNLOCK 0
#endif

#if YOU_REALLY_WANT_PWM_LOCK_AND_UNLOCK
        // You probably don't....
        void unlock() {
            pwm()->PWM_WPCR = PWM_WPCR_WPKEY(0x50574D /* "PWM" */);
        }

        /* WHOA!! Only do this if you know what you're doing!! */
        void lock() {
            // This locks EVERYTHING!!!
            pwm()->PWM_WPCR = PWM_WPCR_WPCMD(1) | PWM_WPCR_WPRG0 | PWM_WPCR_WPRG1 | PWM_WPCR_WPRG2 | PWM_WPCR_WPRG3 | PWM_WPCR_WPRG4 | PWM_WPCR_WPRG5 | TC_WPMR_WPKEY_PASSWD;
        }
#else
        // Non-ops to keep the compiler happy.
        void unlock() {};
        void lock() {};
#endif

        /* Set the mode and frequency.
         * Returns: The actual frequency that was used, or kFrequencyUnattainable
         * freq is not const since we may "change" it.
         * PWM module can optionally use one of two additional prescale multipliers:
         *     A (kPWMClockPrescaleAndDivA) or B (kPWMClockPrescaleAndDivB).
         * However, these are shared clocks used by all PWM channels.
         * Only use these on timers that will have drastically different periods.
         * There is currently no way to set multiple times to the same Clock A or B.
         */
        int32_t setModeAndFrequency(const TimerMode mode, uint32_t frequency, const uint8_t clock = kPWMClockPrescalerOnly) {
            /* Prepare to be able to make changes: */
            /*   Disable TC clock */
            pwm()->PWM_DIS = 1 << timerNum ;
            /*   Disable interrupts */
            pwm()->PWM_IDR1 = 0xFFFFFFFF ;
            pwm()->PWM_IDR2	= 0xFFFFFFFF ;

            SamCommon::enablePeripheralClock(peripheralId());

            if (mode == kTimerInputCapture || mode == kTimerInputCaptureToMatch)
                return kFrequencyUnattainable;

            // Remember: kTimerUpDownToMatch and kPWMCenterAligned are identical.
            if (mode == kPWMCenterAligned)
                frequency /= 2;

            /* Setup clock "prescaler" */
            /* Divisors: 1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024 */

            // Grab the master clock value.
            uint32_t masterClock = SamCommon::getPeripheralClockFreq();

            // Store the divisor temporarily, to avoid looking it up again...
            uint8_t divisor_index = 0;
            uint32_t prescaler;

            if (clock == kPWMClockPrescaleAndDivA || clock == kPWMClockPrescaleAndDivB) {
                // ** THIS CLOCK A/CLOCK B CODE ALL NEEDS CHECKED ** //


                // Find prescaler value
                prescaler = (masterClock / divisors[divisor_index]) / frequency;
                while ((prescaler > 255) && (divisor_index < 11)) {
                    divisor_index++;
                    prescaler = (masterClock / divisors[divisor_index]) / frequency;
                }


                // Set the actual frequency, if we found a match.
                if (clock == kPWMClockPrescaleAndDivA) {
                    // Setup divisor A

                    pwm()->PWM_CLK = (pwm()->PWM_CLK & ~PWM_CLK_DIVA_Msk) | prescaler | (divisors[divisor_index] << 8);

                    int32_t newTop = masterClock/(divisors[divisor_index] * prescaler * frequency);
                    setTop(newTop, /*setOnNext=*/false);

                    // Determine and return the new frequency.
                    return masterClock/(divisors[divisor_index]*newTop);
                }
                else { // if clock == kPWMClockPrescaleAndDivB
                    // SAME THING, BUT B
                }
            }

            // if clock == kPWMClockPrescalerOnly

            // Find prescaler value
            uint32_t test_value = masterClock / divisors[divisor_index];

            // We assume if (divisor_index == 10) then 10 will be the value we use...
            // We want OUT of the while loop when we have the right divisor.
            // AGAIN: FAILING this test means we have the RIGHT divisor.
            while ((divisor_index < 10) && ((frequency > test_value) || (frequency < (test_value / 0x10000)))) {
                divisor_index++;
                test_value = masterClock / divisors[divisor_index];
            }

            pwmChan()->PWM_CMR = (divisor_index & 0xff) | (mode == kPWMCenterAligned ? PWM_CMR_CALG : 0) |
            /* Preserve inversion: */(pwmChan()->PWM_CMR & PWM_CMR_CPOL);

            // ToDo: Polarity setttings, Dead-Time control, Counter events

            int32_t newTop = test_value / frequency;
            setTop(newTop, /*setOnNext=*/ false);

            // Determine and return the new frequency.
            return test_value * newTop;
        };

        // Set the TOP value for modes that use it.
        // WARNING: No sanity checking is done to verify that you are, indeed, in a mode that uses it.
        void setTop(const uint32_t topValue, bool setOnNext = false) {
            if (setOnNext)
                pwmChan()->PWM_CPRDUPD = topValue;
            else
                pwmChan()->PWM_CPRD = topValue;
        };

        // Here we want to get what the TOP value is.
        uint32_t getTopValue() {
            return pwmChan()->PWM_CPRD;
        };

        // Return the current value of the counter. This is a fleeting thing...
        uint32_t getValue() {
            return pwmChan()->PWM_CCNT;
        }

        void start() {
            pwm()->PWM_ENA = 1 << timerNum;
        };

        void stop() {
            pwm()->PWM_DIS = 1 << timerNum;
        };

        // Channel-specific functions. These are Motate channels, but they happen to line-up.

        // Specify the duty cycle as a value from 0.0 .. 1.0;
        void setDutyCycleForChannel(const uint8_t channel, const float ratio, bool setOnNext = false) {
            setExactDutyCycle(ratio, setOnNext);
        };
        void setDutyCycle(const float ratio, bool setOnNext = false) {
            if (setOnNext)
                pwmChan()->PWM_CDTYUPD = getTopValue() * ratio;
            else
                pwmChan()->PWM_CDTY = getTopValue() * ratio;

        };
        float getDutyCycleForChannel(const uint8_t channel) {
            return getDutyCycle();

        };
        float getDutyCycle() {
            return (float)pwmChan()->PWM_CDTY / getTopValue();

        };


        // Specify channel A/B duty cycle as a integer value from 0 .. TOP.
        // TOP in this case is either RC_RC or 0xFFFF.
        void setExactDutyCycleForChannel(const uint8_t channel, const uint32_t absolute, bool setOnNext = false) {
            setExactDutyCycle(absolute, setOnNext);
        };
        void setExactDutyCycle(const uint32_t absolute, bool setOnNext = false) {
            if (setOnNext)
                pwmChan()->PWM_CDTYUPD = absolute;
            else
                pwmChan()->PWM_CDTY = absolute;

        };
        uint32_t getExactDutyCycleForChannel(const uint8_t channel) {
            return getExactDutyCycle();
        };
        uint32_t getExactDutyCycle() {
            return pwmChan()->PWM_CDTY;
        };


        void setOutputOptions(const uint32_t channel, const uint32_t options) {
            setOutputOptions(options);
        };

        void setOutputOptions(const uint32_t options) {
            if (options == kPWMOnInverted) {
                pwmChan()->PWM_CMR |= PWM_CMR_CPOL;
            }
            else if (options == kPWMOn) {
                pwmChan()->PWM_CMR &= ~PWM_CMR_CPOL;
            }
        };


        // ASSUMPTION: The pin is not in Toggle mode.
        void stopPWMOutput(const uint32_t channel) {
            stopPWMOutput();
        };
        void stopPWMOutput() {
            //			stop();
        };

        // These two start the waveform. We try to be as fast as we can.
        // ASSUMPTION: We stopped it with the corresponding function.
        // ASSUMPTION: The pin is not and was not in Toggle mode.
        void startPWMOutput(const uint32_t channel) {
            startPWMOutput();
        };
        void startPWMOutput() {
            //			start();
        };

        void setInterrupts(const uint32_t interrupts) {
            pwm()->PWM_IDR1 = (1 << timerNum);
            pwm()->PWM_IDR2 = (PWM_IDR2_CMPM0 << timerNum);

            if (interrupts != kInterruptsOff) {
                NVIC_EnableIRQ(pwmIRQ());

                if (interrupts & kInterruptOnOverflow) {
                    pwm()->PWM_IER1 = (1 << timerNum);
                }
                if (interrupts & kInterruptOnMatch) {
                    pwm()->PWM_IER2 = (PWM_IER2_CMPM0 << timerNum);
                }

                /* Set interrupt priority */
                if (interrupts & kInterruptPriorityHighest) {
                    NVIC_SetPriority(pwmIRQ(), 0);
                }
                else if (interrupts & kInterruptPriorityHigh) {
                    NVIC_SetPriority(pwmIRQ(), 3);
                }
                else if (interrupts & kInterruptPriorityMedium) {
                    NVIC_SetPriority(pwmIRQ(), 7);
                }
                else if (interrupts & kInterruptPriorityLow) {
                    NVIC_SetPriority(pwmIRQ(), 11);
                }
                else if (interrupts & kInterruptPriorityLowest) {
                    NVIC_SetPriority(pwmIRQ(), 15);
                }

            } else {
                pwm()->PWM_IDR1 = (1 << timerNum);
                pwm()->PWM_IDR2 = (PWM_IDR2_CMPM0 << timerNum);

                // If none of the interrupts are on, we can disable the IRQ altogether.
                if (0==(pwm()->PWM_IMR1) && 0==(pwm()->PWM_IMR2)) {
                    NVIC_DisableIRQ(pwmIRQ());
                }
            }
        }

        void setInterruptPending() {
            NVIC_SetPendingIRQ(pwmIRQ());
        }

        TimerChannelInterruptOptions getInterruptCause(const uint32_t channel) {
            return getInterruptCause();
        }

        TimerChannelInterruptOptions getInterruptCause() {
            // if it is either an overflow or a RC compare
            if (pwm_interrupt_cause_cached_1_ != 0) {
                return kInterruptOnOverflow;
            }
            else if (pwm_interrupt_cause_cached_2_ != 0) {
                return kInterruptOnMatch;
            }

            return kInterruptUnknown;
        }

        // Placeholder for user code.
        static void interrupt();
    }; // struct PWMTimer


    template<uint8_t timerNum, uint8_t channelNum>
    struct PWMTimerChannel : PWMTimer<timerNum> {
        //Intentionally empty
    };

#pragma mark SysTickEvent, Timer<SysTickTimerNum> SysTickTimer
    /**************************************************
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
    template <>
    struct Timer<SysTickTimerNum> {
        static volatile uint32_t _motateTickCount;
        SysTickEvent *firstEvent = nullptr;

        Timer() { init(); };
        Timer(const TimerMode mode, const uint32_t freq) {
            init();
        };

        void init() {
            _motateTickCount = 0;

            // Set Systick to 1ms interval, common to all SAM3 variants
            if (SysTick_Config(SamCommon::getPeripheralClockFreq() / 1000))
            {
                // Capture error
                while (true);
            }
        };

        // Return the current value of the counter. This is a fleeting thing...
        uint32_t getValue() {
            return _motateTickCount;
        };

        void _increment() {
            _motateTickCount++;
        };

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
        };

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
        };

        void _handleEvents() {
            SysTickEvent *event = firstEvent;
            while (event != nullptr) {
                event->callback();
                event = event->next;
            }
        };

        // Placeholder for user code.
        static void interrupt() __attribute__ ((weak));
    };
    extern Timer<SysTickTimerNum> SysTickTimer;

#pragma mark WatchDogTimer, Timer<WatchDogTimerNum>
    /**************************************************
     *
     * Timer<WatchDogTimerNum> and related:
     *  Timer<WatchDogTimerNum> is the special Timer for the hardware watchdog timer.
     *  WatchDogTimer is the global singleton to access it.
     *  Currently only supported is to call WatchDogTimer.disable(0 immediately.
     *
     **************************************************/

    static const timer_number WatchDogTimerNum = 0xFE;
    template <>
    struct Timer<WatchDogTimerNum> {

        Timer() { init(); };
        Timer(const TimerMode mode, const uint32_t freq) {
            init();
            //			setModeAndFrequency(mode, freq);
        };

        void init() {
        };

        void disable() {
            WDT->WDT_MR = WDT_MR_WDDIS;
        };

        void checkIn() {

        };

        // Placeholder for user code.
        static void interrupt() __attribute__ ((weak));
    };
    extern Timer<WatchDogTimerNum> WatchDogTimer;

#pragma mark delay()
    /**************************************************
     *
     * delay(): Arduino-compatible blocking-delay function
     *
     **************************************************/

    inline void delay( uint32_t microseconds )
    {
        uint32_t doneTime = SysTickTimer.getValue() + microseconds;

        do
        {
            __NOP();
        } while ( SysTickTimer.getValue() < doneTime );
    }


#pragma mark Timeout
    /**************************************************
     *
     * Timeout: Simple non-blocking (polling) timeout class.
     *
     **************************************************/
    struct Timeout {
        uint32_t start_, delay_;
        Timeout() : start_ {0}, delay_ {0} {};

        bool isSet() {
            return (start_ > 0);
        }

        bool isPast() {
            if (!isSet()) {
                return false;
            }
            return ((SysTickTimer.getValue() - start_) > delay_);
        };

        void set(uint32_t delay) {
            start_ = SysTickTimer.getValue();
            delay_ = delay;
        };

        void clear() {
            start_ = 0;
            delay_ = 0;
        }
    };

} // namespace Motate

#pragma mark MOTATE_TIMER_INTERRUPT, MOTATE_TIMER_CHANNEL_INTERRUPT
/**************************************************
 *
 * #defines that allow simple definition of interrupt functions
 *
 **************************************************/

#define MOTATE_TIMER_INTERRUPT(number) template<> void Motate::Timer<number>::interrupt()
#define MOTATE_TIMER_CHANNEL_INTERRUPT(t, ch) template<> void Motate::TimerChannel<t, ch>::interrupt()








/** THIS IS OLD INFO, AND NO LONGER RELEVANT TO THIS PROJECT, BUT IT WAS HARD TO COME BY: **/

/*****
 Ok, here we get ugly: We need the *mangled* names for the specialized interrupt functions,
 so that we can use weak references from C functions TCn_Handler to the C++ Timer<n>::interrupt(),
 so that we get clean linkage to user-provided functions, and no errors if those functions don't exist.

 So, to get the mangled names (which will only for for GCC, btw), I do this in a bash shell (ignore any errors after the g++ line):

 cat <<END >> temp.cpp
 #include <cinttypes>
 namespace Motate {
 template <uint8_t timerNum>
 struct Timer {
 static void interrupt();
 };
 template<> void Timer<0>::interrupt() {};
 template<> void Timer<1>::interrupt() {};
 template<> void Timer<2>::interrupt() {};
 template<> void Timer<3>::interrupt() {};
 template<> void Timer<4>::interrupt() {};
 template<> void Timer<5>::interrupt() {};
 template<> void Timer<6>::interrupt() {};
 template<> void Timer<7>::interrupt() {};
 template<> void Timer<8>::interrupt() {};
 }
 END
 arm-none-eabi-g++ temp.cpp -o temp.o -mthumb -nostartfiles -mcpu=cortex-m3
 arm-none-eabi-nm temp.o | grep Motate
 rm temp.o temp.cpp


 You should get output like this:

 00008000 T _ZN6Motate5TimerILh0EE9interruptEv
 0000800c T _ZN6Motate5TimerILh1EE9interruptEv
 00008018 T _ZN6Motate5TimerILh2EE9interruptEv
 00008024 T _ZN6Motate5TimerILh3EE9interruptEv
 00008030 T _ZN6Motate5TimerILh4EE9interruptEv
 0000803c T _ZN6Motate5TimerILh5EE9interruptEv
 00008048 T _ZN6Motate5TimerILh6EE9interruptEv
 00008054 T _ZN6Motate5TimerILh7EE9interruptEv
 00008060 T _ZN6Motate5TimerILh8EE9interruptEv

 Ignore the hex number and T at the beginning, and the rest is the mangled names you need for below.
 I broke the string into three parts to clearly show the part that is changing.
 */


#endif /* end of include guard: SAMTIMERS_H_ONCE */
