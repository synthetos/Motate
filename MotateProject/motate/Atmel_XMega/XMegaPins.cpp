/*
 XMegaPins.cpp - Library for the Arduino-compatible Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2015 Robert Giseburt

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


#if 1

extern "C" void _null_pin_interrupt() {};

#if 1

#define _MAKE_MOTATE_PIN_INTERRUPTS(portLtr, portChr) \
template<> const PORT_t& Port8<portChr>::port_proxy = PORT##portLtr; \
namespace Motate { \
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
ISR(PORT##portLtr##_INT0_vect) { \
    if (LookupIRQPin<portChr,0>::number != -1 && _IRQPin<portChr,0>::interrupt) { \
        _IRQPin<portChr,0>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,1>::number != -1 && _IRQPin<portChr,1>::interrupt) { \
        _IRQPin<portChr,1>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,2>::number != -1 && _IRQPin<portChr,2>::interrupt) { \
        _IRQPin<portChr,2>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,3>::number != -1 && _IRQPin<portChr,3>::interrupt) { \
        _IRQPin<portChr,3>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,4>::number != -1 && _IRQPin<portChr,4>::interrupt) { \
        _IRQPin<portChr,4>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,5>::number != -1 && _IRQPin<portChr,5>::interrupt) { \
        _IRQPin<portChr,5>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,6>::number != -1 && _IRQPin<portChr,6>::interrupt) { \
        _IRQPin<portChr,6>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,7>::number != -1 && _IRQPin<portChr,7>::interrupt) { \
        _IRQPin<portChr,7>::interrupt(); \
    } \
}

#else

#define _MAKE_MOTATE_PIN_INTERRUPTS(portLtr, portChr) \
template<> const PORT_t& Port8<portChr>::port_proxy = PORT##portLtr; \
namespace Motate { \
    template<> void _IRQPin<portChr,0>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,0>::interrupt() {}; \
    template<> void _IRQPin<portChr,1>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,1>::interrupt() {}; \
    template<> void _IRQPin<portChr,2>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,2>::interrupt() {}; \
    template<> void _IRQPin<portChr,3>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,3>::interrupt() {}; \
    template<> void _IRQPin<portChr,4>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,4>::interrupt() {}; \
    template<> void _IRQPin<portChr,5>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,5>::interrupt() {}; \
    template<> void _IRQPin<portChr,6>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,6>::interrupt() {}; \
    template<> void _IRQPin<portChr,7>::interrupt() __attribute__ ((weak)); \
    template<> void _IRQPin<portChr,7>::interrupt() {}; \
} \
\
ISR(PORT##portLtr##_INT0_vect) { \
    if (LookupIRQPin<portChr,0>::number != -1 && _IRQPin<portChr,0>::interrupt) { \
        _IRQPin<portChr,0>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,1>::number != -1 && _IRQPin<portChr,1>::interrupt) { \
        _IRQPin<portChr,1>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,2>::number != -1 && _IRQPin<portChr,2>::interrupt) { \
        _IRQPin<portChr,2>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,3>::number != -1 && _IRQPin<portChr,3>::interrupt) { \
        _IRQPin<portChr,3>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,4>::number != -1 && _IRQPin<portChr,4>::interrupt) { \
        _IRQPin<portChr,4>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,5>::number != -1 && _IRQPin<portChr,5>::interrupt) { \
        _IRQPin<portChr,5>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,6>::number != -1 && _IRQPin<portChr,6>::interrupt) { \
        _IRQPin<portChr,6>::interrupt(); \
    } \
    if (LookupIRQPin<portChr,7>::number != -1 && _IRQPin<portChr,7>::interrupt) { \
        _IRQPin<portChr,7>::interrupt(); \
    } \
}

#endif

_MAKE_MOTATE_PIN_INTERRUPTS(A, 'A')
_MAKE_MOTATE_PIN_INTERRUPTS(B, 'B')
_MAKE_MOTATE_PIN_INTERRUPTS(C, 'C')
_MAKE_MOTATE_PIN_INTERRUPTS(D, 'D')
_MAKE_MOTATE_PIN_INTERRUPTS(E, 'E')
_MAKE_MOTATE_PIN_INTERRUPTS(F, 'F')

#endif // 0

#endif
