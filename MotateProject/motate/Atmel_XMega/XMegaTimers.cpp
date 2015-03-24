/*
  KL05ZTimers.cpp - Library for the Arduino-compatible Motate system
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


#if defined(__AVR_XMEGA__)

#include "Atmel_XMega/XMegaTimers.h"
#include <avr/io.h>
#include <avr/interrupt.h>
//#include "Reset.h"

namespace Motate {

    template<>
    void Motate::Timer<0>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<0, 0>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<0, 1>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<0, 2>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<0, 3>::interrupt()  __attribute__ ((weak));


    template<>
    void Motate::Timer<1>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<1, 0>::interrupt()  __attribute__ ((weak));

    template<>
    void Motate::TimerChannel<1, 1>::interrupt()  __attribute__ ((weak));



    template<>
    TimerChannelInterruptOptions Timer<0>::_interruptCause = kInterruptUnknown;
    template<>
    int8_t                       Timer<0>::_interruptCauseChannel = 0;

    template<>
    TimerChannelInterruptOptions Timer<1>::_interruptCause = kInterruptUnknown;
    template<>
    int8_t                       Timer<1>::_interruptCauseChannel = 0;

    /* System-wide tick counter */
	/*  Inspired by code from Atmel and Arduino.
	 *  Some of which is:   Copyright (c) 2012 Arduino. All right reserved.
	 *  Some of which is:   Copyright (c) 2011-2012, Atmel Corporation. All rights reserved.
	 */

	Timer<SysTickTimerNum> SysTickTimer;
	Timer<WatchDogTimerNum> WatchDogTimer;

	volatile uint32_t Timer<SysTickTimerNum>::_motateTickCount = 0;

} // namespace Motate



// TC0 OVerFlow Interrupt
ISR(TCC0_OVF_vect) {
    if (Motate::Timer<0>::interrupt) {
        Motate::Timer<0>::_setInterruptCause(Motate::kInterruptOnOverflow, -1);
        Motate::Timer<0>::interrupt();
    }
}

// TC0 CC A Interrupt
ISR(TCC0_CCA_vect) {
    if (!Motate::TimerChannel<0, 0>::interrupt) {
        Motate::Timer<0>::_setInterruptCause(Motate::kInterruptOnMatch, 0);
        Motate::Timer<0>::interrupt();
    } else {
        Motate::TimerChannel<0, 0>::interrupt();
    }
}

// TC0 CC B Interrupt
ISR(TCC0_CCB_vect) {
    if (!Motate::TimerChannel<0, 1>::interrupt) {
        Motate::Timer<0>::_setInterruptCause(Motate::kInterruptOnMatch, 1);
        Motate::Timer<0>::interrupt();
    } else {
        Motate::TimerChannel<0, 1>::interrupt();
    }
}
// TC0 CC C Interrupt
ISR(TCC0_CCC_vect) {
    if (!Motate::TimerChannel<0, 2>::interrupt) {
        Motate::Timer<0>::_setInterruptCause(Motate::kInterruptOnMatch, 2);
        Motate::Timer<0>::interrupt();
    } else {
        Motate::TimerChannel<0, 2>::interrupt();
    }
}
// TC0 CC D Interrupt
ISR(TCC0_CCD_vect) {
    if (!Motate::TimerChannel<0, 3>::interrupt) {
        Motate::Timer<0>::_setInterruptCause(Motate::kInterruptOnMatch, 3);
        Motate::Timer<0>::interrupt();
    } else {
        Motate::TimerChannel<0, 3>::interrupt();
    }
}


// TC1 OVerFlow Interrupt
ISR(TCC1_OVF_vect) {
    if (Motate::Timer<1>::interrupt) {
        Motate::Timer<1>::_setInterruptCause(Motate::kInterruptOnOverflow, -1);
        Motate::Timer<1>::interrupt();
    }
}

// TC1 CC A Interrupt
ISR(TCC1_CCA_vect) {
    if (!Motate::TimerChannel<1, 0>::interrupt) {
        Motate::Timer<1>::_setInterruptCause(Motate::kInterruptOnMatch, 0);
        Motate::Timer<1>::interrupt();
    } else {
        Motate::TimerChannel<1, 0>::interrupt();
    }
}
// TC1 CC B Interrupt
ISR(TCC1_CCB_vect) {
    if (!Motate::TimerChannel<1, 1>::interrupt) {
        Motate::Timer<1>::_setInterruptCause(Motate::kInterruptOnMatch, 1);
        Motate::Timer<1>::interrupt();
    } else {
        Motate::TimerChannel<1, 1>::interrupt();
    }
}


// RTC COMPare Interrupt
ISR(RTC_COMP_vect)
{
//    if (Motate::SysTickTimer.interrupt) {
//        Motate::SysTickTimer.interrupt();
//    }
}

// RTC OVerFlow Interrupt
ISR(RTC_OVF_vect)
{
    Motate::SysTickTimer._increment();

    if (Motate::SysTickTimer.interrupt) {
        Motate::SysTickTimer.interrupt();
    }
}

//extern "C" {
//
//    void TPM0_IRQHandler(void) { Motate::Timer<0>::interrupt(); }
//    void TPM1_IRQHandler(void) { Motate::Timer<1>::interrupt(); }
//
//}

#endif // __AVR_XMEGA__
