/*
  SamTimers.cpp - Library for the Arduino-compatible Motate system
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


#include "SamTimers.h"
#include "SamCommon.h"

extern "C" {
    // void _null_pwm_timer_interrupt() __attribute__ ((unused));
    // void _null_pwm_timer_interrupt() {};
}

namespace Motate {
	/* System-wide tick counter */
	/*  Inspired by code from Atmel and Arduino.
	 *  Some of which is:   Copyright (c) 2012 Arduino. All right reserved.
	 *  Some of which is:   Copyright (c) 2011-2012, Atmel Corporation. All rights reserved.
	 */

	Timer<SysTickTimerNum> SysTickTimer;
	Timer<WatchDogTimerNum> WatchDogTimer;

	volatile uint32_t Timer<SysTickTimerNum>::_motateTickCount = 0;

} // namespace Motate

extern "C" void SysTick_Handler(void)
{
//	if (sysTickHook)
//		sysTickHook();

//	tickReset();

	Motate::SysTickTimer._increment();

	if (Motate::SysTickTimer.interrupt) {
		Motate::SysTickTimer.interrupt();
	}

    Motate::SysTickTimer._handleEvents();
}

// , alias("_null_pwm_timer_interrupt")

#define _MAKE_TCx_Handler(x) \
    namespace Motate { \
        template<> void TimerChannel<x, 0>::interrupt() __attribute__ ((weak)); \
        template<> void TimerChannel<x, 1>::interrupt() __attribute__ ((weak)); \
        template<> void Timer<x>::interrupt() __attribute__ ((weak)); \
        template<> volatile uint32_t Timer<x>::_interrupt_cause_cached = 0; \
    } \
    extern "C" \
    void TC##x##_Handler(void) { /* delegate to the TimerChannels */ \
        Motate::Timer<x>::_interrupt_cause_cached = Motate::Timer<x>::tcChan()->TC_SR;\
        Motate::SamCommon::sync();\
        int16_t ch_ = 0; \
        /*auto tcio =*/ Motate::Timer<x>::getInterruptCause(ch_); \
        if (  Motate::TimerChannel<x, 0>::interrupt && \
              (ch_  == 0 || ch_  == -1) \
            ) { \
            Motate::TimerChannel<x, 0>::interrupt(); \
        } \
        if (  Motate::TimerChannel<x, 1>::interrupt || \
              (ch_  == 1 || ch_  == -1) \
            ) { \
            Motate::TimerChannel<x, 1>::interrupt(); \
        } \
        if (Motate::Timer<x>::interrupt) { \
            Motate::Timer<x>::interrupt(); \
        } \
    }

    _MAKE_TCx_Handler(0)
    _MAKE_TCx_Handler(1)
    _MAKE_TCx_Handler(2)
#ifdef TC1
    _MAKE_TCx_Handler(3)
    _MAKE_TCx_Handler(4)
    _MAKE_TCx_Handler(5)
#endif
#ifdef TC2
    _MAKE_TCx_Handler(6)
    _MAKE_TCx_Handler(7)
    _MAKE_TCx_Handler(8)
#endif
#ifdef TC3
    _MAKE_TCx_Handler(9)
    _MAKE_TCx_Handler(10)
    _MAKE_TCx_Handler(11)
#endif

#undef _MAKE_TCx_Handler

namespace Motate {
    uint32_t pwm_interrupt_cause_cached_1_ = 0;
    uint32_t pwm_interrupt_cause_cached_2_ = 0;

    template<> void PWMTimer<  0>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  1>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  2>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  3>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  4>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  5>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  6>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<  7>::interrupt() __attribute__ ((weak));

#if defined(PWM1)
    template<> void PWMTimer<8+0>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+1>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+2>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+3>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+4>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+5>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+6>::interrupt() __attribute__ ((weak));
    template<> void PWMTimer<8+7>::interrupt() __attribute__ ((weak));
#endif
}

