/*
 MotateCommon.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2016 Robert Giseburt

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

#ifndef MOTATECOMMON_H_ONCE
#define MOTATECOMMON_H_ONCE

// NOTE: This file should not have any non-system #includes, to avoid additional dependencies.

namespace Motate {
    struct Interrupt {
        static constexpr uint16_t Off              = 0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr uint16_t Unknown           = 0;

        static constexpr uint16_t OnTxReady         = 1<<1;
        static constexpr uint16_t OnTransmitReady   = 1<<1;
        static constexpr uint16_t OnTxDone          = 1<<1;
        static constexpr uint16_t OnTransmitDone    = 1<<1;

        static constexpr uint16_t OnRxReady         = 1<<2;
        static constexpr uint16_t OnReceiveReady    = 1<<2;
        static constexpr uint16_t OnRxDone          = 1<<2;
        static constexpr uint16_t OnReceiveDone     = 1<<2;

        static constexpr uint16_t OnTxTransferDone  = 1<<3;
        static constexpr uint16_t OnRxTransferDone  = 1<<4;

        /* Set priority levels here as well: */
        static constexpr uint16_t PriorityHighest   = 1<<5;
        static constexpr uint16_t PriorityHigh      = 1<<6;
        static constexpr uint16_t PriorityMedium    = 1<<7;
        static constexpr uint16_t PriorityLow       = 1<<8;
        static constexpr uint16_t PriorityLowest    = 1<<9;
    };
} // namespace Motate

#endif //MOTATECOMMON_H_ONCE
