/*
  MotateServiceCall.hpp - Library for the Motate system
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

#ifndef MOTATESERVICECALL_H_ONCE
#define MOTATESERVICECALL_H_ONCE


namespace Motate {

struct ServiceCallEventHandler {
    virtual void handleServiceCallEvent();
};

}  // namespace Motate

#include <ProcessorServiceCall.h>

namespace Motate {

struct ServiceCall final : ServiceCallEvent {
    ServiceCall(){};

    // Interface that makes sense for this object...
    void call() { _call_or_queue(); };

    // Interface for compatibility with Pins and Timers....
    void setInterruptPending() { call(); };

    void setInterrupts(const uint32_t interrupts) {
        _interrupt_level = interrupts;
        if (_interrupt_level & kInterruptPriorityHighest) {
            _priority_value = 0;
        } else if (_interrupt_level & kInterruptPriorityHigh) {
            _priority_value = 1;
        } else if (_interrupt_level & kInterruptPriorityMedium) {
            _priority_value = 2;
        } else if (_interrupt_level & kInterruptPriorityLow) {
            _priority_value = 3;
        } else if (_interrupt_level & kInterruptPriorityLowest) {
            _priority_value = 4;
        }
    };

    // Stub to match the interface of Timer
    uint16_t getInterruptCause() { return 0; };

    void setInterruptHandler(ServiceCallEventHandler* handler) { handler_ = handler; }
};

} // namespace Motate

#endif /* end of include guard: MOTATESERVICECALL_H_ONCE */
