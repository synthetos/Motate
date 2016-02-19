/*
 XMegaPins.cpp - Library for the Motate system
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


#if defined(__AVR_XMEGA__)

#include "Atmel_XMega/XMegaPins.h"

using namespace Motate;

// Here we define a MY_ISR, which is similar to the avrlibc ISR routine, except it does NOT
// declare the function as 'signal', which will push/pop EVERY register for every ISR,
// even if it does nothing.

// Using MY_ISR means that a interrupt routine can be optimized down to just the ret
// instruction, and takes very little space.

// However, there's a cost: any function that is called from a MY_ISR function MUST be
// declared with the 'sinal' attribute. You must also be careful not to do anything that
// would require the use of registers in the MY_ISR directly. This is because, as an
// interrupt, there will be some other context that is being interrupted and is expecting
// the registers to not change. The push and pop of registers has to happen if they are
// used.

// Our intent here is to ONLY do comparisons that will compile out, and then call functions
// that are declared as 'signals'. We're also assuming, in this case, that we will only call
// one or two pin change interrupts per port, since there will be redundant register push/pop
// and the higher pins will have to wait for the lower pins to finish.

// Also note that on the XMega, the hardware doesn't record which pin triggered the interrupt,
// so we have no choice but to call all of the pin change interrupts of a whole port. :-/

#  define MY_ISR(vector, ...) \
extern "C" void vector (void) __attribute__ ((used, externally_visible)) __VA_ARGS__; \
void vector (void)


#if 1

extern "C" {
    void _null_pin_interrupt() __attribute__ ((naked));
    void _null_pin_interrupt() {};
}

#define _MAKE_MOTATE_PIN_INTERRUPTS(portLtr, portChr) \
namespace Motate { \
    template<> PORT_t& Port8<portChr>::port_proxy = PORT##portLtr; \
    Port8<portChr> port##portLtr; \
    template<> void _IRQPin<portChr,0>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,1>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,2>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,3>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,4>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,5>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,6>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
    template<> void _IRQPin<portChr,7>::interrupt() __attribute__ ((weak, alias("_null_pin_interrupt"))); \
} \
\
MY_ISR(PORT##portLtr##_INT0_vect) { \
    if (LookupIRQPin<portChr,0>::number != -1 && _IRQPin<portChr,0>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,0>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,1>::number != -1 && _IRQPin<portChr,1>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,1>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,2>::number != -1 && _IRQPin<portChr,2>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,2>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,3>::number != -1 && _IRQPin<portChr,3>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,3>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,4>::number != -1 && _IRQPin<portChr,4>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,4>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,5>::number != -1 && _IRQPin<portChr,5>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,5>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,6>::number != -1 && _IRQPin<portChr,6>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,6>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,7>::number != -1 && _IRQPin<portChr,7>::interrupt != _null_pin_interrupt) { \
        _IRQPin<portChr,7>::interrupt(); \
    } \
}

_MAKE_MOTATE_PIN_INTERRUPTS(A, 'A') // PORTA_INT0_vect
_MAKE_MOTATE_PIN_INTERRUPTS(B, 'B') // PORTB_INT0_vect
_MAKE_MOTATE_PIN_INTERRUPTS(C, 'C') // PORTC_INT0_vect
_MAKE_MOTATE_PIN_INTERRUPTS(D, 'D') // PORTD_INT0_vect
_MAKE_MOTATE_PIN_INTERRUPTS(E, 'E') // PORTE_INT0_vect
_MAKE_MOTATE_PIN_INTERRUPTS(F, 'F') // PORTF_INT0_vect

#endif // 0

#endif
