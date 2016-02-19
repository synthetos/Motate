/*
 Atem_XMega/XMegaTimers.h - Library for the Motate system
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

#ifndef XMEGATIMERS_H_ONCE
#define XMEGATIMERS_H_ONCE

#include "xmega.h"
#include "avr/io.h"

namespace Motate {
    /***************************************
     * Some note about the XMega timers:
     *
     * Unlike other platforms, where each channel can have it's own "Waveform Generation,"
     * on the XMega the timer holds the setting for Waveform Generation, which means that
     * all timers that use the waveform settings to trigger interrupts will share that setting.
     *
     * Also, the chosen interrupt triggers is set along with the waveform generation mode.
     * So we simpley aloways choose the turn on all possible interrupts and provide reasonable
     * and minimal default implementations. (We have to anyway, to support them in general on
     * the AVR architecture.)
     *
     */

    enum TimerMode {
        /* InputCapture mode (WAVE = 0) */
        kTimerInputCapture         = 0,

        /* Waveform select, Up to MAX */
        kTimerUp            = 1,
        /* Waveform select, Up to TOP */
        kTimerUpToTop     = 2,
        kTimerUpToMatch     = 2,
        /* For PWM, we'll alias kTimerUpToMatch as: */
        kPWMLeftAligned     = kTimerUpToMatch,

        /* Waveform select, Up to 0xFFFFFFFF, then Down */
        kTimerUpDown        = 3,
        /* Waveform select, Up to TOP, then Down */
        kTimerUpDownToTop = 4,
        kTimerUpDownToMatch = 4,

        /* For PWM, we'll alias kTimerUpDownToMatch as: */
        kPWMCenterAligned     = kTimerUpDownToMatch,
    };

    /* We're trading acronyms for verbose CamelCase. Dubious. */
//    enum TimerChannelOutputOptions {
//        kOutputDisconnected = 0,
//
//        kToggleOnMatch      = 1<<1,
//        kClearOnMatch       = 1<<2,
//        kSetOnMatch         = 1<<3,
//
//        // Can't actually toggle on overflow...
//        /*kToggleOnOverflow   = 1<<4,*/
//        kClearOnOverflow    = 1<<5,
//        kSetOnOverflow      = 1<<6,
//
//        /* Aliases for use with PWM */
//        kPWMOn              = kClearOnMatch | kSetOnOverflow,
//        kPWMOnInverted      = kSetOnMatch | kClearOnOverflow,
//    };

    enum TimerChannelInterruptOptions {
        kInterruptsOff              = 0,
        /* Alias for "off" to make more sense
         when returned from getInterruptCause(). */
        kInterruptUnknown           = 0,

        kInterruptOnMatch           = 1<<1,
        /* Note: Interrupt on overflow is actually in the timer, not the channel. */
        kInterruptOnOverflow        = 1<<2,

        /* This turns the IRQ on, but doesn't set the timer to ever trigger it. */
//        kInterruptOnSoftwareTrigger = 1<<3,

        /* Set priority levels here as well: */
        kInterruptPriorityHighest   = 1<<5, // highest == high
        kInterruptPriorityHigh      = 1<<5,
        kInterruptPriorityMedium    = 1<<7,
        kInterruptPriorityLow       = 1<<8,
        kInterruptPriorityLowest    = 1<<8, // Lowest == low
    };

    enum TimerErrorCodes {
        kFrequencyUnattainable = -1,
        kInvalidMode = -2,
    };

    typedef const uint8_t timer_number;

    /*******************************
     * Since the headers for the XMega contains different structures for Timer 0
     * and Timer 1, and many of the defines are different for the two, we break
     * out the functions that are timer specific into a parent templated class and
     * then provide specializations for them.
     *
     * We then have to use the UGLY huge macros, or duplicate the code 7 times,
     * since there are four timers, C-F, and C-E use both timer 0 and 1.
     *
     */


    template <uint8_t timerNum>
    struct _TC_Stub {
        static void _setWGMode(const TC_WGMODE_t wgm);
        static void _setClock(const uint8_t clk);
        static uint8_t _getClock();
        static void _restart();
        static void _setPeriod(uint16_t per);
        static uint16_t _getPeriod();
        static uint16_t _getCount();

        static void _setOverflowInterruptPriority(TC_OVFINTLVL_t lvl);

        static void _setChannelAMatch(uint16_t match);
        static void _setChannelACCEN(bool output);
        static void _setChannelACCInterruptPriority(TC_CCAINTLVL_t lvl);

        static void _setChannelBMatch(uint16_t match);
        static void _setChannelBCCEN(bool output);
        static void _setChannelBCCInterruptPriority(TC_CCBINTLVL_t lvl);

        static void _setChannelCMatch(uint16_t match);
        static void _setChannelCCCEN(bool output);
        static void _setChannelCCCInterruptPriority(TC_CCCINTLVL_t lvl);

        static void _setChannelDMatch(uint16_t match);
        static void _setChannelDCCEN(bool output);
        static void _setChannelDCCInterruptPriority(TC_CCDINTLVL_t lvl);

        static void _stopAllInterrupts();
    };


