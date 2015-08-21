/*
 KL05ZPinss.cpp - Library for the Motate system
 http://github.com/synthetos/motate/
 
 Copyright (c) 2014 Robert Giseburt
 
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


#if defined(__KL05Z__)

#include "Freescale_klxx/KL05ZPins.h"


#if 1

extern "C" {
    void _null_pin_change_interrupt() __attribute__ ((unused));
    void _null_pin_change_interrupt() {};
}

namespace Motate {

#define _TEMP_MAKE_IRQ_INTERRUPT( registerChar, registerPin) \
    void IRQPin< ReversePinLookup< registerChar, registerPin >::number >::interrupt() __attribute__ ((weak, alias("_null_pin_change_interrupt")));

    // Interrupt pins
    // PTA0/PTA1/PTA7/PTA10/PTA11/PTA12/PTA16/PTA17/PTA18
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  0 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  1 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  7 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 10 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 11 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 12 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 16 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 17 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 18 );

    // PTB0/PTB1/PTB2/PTB3/PTB4/PTB5/PTB6/PTB7/PTB14
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  0 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  1 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  2 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  3 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  4 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  5 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  6 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  7 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B', 14 );

#undef _TEMP_MAKE_IRQ_INTERRUPT

}// namespace

using Motate::IRQPin;
using Motate::ReversePinLookup;

extern "C" void PORTA_IRQHandler(void) {
    uint32_t isr = PORTA->ISFR;

#define _TEMP_MAKE_IRQ_INTERRUPT(registerChar, registerPin) \
    if (IRQPin< ReversePinLookup< registerChar, registerPin >::number >::interrupt && \
        isr & IRQPin< ReversePinLookup< registerChar, registerPin >::number >::mask) \
    { \
        IRQPin< ReversePinLookup< registerChar, registerPin >::number >::interrupt(); \
        PORTA->ISFR |= IRQPin< ReversePinLookup< registerChar, registerPin >::number >::mask; \
    }

    // Interrupt pins
    // PTA0/PTA1/PTA7/PTA10/PTA11/PTA12/PTA16/PTA17/PTA18
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  0 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  1 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A',  7 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 10 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 11 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 12 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 16 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 17 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'A', 18 );

#undef _TEMP_MAKE_IRQ_INTERRUPT

    NVIC_ClearPendingIRQ(PORTA_IRQn);
}


extern "C" void PORTB_IRQHandler(void) {
    uint32_t isr = PORTB->ISFR;

#define _TEMP_MAKE_IRQ_INTERRUPT(registerChar, registerPin) \
    if (IRQPin< ReversePinLookup< registerChar, registerPin >::number >::interrupt && \
        isr & IRQPin< ReversePinLookup< registerChar, registerPin >::number >::mask) \
    { \
        IRQPin< ReversePinLookup< registerChar, registerPin >::number >::interrupt(); \
        PORTB->ISFR |= IRQPin< ReversePinLookup< registerChar, registerPin >::number >::mask; \
    }

    // PTB0/PTB1/PTB2/PTB3/PTB4/PTB5/PTB6/PTB7/PTB14
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  0 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  1 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  2 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  3 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  4 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  5 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  6 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B',  7 );
    _TEMP_MAKE_IRQ_INTERRUPT( 'B', 14 );

#undef _TEMP_MAKE_IRQ_INTERRUPT

    NVIC_ClearPendingIRQ(PORTB_IRQn);
}


#endif // 0

#endif