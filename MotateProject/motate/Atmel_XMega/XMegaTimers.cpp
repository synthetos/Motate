/*
  KL05ZTimers.cpp - Library for the Motate system
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


#if defined(__AVR_XMEGA__)

#include "Atmel_XMega/XMegaTimers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//#include "Reset.h"

// See the explanation of MY_ISR in XMegaPins.cpp

#  define MY_ISR(vector, ...) \
    extern "C" void vector (void) __attribute__ ((used, externally_visible)) __VA_ARGS__; \
    void vector (void)


extern "C" {
    void _null_interrupt() __attribute__ ((naked));
    void _null_interrupt() {};
}

#define _MOTATE_PREDEFINE_TIMER0(tNum) \
    namespace Motate { \
    template<> void Motate::Timer<tNum>::interrupt()  __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 0>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 1>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 2>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 3>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    }

#define _MOTATE_PREDEFINE_TIMER1(tNum) \
    namespace Motate { \
    template<> void Motate::Timer<tNum>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 0>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    template<> void Motate::TimerChannel<tNum, 1>::interrupt() __attribute__ ((weak, alias("_null_interrupt"))); \
    }


    _MOTATE_PREDEFINE_TIMER0(0); // C 0
    _MOTATE_PREDEFINE_TIMER1(1); // C 1
    _MOTATE_PREDEFINE_TIMER0(2); // D 0
    _MOTATE_PREDEFINE_TIMER1(3); // D 1
    _MOTATE_PREDEFINE_TIMER0(4); // E 0
    _MOTATE_PREDEFINE_TIMER1(5); // E 1
    _MOTATE_PREDEFINE_TIMER0(6); // F 0


namespace Motate { \

    /* System-wide tick counter */
	/*  Inspired by code from Atmel and Arduino.
	 *  Some of which is:   Copyright (c) 2012 Arduino. All right reserved.
	 *  Some of which is:   Copyright (c) 2011-2012, Atmel Corporation. All rights reserved.
	 */

	Timer<SysTickTimerNum> SysTickTimer;
	Timer<WatchDogTimerNum> WatchDogTimer;

	volatile uint32_t Timer<SysTickTimerNum>::_motateTickCount = 0;

} // namespace Motate


#define _MOTATE_CREATE_ISRS0(tLetter, tNum) \
    /* OVerFlow Interrupt */ \
    MY_ISR(TC ## tLetter ## 0_OVF_vect) { \
        if (Motate::Timer<tNum>::interrupt != _null_interrupt) { \
            Motate::Timer<tNum>::interrupt(); \
        } \
    } \
    /* CC A Interrupt */ \
    MY_ISR(TC ## tLetter ## 0_CCA_vect) { \
        if (Motate::TimerChannel<tNum, 0>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 0>::interrupt(); \
        } \
    } \
    /* CC B Interrupt */ \
    MY_ISR(TC ## tLetter ## 0_CCB_vect) { \
        if (Motate::TimerChannel<tNum, 1>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 1>::interrupt(); \
        } \
    } \
    /* CC C Interrupt */ \
    MY_ISR(TC ## tLetter ## 0_CCC_vect) { \
        if (Motate::TimerChannel<tNum, 2>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 2>::interrupt(); \
        } \
    } \
    /* CC D Interrupt */ \
    MY_ISR(TC ## tLetter ## 0_CCD_vect) { \
        if (Motate::TimerChannel<tNum, 3>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 3>::interrupt(); \
        } \
    }

#define _MOTATE_CREATE_ISRS1(tLetter, tNum) \
    /* OVerFlow Interrupt */ \
    MY_ISR(TC ## tLetter ## 1_OVF_vect) { \
        if (Motate::Timer<tNum>::interrupt != _null_interrupt) { \
            Motate::Timer<tNum>::interrupt(); \
        } \
    } \
    /* CC A Interrupt */ \
    MY_ISR(TC ## tLetter ## 1_CCA_vect) { \
        if (Motate::TimerChannel<tNum, 0>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 0>::interrupt(); \
        } \
    } \
    /* CC B Interrupt */ \
    MY_ISR(TC ## tLetter ## 1_CCB_vect) { \
        if (Motate::TimerChannel<tNum, 1>::interrupt != _null_interrupt) { \
            Motate::TimerChannel<tNum, 1>::interrupt(); \
        } \
    }

_MOTATE_CREATE_ISRS0(C, 0)
_MOTATE_CREATE_ISRS1(C, 1)
_MOTATE_CREATE_ISRS0(D, 2)
_MOTATE_CREATE_ISRS1(D, 3)
_MOTATE_CREATE_ISRS0(E, 4)
_MOTATE_CREATE_ISRS1(E, 5)
_MOTATE_CREATE_ISRS0(F, 6)

// RTC COMPare Interrupt
MY_ISR(RTC_COMP_vect)
{
}

// RTC OVerFlow Interrupt
MY_ISR(RTC_OVF_vect)
{
    Motate::SysTickTimer._increment();

    if (Motate::SysTickTimer.interrupt) {
        Motate::SysTickTimer.interrupt();
    }
}

#endif // __AVR_XMEGA__
