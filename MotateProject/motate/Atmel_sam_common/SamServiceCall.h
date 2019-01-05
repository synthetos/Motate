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
#include <functional>
#include <atomic>

#include "MotateTimers.h" // for the interrupt definitions
#include "MotateDebug.h"

// Unless debugging, this should always read "#if 0 && ..."
// DON'T COMMIT with anything else!
#if 0 && (IN_DEBUGGER == 1)
    template<int32_t len>
    void svc_call_debug(const char (&str)[len]) { Motate::debug.write(str); };
#else
    template<int32_t len>
    void svc_call_debug(const char (&str)[len]) { ; };
#endif

namespace Motate {
    typedef const uint32_t service_call_number;

    //extern volatile uint32_t _internal_pendsv_handler_number;

    struct ServiceCallEventHandler {
        virtual void handleServiceCallEvent();
    };

    struct ServiceCallEvent {
        ServiceCallEventHandler *handler_;
        std::atomic<ServiceCallEvent *> _next = nullptr;
        std::atomic<bool> _queued = false;
        std::atomic<bool> _pended = false;

        static std::atomic<ServiceCallEvent *> _first_service_call; // the pointer is volatile

        uint32_t _interrupt_level = kInterruptPriorityLowest; // start at the lowest
        // we need to convert the enum to a priority value we can compare with
        int32_t _priority_value = 4;

        void _call_or_queue() {
            if (_queued) {
                _debug_print_num(); svc_call_debug("💣");
                return;
            }

            _queued = true;
            _next = nullptr;

            // Things we know:
            //   We are already in the highest priority ServiceCall or interrrupt, or we would have been interrupted.
            //   We might get interrupted.

            // This we need to know:
            //  What is the current priority, either as an interrupt, or as BASEPRI?
            //  What is the target priority?

            // Then we can follow this plan:
            //   If we are requesting a higher priority, we raise BASEPRI and call it.
            //   If we are requesting the same priority, we queue and pend the task
            //   If we are requesting a lower priority, we queue and pend the task

            // Queuing a task:
            //   Add the item to the linked list

            // Pending a task:
            //   If the queue is empty or has only lower-priority tasks, we set the priority of pendSV and pend it

            // Handling pendSV:
            //   Pop the highest priority item off of the list, call it

            bool needs_pended = false;

            // Thanks to http://embeddedgurus.com/state-space/2014/02/cutting-through-the-confusion-with-arm-cortex-m-interrupt-priorities/
            // for explaining __get_BASEPRI and __set_BASEPRI needing shifted.

            // REMEMBER: "higher priority" means a lower number!!!
            //           0 is the highest priority!

//             int32_t current_basepri = __get_BASEPRI() >> (8 - __NVIC_PRIO_BITS);
//             int32_t effective_priority_level = current_basepri;
// //            bool in_pendsv_irq = false;

//             // get the currently highest-priority active "exception" (interrupt)
//             // see http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0552a/CHDBIBGJ.html
//             // NOTE: it's signed!
//             int32_t active_interrupt_number = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos;
//             if (active_interrupt_number != 0) {
//                 // See note in previous link: we need to subtract 16 from the interrupt number to get the IRQn_Type
//                 IRQn_Type active_irq = (IRQn_Type)(active_interrupt_number - 16);

//                 int32_t interrupt_priority_level = NVIC_GetPriority(active_irq);
//                 if (interrupt_priority_level < effective_priority_level) {
//                     effective_priority_level = interrupt_priority_level;
//                 }
// //
// //                if (PendSV_IRQn == active_irq) {
// //                    in_pendsv_irq = true;
// //                }
//             }

            // if (_priority_value < effective_priority_level) {
            //     // we are asking for a higher priority, so we elevate BASEPI, call, reset BASEPRI, and return

            //     __set_BASEPRI(_priority_value << (8 - __NVIC_PRIO_BITS));
            //     svc_call_debug("🛫");
            //     switch(_priority_value) {
            //         case 0: svc_call_debug("⇈"); break;
            //         case 1: svc_call_debug("⇡"); break;
            //         case 2: svc_call_debug("⦿"); break;
            //         case 3: svc_call_debug("⇣"); break;
            //         case 4: svc_call_debug("⇊"); break;
            //         default: svc_call_debug("?"); break;
            //     }

            //     _call();

            //     svc_call_debug("🛬");
            //     __set_BASEPRI(current_basepri << (8 - __NVIC_PRIO_BITS));

            //     return;
            // }

            // else if (_priority_value == effective_priority_level) {
            //     // we are asking for the same priority, so just call it and return
            //     _call();
            //     return;
            // }

            do {  // loop until it works
                // SamCommon::InterruptDisabler disabler;

                auto orig_first_service_call = _first_service_call.load();
                // Again, higher _priority_value is lower priority!
                if (true || orig_first_service_call == nullptr || orig_first_service_call->_priority_value > _priority_value) {
                    if (!_first_service_call.compare_exchange_weak(orig_first_service_call, this)) {
                        continue;  // something changed out from under us, try again
                    }
                    _next = orig_first_service_call;

                    _debug_print_num();
                    svc_call_debug("✂️");

                    needs_pended = true;
                } else {
                    ServiceCallEvent* walker = orig_first_service_call;
                    ServiceCallEvent* next   = nullptr;
                    while ((next = walker->_next) != nullptr) {
                        // // put this in front of lower priority tasks
                        // // Again, higher _priority_value is lower priority!
                        // if (next->_priority_value > _priority_value) {
                        //     if (walker == orig_first_service_call) {
                        //         needs_pended = true;
                        //     }
                        //     break;
                        // }
                        walker = next;
                    }

                    _next = next;
                    if (!walker->_next.compare_exchange_weak(next, this)) {
                        continue;  // something changed out from under us, try again
                    }
                    // if (walker == _first_service_call) {
                        needs_pended = true;
                    // }
                }
            } while (0);

            // If the first_service_call's priority is lower (has a higher number)
            // then we'll pend another PendSV to esure the correct level.
            if (needs_pended) {
                _pend();
            }
        };

