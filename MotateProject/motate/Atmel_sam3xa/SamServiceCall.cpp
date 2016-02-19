/*
 SamServiceCall.cpp - Library for the Motate system
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

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)

#include <sam.h>
#include "Atmel_sam3xa/SamServiceCall.h"

extern "C" {
    void _null_svc_call_interrupt() __attribute__ ((unused));
    void _null_svc_call_interrupt() {};
}

namespace Motate {
    uint32_t _internal_pendsv_handler_number = 0;

    // We'll support just ten for now. These take up space when not using LTO.
    template<> void ServiceCall<  0 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  0 >::_interrupt_level = 0;

    template<> void ServiceCall<  1 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  1 >::_interrupt_level = 0;

    template<> void ServiceCall<  2 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  2 >::_interrupt_level = 0;

    template<> void ServiceCall<  3 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  3 >::_interrupt_level = 0;

    template<> void ServiceCall<  4 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  4 >::_interrupt_level = 0;

    template<> void ServiceCall<  5 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  5 >::_interrupt_level = 0;

    template<> void ServiceCall<  6 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  6 >::_interrupt_level = 0;

    template<> void ServiceCall<  7 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  7 >::_interrupt_level = 0;

    template<> void ServiceCall<  8 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  8 >::_interrupt_level = 0;

    template<> void ServiceCall<  9 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall<  9 >::_interrupt_level = 0;

    template<> void ServiceCall< 10 >::interrupt() __attribute__ ((weak, alias("_null_svc_call_interrupt")));
    template<> uint32_t ServiceCall< 10 >::_interrupt_level = 0;
}

#if 0 

//-- to toss later

// Here we use some tricks that require knowledge of the ARMv7-M (for M3 processors) architecture.
// See the ARMv7-m Technical Reference Manual (TRM) PDF available from:
//   http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.ddi0403e.b/index.html
// In particular, section B1.5.8 "Exception return behavior" for description of how the LR hold in bit 2
// which stack is in use outside of rthe interrupt context, so we can tell which stack to look at for the
// SVC instruction used.

void _internal_svc_handler(uint32_t *svc_args) {
    // Now that we have the stack in svc_args, we need to get to the previous program counter to
    // get the SVC instruction. See section 2.3.7 of:
    //   http://infocenter.arm.com/help/topic/com.arm.doc.dui0552a/DUI0552A_cortex_m3_dgug.pdf
    // for the stack structure of the M3. You'll see int he picture on page 2-26 that PC is at
    // byte offset 0x18 (24), or the 6th 32-bit offset.
    // Then in the ARMv7-M TRM (linked to above) in section A6.7.136 SVC (formerly SWI) you see
    // the lower byte of the SVC instruction holds the value we want.
    // Since an instruction is two bytes in size, we want the bottom byte of the previous
    // instruction, so we use an offset of -2.

    uint32_t handler_number = ((char *)svc_args[6])[-2];

#define _temp_call_handler(n) \
if (Motate::ServiceCall< n >::interrupt && handler_number == n) {\
Motate::ServiceCall< n >::interrupt();\
}

    _temp_call_handler( 0)
    else _temp_call_handler( 1)
    else _temp_call_handler( 2)
    else _temp_call_handler( 3)
    else _temp_call_handler( 4)
    else _temp_call_handler( 5)
    else _temp_call_handler( 6)
    else _temp_call_handler( 7)
    else _temp_call_handler( 8)
    else _temp_call_handler( 9)
    else _temp_call_handler(10)
#undef _temp_call_handler
}


__attribute__ (( naked ))
void SVC_Handler() {

    /*
     * Get the pointer to the stack frame which was saved before the SVC
     * call, so we can grab the SVC call and know where to go from here.
     */
    asm volatile (
                  "tst lr, #4 \t\n"       // Check EXC_RETURN[2]
                  "ite eq \t\n"           // Set the next instructions as If equal...Else
                  "mrseq r0, msp \t\n"    // Grab the Main Stack Pointer and put it in R0
                  "mrsne r0, psp \t\n"    // Grab the Priveledged Stack Pointer and put it in R0
                  "b %[_internal_svc_handler] \n\t"
                  : // no outputs
                  : [_internal_svc_handler] "i" (_internal_svc_handler)// input is function address
                  : "r0" // clobbers
                  );

}


#else // if 0


void PendSV_Handler() {

#define _temp_call_handler(n) \
if (Motate::ServiceCall< n >::interrupt && Motate::_internal_pendsv_handler_number == n) {\
    Motate::ServiceCall< n >::interrupt();\
}

         _temp_call_handler( 0)
    else _temp_call_handler( 1)
    else _temp_call_handler( 2)
    else _temp_call_handler( 3)
    else _temp_call_handler( 4)
    else _temp_call_handler( 5)
    else _temp_call_handler( 6)
    else _temp_call_handler( 7)
    else _temp_call_handler( 8)
    else _temp_call_handler( 9)
    else _temp_call_handler(10)
#undef _temp_call_handler

}

#endif // if 0 .. else

#endif
