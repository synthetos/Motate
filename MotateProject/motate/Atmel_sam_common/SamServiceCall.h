/*
 SamServiceCall.h - Library for the Motate system
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

#ifndef SAMSERVICECALL_H_ONCE
#define SAMSERVICECALL_H_ONCE

#include <sys/types.h>
#include "MotateTimers.h" // for the interrupt definitions
#include <functional>

namespace Motate {
    typedef const uint32_t service_call_number;

    //extern volatile uint32_t _internal_pendsv_handler_number;

    struct ServiceCallEvent {
        std::function<void(void)> _callback {};
        ServiceCallEvent * volatile _next = nullptr;
        volatile bool _queued = false;
        volatile bool _setup = false; // this means we've setup the interrupt and triggered the call

        static ServiceCallEvent * volatile _first_service_call; // the pointer is volatile

        uint32_t _interrupt_level = kInterruptPriorityLowest; // start at the lowest

        //ServiceCallEvent(std::function<void(void)> e) : _callback{e} {};
        //ServiceCallEvent(std::function<void(void)> &&e) : _callback{std::move(e)} {};

        void _add_to_queue() {
            if (_queued) { return; }

            _queued = true;
            _setup = false;
            _next = nullptr;

            if (_first_service_call == nullptr) {
                _first_service_call = this;
                _pend();
                return;
            }

            // This happens when we are already in a pendSV context, and we are adding a second one.
            // This can also happen if there was already a second one, and we just pre-empted it with a
            // higher-priority one.
            // IMPORTANT: HIGHER priority tasks have a LOWER number
            if (_first_service_call->_interrupt_level > _interrupt_level) {
                this->_next = _first_service_call;
                _first_service_call = this;
                _pend();
                return;
            }

            ServiceCallEvent *walker = _first_service_call;
            while (walker->_next != nullptr) {
                // put this in front of lower priority tasks
                // IMPORTANT: HIGHER priority tasks have a LOWER number
                if (walker->_next->_interrupt_level > _interrupt_level) { break; }
                walker = walker->_next;
            }
            _next = walker->_next; // in case we aren't putting it last in line
            walker->_next = this;
        };

        // This is static, it doesn't use any local variables
        static void _pend() {
            uint32_t _interrupt_level = _first_service_call->_interrupt_level;

            /* Set interrupt priority */
            if (_interrupt_level & kInterruptPriorityHighest) {
                NVIC_SetPriority(PendSV_IRQn, 0);
            }
            else if (_interrupt_level & kInterruptPriorityHigh) {
                NVIC_SetPriority(PendSV_IRQn, 3);
            }
            else if (_interrupt_level & kInterruptPriorityMedium) {
                NVIC_SetPriority(PendSV_IRQn, 7);
            }
            else if (_interrupt_level & kInterruptPriorityLow) {
                NVIC_SetPriority(PendSV_IRQn, 11);
            }
            else if (_interrupt_level & kInterruptPriorityLowest) {
                NVIC_SetPriority(PendSV_IRQn, 15);
            }

            SamCommon::sync();
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
            SamCommon::sync();
        }

        // We were queued and pended, then called.
        void _call() {
            // If we are here then we are in the interrupt context

            // pop this call off the list
            _first_service_call = _next;
            _next = nullptr;

            // Mark it as un-queued so it can be re-queued
            _queued = false;

            // ... and finally:
            if (_callback) {
                _callback();
            } else {
                interrupt();
            }

            if (_first_service_call != nullptr) {
                _pend();
            }
        };

        virtual void interrupt() {;};
    };

    template <service_call_number svcNumber>
    struct ServiceCall final : ServiceCallEvent {
        //std::function<void(void)> _altInterruptHandler;

        ServiceCall() {
            //_callback = [&](){
            //    interrupt();
            //};
            _first_service_call = nullptr;
        };

        // Interface that makes sense for this object...
        void call() {
            _add_to_queue();
        };

        // Interface for compatibility with Pins and Timers....
        void setInterruptPending() {
            call();
        };

        void setInterrupts(const uint32_t interrupts) {
            _interrupt_level = interrupts;
        };

        // Stub to match the interface of Timer
        uint16_t getInterruptCause() {
//            SCB->ICSR |= SCB_ICSR_PENDSVCLR_Msk;
//            SamCommon::sync();
            return 0;
        };

        void setInterruptHandler(std::function<void(void)> &&handler) {
            _callback = std::move(handler);
        }

        // If the main interrupt isn't overidden, then we'll this one
        //static void alternate_interrupt() {
        //    if (_altInterruptHandler) {
        //        _altInterruptHandler();
        //    }
        //};

        // Override this to implement this call
        void interrupt() override;
    };
}

#endif /* end of include guard: SAMSERVICECALL_H_ONCE */