#if defined(PWM)
void PWM_Handler(void) {
    Motate::pwm_interrupt_cause_cached_1_ = PWM->PWM_ISR1 & 0x00ff;
    Motate::pwm_interrupt_cause_cached_2_ = PWM->PWM_ISR2 & 0xff00;

    uint32_t pwm_interrupt_cause_ = Motate::pwm_interrupt_cause_cached_1_ | (Motate::pwm_interrupt_cause_cached_2_>>8);
    Motate::SamCommon::sync();

    if (Motate::PWMTimer< 0>::interrupt && (pwm_interrupt_cause_ & (1<<  0))) {
        Motate::PWMTimer< 0>::interrupt();
    };
    if (Motate::PWMTimer< 1>::interrupt && (pwm_interrupt_cause_ & (1<<  1))) {
        Motate::PWMTimer< 1>::interrupt();
    };
    if (Motate::PWMTimer< 2>::interrupt && (pwm_interrupt_cause_ & (1<<  2))) {
        Motate::PWMTimer< 2>::interrupt();
    };
    if (Motate::PWMTimer< 3>::interrupt && (pwm_interrupt_cause_ & (1<<  3))) {
        Motate::PWMTimer< 3>::interrupt();
    };
    if (Motate::PWMTimer< 4>::interrupt && (pwm_interrupt_cause_ & (1<<  4))) {
        Motate::PWMTimer< 4>::interrupt();
    };
    if (Motate::PWMTimer< 5>::interrupt && (pwm_interrupt_cause_ & (1<<  5))) {
        Motate::PWMTimer< 5>::interrupt();
    };
    if (Motate::PWMTimer< 6>::interrupt && (pwm_interrupt_cause_ & (1<<  6))) {
        Motate::PWMTimer< 6>::interrupt();
    };
    if (Motate::PWMTimer< 7>::interrupt && (pwm_interrupt_cause_ & (1<<  7))) {
        Motate::PWMTimer< 7>::interrupt();
    };
}
#elif defined(PWM1)
void PWM0_Handler(void) {
    Motate::pwm_interrupt_cause_cached_1_ = PWM0->PWM_ISR1 & 0x00ff;
    Motate::pwm_interrupt_cause_cached_2_ = PWM0->PWM_ISR2 & 0xff00;

    uint32_t pwm_interrupt_cause_ = Motate::pwm_interrupt_cause_cached_1_ | (Motate::pwm_interrupt_cause_cached_2_>>8);
    Motate::SamCommon::sync();

    if (Motate::PWMTimer<  0>::interrupt && (pwm_interrupt_cause_ & (1<<  0))) {
        Motate::PWMTimer<  0>::interrupt();
    };
    if (Motate::PWMTimer<  1>::interrupt && (pwm_interrupt_cause_ & (1<<  1))) {
        Motate::PWMTimer<  1>::interrupt();
    };
    if (Motate::PWMTimer<  2>::interrupt && (pwm_interrupt_cause_ & (1<<  2))) {
        Motate::PWMTimer<  2>::interrupt();
    };
    if (Motate::PWMTimer<  3>::interrupt && (pwm_interrupt_cause_ & (1<<  3))) {
        Motate::PWMTimer<  3>::interrupt();
    };
    if (Motate::PWMTimer<  4>::interrupt && (pwm_interrupt_cause_ & (1<<  4))) {
        Motate::PWMTimer<  4>::interrupt();
    };
    if (Motate::PWMTimer<  5>::interrupt && (pwm_interrupt_cause_ & (1<<  5))) {
        Motate::PWMTimer<  5>::interrupt();
    };
    if (Motate::PWMTimer<  6>::interrupt && (pwm_interrupt_cause_ & (1<<  6))) {
        Motate::PWMTimer<  6>::interrupt();
    };
    if (Motate::PWMTimer<  7>::interrupt && (pwm_interrupt_cause_ & (1<<  7))) {
        Motate::PWMTimer<  7>::interrupt();
    };
}
void PWM1_Handler(void) {
    Motate::pwm_interrupt_cause_cached_1_ = PWM1->PWM_ISR1 & 0x00ff;
    Motate::pwm_interrupt_cause_cached_2_ = PWM1->PWM_ISR2 & 0xff00;

    uint32_t pwm_interrupt_cause_ = Motate::pwm_interrupt_cause_cached_1_ | (Motate::pwm_interrupt_cause_cached_2_>>8);
    Motate::SamCommon::sync();

    if (Motate::PWMTimer<8+0>::interrupt && (pwm_interrupt_cause_ & (1<<  0))) {
        Motate::PWMTimer<8+0>::interrupt();
    };
    if (Motate::PWMTimer<8+1>::interrupt && (pwm_interrupt_cause_ & (1<<  1))) {
        Motate::PWMTimer<8+1>::interrupt();
    };
    if (Motate::PWMTimer<8+2>::interrupt && (pwm_interrupt_cause_ & (1<<  2))) {
        Motate::PWMTimer<8+2>::interrupt();
    };
    if (Motate::PWMTimer<8+3>::interrupt && (pwm_interrupt_cause_ & (1<<  3))) {
        Motate::PWMTimer<8+3>::interrupt();
    };
    if (Motate::PWMTimer<8+4>::interrupt && (pwm_interrupt_cause_ & (1<<  4))) {
        Motate::PWMTimer<8+4>::interrupt();
    };
    if (Motate::PWMTimer<8+5>::interrupt && (pwm_interrupt_cause_ & (1<<  5))) {
        Motate::PWMTimer<8+5>::interrupt();
    };
    if (Motate::PWMTimer<8+6>::interrupt && (pwm_interrupt_cause_ & (1<<  6))) {
        Motate::PWMTimer<8+6>::interrupt();
    };
    if (Motate::PWMTimer<8+7>::interrupt && (pwm_interrupt_cause_ & (1<<  7))) {
        Motate::PWMTimer<8+7>::interrupt();
    };
}

#endif
