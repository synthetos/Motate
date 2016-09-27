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

namespace Motate {
    extern uint32_t _internal_pendsv_handler_number;

    template <uint8_t svcNumber>
    struct ServiceCall {
        static uint32_t _interrupt_level;

        // Interface that makes sense for this object...
        static void call() {
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

            _internal_pendsv_handler_number = svcNumber;

            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
        };

        // Interface for compatibility with Pins and Timers....
        static void setInterruptPending() {
            call();
        }

        static void setInterrupts(const uint32_t interrupts) {
            _interrupt_level = interrupts;
        }

        // Stub to match the interface of Timer
        uint16_t getInterruptCause() {
            return 0;
        }

        // Override this to implement this call
        static void interrupt();
    };
}

#endif /* end of include guard: SAMSERVICECALL_H_ONCE */