        // We were queued and pended, then called.
        void _call() {
            _pended = false;
            #if 0
            // If we are here then we are in the interrupt context
            _debug_print_num(); svc_call_debug("☎️");

            // debug code to get priority level
            // see _pend comments for description
            // this should fold away since these aren't used anywhere.
            int32_t priority_value = 0;
            int32_t active_interrupt_number = (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) >> SCB_ICSR_VECTACTIVE_Pos;
            if (active_interrupt_number != 0) {
                IRQn_Type active_irq = (IRQn_Type)(active_interrupt_number - 16);
                priority_value = NVIC_GetPriority(active_irq);
                if (PendSV_IRQn == active_irq) {
                    svc_call_debug("🕶");
                } else {
                    svc_call_debug("👀");
                }
            } else {
                priority_value = __get_BASEPRI() >> (8 - __NVIC_PRIO_BITS);
            }

            switch(priority_value) {
                case 0: svc_call_debug("⇈"); break;
                case 1: svc_call_debug("⇡"); break;
                case 2: svc_call_debug("⦿"); break;
                case 3: svc_call_debug("⇣"); break;
                case 4: svc_call_debug("⇊"); break;
                default: svc_call_debug("?"); break;
            }
            // end debug code
            #endif

            // Mark it as un-queued so it can be re-queued
            _queued = false;

            // ... and finally:
            if (handler_) {
                handler_->handleServiceCallEvent();
            } else {
                // interrupt();
            }

            _debug_print_num(); svc_call_debug("🎉\n");
        };

        // This is called *ONLY* from the handler (in the .cpp file)
        void _call_from_handler() {
            ServiceCallEvent* first_service_call;
            while ((first_service_call = _first_service_call.load()) != nullptr) {
                if (!_first_service_call.compare_exchange_weak(first_service_call, first_service_call->_next)) {
                    continue; // it changed, try again
                }
                first_service_call->_next = nullptr;
                first_service_call->_call();

                first_service_call = _first_service_call.load();
                if (first_service_call != nullptr) {
                    first_service_call->_pend();
                }

                break;
            }
        }

        void _pend() {
            #if 0
            _debug_print_num(); svc_call_debug("✍🏻");

            switch(_priority_value) {
                case 0: svc_call_debug("⇈"); break;
                case 1: svc_call_debug("⇡"); break;
                case 2: svc_call_debug("⦿"); break;
                case 3: svc_call_debug("⇣"); break;
                case 4: svc_call_debug("⇊"); break;
                default: svc_call_debug("?"); break;
            }
            #endif

            /* Set interrupt priority */
            {
                // See erratum 837070 in "ARM Processor Cortex-M7 (AT610) and Cortex-M7 with FPU (AT611), Product revision r0, Sofware Developers Errata Notice".
                SamCommon::InterruptDisabler disabler;
                NVIC_SetPriority(PendSV_IRQn, _first_service_call.load()->_priority_value);
            }

            _pended = true;

            // see
            // http://infocenter.arm.com/help/topic/com.arm.doc.dai0321a/DAI0321A_programming_guide_memory_barriers_for_m_profile.pdf
            // Section 4.5 and 4.10. Specifically:
            // "if it is necessary to have the side-effect of the priority change recognized immediately,
            // an ISB instruction is required"
            __ISB();
            SCB->ICSR |= SCB_ICSR_PENDSVSET_Msk;
            __ISB();
        };

        virtual void _debug_print_num() {;};

        // virtual void interrupt() {;};
    };

    template <service_call_number svcNumber>
    struct ServiceCall final : ServiceCallEvent {
        ServiceCall() {
        };

        // Interface that makes sense for this object...
        void call() {
            _call_or_queue();
        };

        // Interface for compatibility with Pins and Timers....
        void setInterruptPending() {
            call();
        };

        void setInterrupts(const uint32_t interrupts) {
            _interrupt_level = interrupts;
            if (_interrupt_level & kInterruptPriorityHighest) {
                _priority_value = 0;
            }
            else if (_interrupt_level & kInterruptPriorityHigh) {
                _priority_value = 1;
            }
            else if (_interrupt_level & kInterruptPriorityMedium) {
                _priority_value = 2;
            }
            else if (_interrupt_level & kInterruptPriorityLow) {
                _priority_value = 3;
            }
            else if (_interrupt_level & kInterruptPriorityLowest) {
                _priority_value = 4;
            }
        };

        // Stub to match the interface of Timer
        uint16_t getInterruptCause() {
            return 0;
        };

        void setInterruptHandler(ServiceCallEventHandler *handler) {
            handler_ = handler;
        }

        void _debug_print_num() override {
            switch (svcNumber) {
                case 0: svc_call_debug("<0>"); break;
                case 1: svc_call_debug("<1>"); break;
                case 2: svc_call_debug("<2>"); break;
                case 3: svc_call_debug("<3>"); break;
                case 4: svc_call_debug("<4>"); break;
                case 5: svc_call_debug("<5>"); break;

                default:
                    svc_call_debug("<?>");
                    break;
            }
        };

        // If the main interrupt isn't overidden, then we'll this one
        //static void alternate_interrupt() {
        //    if (_altInterruptHandler) {
        //        _altInterruptHandler();
        //    }
        //};

        // Override this to implement this call
        // void interrupt() override;
    };
}

#endif /* end of include guard: SAMSERVICECALL_H_ONCE */