#define _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, cLetter)\
    static void _setChannel##cLetter##Match(uint16_t match) { tc().CC##cLetter = (match); }\
    static void _setChannel##cLetter##CCEN(bool output) {\
        if (output) {\
            tc().CTRLB = tc().CTRLB | TC##tNum##_CC##cLetter##EN_bm;\
        } else {\
            tc().CTRLB = tc().CTRLB & ~TC##tNum##_CC##cLetter##EN_bm;\
        }\
    };\
    static void _setChannel##cLetter##CCInterruptPriority(TC_CC##cLetter##INTLVL_t lvl) {\
        tc().INTCTRLB = (tc().INTCTRLB & ~TC##tNum##_CC##cLetter##INTLVL_gm) | lvl;\
    };

    #define _MAKE_MOTATE_TC_FAKE_STUB_CHANNEL(cLetter)\
    static void _setChannel##cLetter##Match(uint16_t match) {  }\
    static void _setChannel##cLetter##CCEN(bool output) { };\
    static void _setChannel##cLetter##CCInterruptPriority(TC_CC##cLetter##INTLVL_t lvl) {};

#define _MAKE_MOTATE_TC0_STUB(timerNum, tNum, tLetter)\
    template <>\
    struct _TC_Stub<timerNum> {\
        static TC##tNum##_t& tc() {\
            return TC ## tLetter ## tNum;\
        };\
        static void _setWGMode(const TC_WGMODE_t wgm) {\
            tc().CTRLB = ( tc().CTRLB & ~TC##tNum##_WGMODE_gm ) | wgm;\
        };\
        static void _setClock(const uint8_t clk) {\
            tc().CTRLA = ( tc().CTRLA & ~TC##tNum##_CLKSEL_gm ) | clk;\
        };\
        static uint8_t _getClock() { return tc().CTRLA & TC##tNum##_CLKSEL_gm; };\
        static void _restart() { tc().CTRLFSET = TC_CMD_RESTART_gc; };\
        static void _setPeriod(uint16_t per) { tc().PER = (per); };\
        static uint16_t _getPeriod() { return tc().PER; };\
        static uint16_t _getCount() { return tc().CNT; };\
        \
        static void _setOverflowInterruptPriority(TC_OVFINTLVL_t lvl) {\
            tc().INTCTRLA = (tc().INTCTRLA & TC##tNum##_OVFINTLVL_gm) | lvl;\
        };\
        \
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, A)\
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, B)\
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, C)\
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, D)\
        \
        static void _stopAllInterrupts() {\
            tc().INTCTRLA = (tc().INTCTRLA & TC##tNum##_OVFINTLVL_gm);\
            tc().INTCTRLB = (tc().INTCTRLB & ~(TC##tNum##_CCAINTLVL_gm|TC##tNum##_CCBINTLVL_gm|TC##tNum##_CCCINTLVL_gm|TC##tNum##_CCDINTLVL_gm));\
        }\
    };

#define _MAKE_MOTATE_TC1_STUB(timerNum, tNum, tLetter)\
    template <>\
    struct _TC_Stub<timerNum> {\
        static TC##tNum##_t& tc() {\
            return TC ## tLetter ## tNum;\
        };\
        static void _setWGMode(const TC_WGMODE_t wgm) {\
            tc().CTRLB = ( tc().CTRLB & ~TC##tNum##_WGMODE_gm ) | wgm;\
        };\
        static void _setClock(const uint8_t clk) {\
            tc().CTRLA = ( tc().CTRLA & ~TC##tNum##_CLKSEL_gm ) | clk;\
        };\
        static uint8_t _getClock() { return tc().CTRLA & TC##tNum##_CLKSEL_gm; };\
        static void _restart() { tc().CTRLFSET = TC_CMD_RESTART_gc; };\
        static void _setPeriod(uint16_t per) { tc().PER = (per); };\
        static uint16_t _getPeriod() { return tc().PER; };\
        static uint16_t _getCount() { return tc().CNT; };\
        \
        static void _setOverflowInterruptPriority(TC_OVFINTLVL_t lvl) {\
            tc().INTCTRLA = (tc().INTCTRLA & TC##tNum##_OVFINTLVL_gm) | lvl;\
        };\
        \
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, A)\
        _MAKE_MOTATE_TC_STUB_CHANNEL(tNum, tLetter, B)\
        _MAKE_MOTATE_TC_FAKE_STUB_CHANNEL(C)\
        _MAKE_MOTATE_TC_FAKE_STUB_CHANNEL(D)\
        \
        static void _stopAllInterrupts() {\
            tc().INTCTRLA = (tc().INTCTRLA & TC##tNum##_OVFINTLVL_gm);\
            tc().INTCTRLB = (tc().INTCTRLB & ~(TC##tNum##_CCAINTLVL_gm|TC##tNum##_CCBINTLVL_gm));\
        }\
    };


    _MAKE_MOTATE_TC0_STUB(0, 0, C)
    _MAKE_MOTATE_TC1_STUB(1, 1, C)
    _MAKE_MOTATE_TC0_STUB(2, 0, D)
    _MAKE_MOTATE_TC1_STUB(3, 1, D)
    _MAKE_MOTATE_TC0_STUB(4, 0, E)
    _MAKE_MOTATE_TC1_STUB(5, 1, E)
    _MAKE_MOTATE_TC0_STUB(6, 0, F)


    template <uint8_t timerNum>
    struct Timer : _TC_Stub<timerNum> {
        uint8_t                             _storedClock;

        Timer() { init(); };
        Timer(const TimerMode mode, const uint32_t freq) {
            init();
            setModeAndFrequency(mode, freq, /* fromConstructor = */ true);
        };

        void init() {
            /* Unlock this thing */
            unlock();
            _storedClock = 0;
            this->_stopAllInterrupts();
        }

        void unlock() const {
            // no unlock
        }

        /* WHOA!! Only do this if you know what you're doing!! */
        void lock() const {
            // no lock
        }

        // Set the mode and frequency.
        // Returns: The actual frequency that was used, or kFrequencyUnattainable
        // freq is not const since we may "change" it
        int32_t setModeAndFrequency(const TimerMode mode, uint32_t freq, const bool fromConstructor = false) {
            /* Prepare to be able to make changes: */

            if (fromConstructor) {
                this->_restart(); // restart (reset) the timer
            }

            if (mode == kTimerUpDownToMatch || mode == kTimerUpDown) {
                freq /= 2;
                this->_setWGMode(TC_WGMODE_DS_TB_gc);
            } else if (mode == kTimerUpToMatch || mode == kTimerUpToTop) {
                this->_setWGMode(TC_WGMODE_SS_gc);
            }

            /* Setup clock "prescaler" */
            /* Divisors: TC1: 2, TC2: 8, TC3: 32, TC4: 128, TC5: ???! */
            /* For now, we don't support TC5. */

            // Grab the SystemCoreClock value, in case it's volatile.
            const uint32_t masterClock = F_CPU;

            uint32_t divisors[8] = {1, 2, 4, 8, 64, 256, 1024};
            uint8_t divisor_index = 0; // 0 is actually OFF, se we want to add one to this...

            // Find prescaler value
            uint32_t test_value = masterClock / divisors[divisor_index];

            // We assume if (divisor_index == 10) then 10 will be the value we use...
            // We want OUT of the while loop when we have the right divisor.
            // AGAIN: FAILING this test means we have the RIGHT divisor.
            while ((divisor_index < 8) && ((freq > test_value) || (freq < (test_value / 0x10000)))) {
                divisor_index++;
                test_value = masterClock / divisors[divisor_index];
            }

            _storedClock = divisor_index+1;
            this->_setClock(divisor_index+1);

            int32_t newTop = test_value / freq;
            setTop(newTop);

            // Determine and return the new frequency.
            return test_value * newTop;
        };

        // Set the TOP value for modes that use it.
        // WARNING: No sanity checking is done to verify that you are, indeed, in a mode that uses it.
        void setTop(const uint32_t topValue) {
            this->_setPeriod(topValue);
        };

        // Here we want to get what the TOP value is. Is the mode is one that resets on RC, then RC is the TOP.
        // Otherwise, TOP is 0xFFFF. In order to see if TOP is RC, we need to look at the CPCTRG (RC Compare
        // Trigger Enable) bit of the CMR (Channel Mode Register). Note that this bit position is the same for
        // waveform or Capture mode, even though the Datasheet seems to obfuscate this fact.
        uint32_t getTopValue() {
            return this->_getPeriod();
        };

        // Return the current value of the counter. This is a fleeting thing...
        uint32_t getValue() {
            return this->_getCount();
        }

        void start() {
            this->_setClock(_storedClock);
        };

        void stop() {
            _storedClock = this->_getClock();
            this->setClock(0);
        };

//        void stopOnMatch() {
//            // Not supported
//        };

        // Channel-specific functions. These are Motate channels, but they happen to line-up.
        // Motate channel A = Sam channel A.
        // Motate channel B = Sam channel B.

        // Specify the duty cycle as a value from 0.0 .. 1.0;
        void setDutyCycleForChannel(const uint8_t channel, const float ratio) {
            setExactDutyCycleForChannel(channel, getTopValue() * ratio);
        };

        // Specify channel A/B duty cycle as a integer value from 0 .. TOP.
        // TOP in this case is either RC_RC or 0xFFFF.
        void setExactDutyCycleForChannel(const uint8_t channel, const uint16_t absolute) {
            if (channel == 0) {
                this->_setChannelAMatch(absolute);
            } else if (channel == 1) {
                this->_setChannelBMatch(absolute);
            } else if (channel == 2) {
                this->_setChannelCMatch(absolute);
            } else if (channel == 3) {
                this->_setChannelDMatch(absolute);
            }
        };


/**** setOutputOptions is not supported on XMega. Should we put in a stub??
        void setOutputOptions(const uint32_t options) {
            if (options & kToggleOnMatch) {
                this->_setWGMode(TC_WGMODE_FRQ_gc);
            }
            if (options & (kClearOnMatch|kSetOnOverflow) == (kClearOnMatch|kSetOnOverflow)) {
                this->_setWGMode(TC_WGMODE__gc);
            }
            if (options & kSetOnMatch) {
                bitfield |= TC_CMR_ACPA_SET;
            }
            if (options & kToggleOnOverflow) {
                this->_setWGMode(TC_WGMODE_FRQ_gc);
            }
            if (options & kClearOnOverflow) {
                this->_setWGMode(TC_WGMODE_FRQ_gc);
            }
            if (options & kSetOnOverflow) {
                bitfield |= TC_CMR_ACPC_SET;
            }
        };
*/

        // These two start the waveform. We try to be as fast as we can.
        void startPWMOutput(const uint8_t channel) {
            if (channel == 0) {
                this->_setChannelACCEN(true);
            } else if (channel == 1) {
                this->_setChannelBCCEN(true);
            } else if (channel == 2) {
                this->_setChannelCCCEN(true);
            } else if (channel == 3) {
                this->_setChannelDCCEN(true);
            }
        };

        // These are special function for stopping output waveforms.
        // This is different from stopping the timer, which kill both channels and
        // all interrupts. This simply stops the pin output from changing, and is used
        // to set the duty cycle to 0.

        void stopPWMOutput(const uint8_t channel) {
            if (channel == 0) {
                this->_setChannelACCEN(false);
            } else if (channel == 1) {
                this->_setChannelBCCEN(false);
            } else if (channel == 2) {
                this->_setChannelCCCEN(false);
            } else if (channel == 3) {
                this->_setChannelDCCEN(false);
            }
        };

        void setInterrupts(const uint32_t interrupts, const int16_t channel = -1) {
            if (interrupts != kInterruptsOff) {
                // We intentionally don't clear the interrupts so multiple calls
                // will add to the interrupts, not replace them.

                if (interrupts & kInterruptOnOverflow) {
                    /* Set interrupt priority */
                    if (interrupts & kInterruptPriorityHigh) {
                        this->_setOverflowInterruptPriority(TC_OVFINTLVL_LO_gc);
                    }
                    else if (interrupts & kInterruptPriorityMedium) {
                        this->_setOverflowInterruptPriority(TC_OVFINTLVL_MED_gc);
                    }
                    else { // assume (interrupts & kInterruptPriorityLow)
                        this->_setOverflowInterruptPriority(TC_OVFINTLVL_HI_gc);
                    }
                }
                if (interrupts & kInterruptOnMatch && channel == 0) {
                    /* Set interrupt priority */
                    if (interrupts & kInterruptPriorityHigh) {
                        this->_setChannelACCInterruptPriority(TC_CCAINTLVL_HI_gc);
                    }
                    else if (interrupts & kInterruptPriorityMedium) {
                        this->_setChannelACCInterruptPriority(TC_CCAINTLVL_MED_gc);
                    }
                    else { // assume (interrupts & kInterruptPriorityLow)
                        this->_setChannelACCInterruptPriority(TC_CCAINTLVL_LO_gc);
                    }
                }
                if (interrupts & kInterruptOnMatch && channel == 1) {
                    /* Set interrupt priority */
                    if (interrupts & kInterruptPriorityHigh) {
                        this->_setChannelBCCInterruptPriority(TC_CCBINTLVL_HI_gc);
                    }
                    else if (interrupts & kInterruptPriorityMedium) {
                        this->_setChannelBCCInterruptPriority(TC_CCBINTLVL_MED_gc);
                    }
                    else { // assume (interrupts & kInterruptPriorityLow)
                        this->_setChannelBCCInterruptPriority(TC_CCBINTLVL_LO_gc);
                    }
                }
                if (interrupts & kInterruptOnMatch && channel == 2) {
                    /* Set interrupt priority */
                    if (interrupts & kInterruptPriorityHigh) {
                        this->_setChannelCCCInterruptPriority(TC_CCCINTLVL_HI_gc);
                    }
                    else if (interrupts & kInterruptPriorityMedium) {
                        this->_setChannelCCCInterruptPriority(TC_CCCINTLVL_MED_gc);
                    }
                    else { // assume (interrupts & kInterruptPriorityLow)
                        this->_setChannelCCCInterruptPriority(TC_CCCINTLVL_LO_gc);
                    }
                }
                if (interrupts & kInterruptOnMatch && channel == 3) {
                    /* Set interrupt priority */
                    if (interrupts & kInterruptPriorityHigh) {
                        this->_setChannelDCCInterruptPriority(TC_CCDINTLVL_HI_gc);
                    }
                    else if (interrupts & kInterruptPriorityMedium) {
                        this->_setChannelDCCInterruptPriority(TC_CCDINTLVL_MED_gc);
                    }
                    else { // assume (interrupts & kInterruptPriorityLow)
                        this->_setChannelDCCInterruptPriority(TC_CCDINTLVL_LO_gc);
                    }
                }


                // Start interrupts
            } else {
                // disable interrupts
                this->_stopAllInterrupts();
            }
        }

        void setInterruptPending() {
            // Trigger a software interrupt
        }


        static const bool has_channel_interrupts = true;

        // These two are for compile-compatibility platforms where has_channel_interrupts == false:
        static void _setInterruptCause(TimerChannelInterruptOptions c, int8_t ch) {
        }
        static TimerChannelInterruptOptions getInterruptCause(int16_t &channel) {
            return kInterruptUnknown;
        }

        // Placeholder for user code.
        static void interrupt();
    }; // Timer






    template<uint8_t timerNum, uint8_t channelNum>
    struct TimerChannel : Timer<timerNum> {
        TimerChannel() : Timer<timerNum>{} {};
        TimerChannel(const TimerMode mode, const uint32_t freq) : Timer<timerNum>{mode, freq} {};

        void setDutyCycle(const float ratio) {
            Timer<timerNum>::setDutyCycleForChannel(channelNum, ratio);
        };

        void setExactDutyCycle(const uint32_t absolute) {
            Timer<timerNum>::setExactDutyCycle(channelNum, absolute);
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

        // Placeholder for user code.
        static void interrupt();
    };


    static const timer_number SysTickTimerNum = 0xFF;
    template <>
    struct Timer<SysTickTimerNum> {
        static volatile uint32_t _motateTickCount;

        Timer() { init(); };
//        Timer(const TimerMode mode, const uint32_t freq) {
//            _init();
//        };


        /*
         * Code originally from rtc.c of the TinyG project:
         * rtc_init() - initialize and start the clock
         *
         * This routine follows the code in app note AVR1314.
         */

        void init() {
            _motateTickCount = 0;

            OSC.CTRL |= OSC_RC32KEN_bm;							// Turn on internal 32kHz.
            do {} while ((OSC.STATUS & OSC_RC32KRDY_bm) == 0);	// Wait for 32kHz oscillator to stabilize.
            do {} while (RTC.STATUS & RTC_SYNCBUSY_bm);			// Wait until RTC is not busy

            CLK.RTCCTRL = CLK_RTCSRC_RCOSC_gc | CLK_RTCEN_bm;	// Set internal 32kHz osc as RTC clock source
            do {} while (RTC.STATUS & RTC_SYNCBUSY_bm);			// Wait until RTC is not busy

            // the following must be in this order or it doesn;t work
            RTC.PER = 0xfffe;	// set overflow period to maximum -- we'll use the count + out own overflow counter
            RTC.CNT = 0;
            RTC.COMP = 0xfffe;
            RTC.CTRL = RTC_PRESCALER_DIV1_gc;					// no prescale (1x)
            RTC.INTCTRL = RTC_OVFINTLVL_MED_gc;                 // interrupt on compare
        };

        // Return the current value of the counter. This is a fleeting thing...
        uint32_t getValue() {
            return _motateTickCount + RTC.CNT;
        };

        void _increment() { // called on overflow, every 0xffff ms
            _motateTickCount += 0xffff;
        };

        // Placeholder for user code.
        static void interrupt() __attribute__ ((weak));
    };
    extern Timer<SysTickTimerNum> SysTickTimer;

    static const timer_number WatchDogTimerNum = 0xFE;
    template <>
    struct Timer<WatchDogTimerNum> {

        Timer() { init(); };
//        Timer(const TimerMode mode, const uint32_t freq) {
//            init();
//            //			setModeAndFrequency(mode, freq);
//        };

        void init() {
        };

        void disable() {
        // FIXME
//            WDT->WDT_MR = WDT_MR_WDDIS;
        };

        void checkIn() {

        };

        // Placeholder for user code.
        static void interrupt();
    };
    extern Timer<WatchDogTimerNum> WatchDogTimer;

    // Provide a Arduino-compatible blocking-delay function
    inline void delay( uint32_t microseconds )
    {
        uint32_t doneTime = SysTickTimer.getValue() + microseconds;

        do
        {
            // Huh! No __NOP() macro. Oh well...
            __asm__ __volatile__ ("nop");
        } while ( SysTickTimer.getValue() < doneTime );
    }

    struct Timeout {
        uint32_t start_, delay_;
        Timeout() : start_ {0}, delay_ {0} {};

        bool isPast() {
            return ((SysTickTimer.getValue() - start_) > delay_);
        };

        void set(uint32_t delay) {
            start_ = SysTickTimer.getValue();
            delay_ = delay;
        }
    };

} // namespace Motate

#define MOTATE_TIMER_INTERRUPT(number) \
    template<> void Motate::Timer<number>::interrupt() __attribute__((signal)); \
    template<> void Motate::Timer<number>::interrupt()
#define MOTATE_TIMER_CHANNEL_INTERRUPT(t, ch) \
    template<> void Motate::TimerChannel<t, ch>::interrupt() __attribute__((signal)) ; \
    template<> void Motate::TimerChannel<t, ch>::interrupt()

#endif /* end of include guard: XMEGATIMERS_H_ONCE */