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

#include <sam.h>
#include "SamServiceCall.h"
#include "SamCommon.h"

extern "C" {
    void _null_svc_call_interrupt() __attribute__ ((unused));
    void _null_svc_call_interrupt() {};
}

namespace Motate {
    //volatile uint32_t _internal_pendsv_handler_number = 0;
    ServiceCallEvent * volatile ServiceCallEvent::_first_service_call = nullptr;

    // We'll support just ten for now. These take up space when not using LTO.
    template<> void ServiceCall<  0 >::interrupt() __attribute__ ((weak));
    template<> void ServiceCall<  0 >::interrupt() {;}
    //template<> uint32_t ServiceCall<  0 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  0 >::_altInterruptHandler {};

    template<> void ServiceCall<  1 >::interrupt() __attribute__ ((weak));
    template<> void ServiceCall<  1 >::interrupt() {;}
    //template<> uint32_t ServiceCall<  1 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  1 >::_altInterruptHandler {};

    template<> void ServiceCall<  2 >::interrupt() __attribute__ ((weak));
    template<> void ServiceCall<  2 >::interrupt() {;}
    //template<> uint32_t ServiceCall<  2 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  2 >::_altInterruptHandler {};

    template<> void ServiceCall<  3 >::interrupt() __attribute__ ((weak));
    template<> void ServiceCall<  3 >::interrupt() {;}
    //template<> uint32_t ServiceCall<  3 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  3 >::_altInterruptHandler {};

    //template<> void ServiceCall<  4 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  4 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  4 >::_altInterruptHandler {};

    //template<> void ServiceCall<  5 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  5 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  5 >::_altInterruptHandler {};

    //template<> void ServiceCall<  6 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  6 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  6 >::_altInterruptHandler {};

    //template<> void ServiceCall<  7 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  7 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  7 >::_altInterruptHandler {};

    //template<> void ServiceCall<  8 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  8 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  8 >::_altInterruptHandler {};

    //template<> void ServiceCall<  9 >::interrupt() __attribute__ ((weak));
    //template<> uint32_t ServiceCall<  9 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall<  9 >::_altInterruptHandler {};

    //template<> void ServiceCall< 10 >::interrupt() __attribute__ ((weak));//, alias("_null_svc_call_interrupt")
    //template<> uint32_t ServiceCall< 10 >::_interrupt_level = 0;
    //template<> std::function<void(void)> ServiceCall< 10 >::_altInterruptHandler {};
}

#if 0
void PendSV_Handler() {

#define _temp_call_handler(n) \
if (Motate::_internal_pendsv_handler_number == n) {\
    if (Motate::ServiceCall< n >::interrupt) {\
        Motate::ServiceCall< n >::interrupt();\
    } else {\
        Motate::ServiceCall< n >::alternate_interrupt();\
    }\
}
    Motate::SamCommon::sync();

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
#endif


void PendSV_Handler() {
    Motate::SamCommon::sync();
    if (Motate::ServiceCallEvent::_first_service_call) {
        Motate::ServiceCallEvent::_pop()->_call();
    }
}
