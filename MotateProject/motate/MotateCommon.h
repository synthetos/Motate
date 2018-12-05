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

#include <cstdint>

// NOTE: This file should not have any non-system #includes, to avoid additional dependencies.

namespace Motate {
    struct Interrupt {
        using Type                    = uint16_t;
        static constexpr Type Off = 0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr Type Unknown = 0;

        // TxRead means more data *can* be sent
        static constexpr Type OnTxReady       = 1 << 0;
        static constexpr Type OnTransmitReady = 1 << 0;
        // TxDone means all data requested to be sent has been
        static constexpr Type OnTxDone       = 1 << 1;
        static constexpr Type OnTransmitDone = 1 << 1;
        // TxError means a miscellaneous tx error
        static constexpr Type OnTxError       = 1 << 2;
        static constexpr Type OnTransmitError = 1 << 2;

        static constexpr Type OnRxReady      = 1 << 3;
        static constexpr Type OnReceiveReady = 1 << 3;
        static constexpr Type OnRxDone       = 1 << 4;
        static constexpr Type OnReceiveDone  = 1 << 4;

        static constexpr Type OnRxError      = 1 << 5;
        static constexpr Type OnReceiveError = 1 << 5;

        static constexpr Type OnTxTransferDone = 1 << 6;
        static constexpr Type OnRxTransferDone = 1 << 7;

        // Leave bits 9-10 for use in child classes

        /* Set priority levels here as well: */
        static constexpr Type PriorityHighest = 1 << 11;
        static constexpr Type PriorityHigh    = 1 << 12;
        static constexpr Type PriorityMedium  = 1 << 13;
        static constexpr Type PriorityLow     = 1 << 14;
        static constexpr Type PriorityLowest  = 1 << 15;
    };

    class InterruptCause {
        protected:

        union {
            Interrupt::Type value_;
#ifdef IN_DEBUGGER
            struct {
                uint16_t tx_ready : 1;
                uint16_t tx_done : 1;
                uint16_t tx_err : 1;

                uint16_t rx_ready : 1;
                uint16_t rx_done : 1;
                uint16_t rx_err : 1;

                uint16_t tx_trans_done : 1;
                uint16_t rx_trans_done : 1;
            };
#endif
        };
       public:
        InterruptCause(const Interrupt::Type& c = 0) : value_{c} { ; }

        void clear() { value_ = 0; }

        bool isEmpty() const { return 0 == value_; }

        bool isTxReady() const { return value_ & Interrupt::OnTxReady; }
        void setTxReady() { value_ |= Interrupt::OnTxReady; }
        void clearTxReady() { value_ &= ~Interrupt::OnTxReady; }

        bool isTxDone() const { return value_ & Interrupt::OnTxDone; }
        void setTxDone() { value_ |= Interrupt::OnTxDone; }
        void clearTxDone() { value_ &= ~Interrupt::OnTxDone; }

        bool isTxError() const { return value_ & Interrupt::OnTxError; }
        void setTxError() { value_ |= Interrupt::OnTxError; }
        void clearTxError() { value_ &= ~Interrupt::OnTxError; }

        bool isRxReady() const { return value_ & Interrupt::OnRxReady; }
        void setRxReady() { value_ |= Interrupt::OnRxReady; }
        void clearRxReady() { value_ &= ~Interrupt::OnRxReady; }

        bool isRxError() const { return value_ & Interrupt::OnRxError; }
        void setRxError() { value_ |= Interrupt::OnRxError; }
        void clearRxError() { value_ &= ~Interrupt::OnRxError; }

        bool isTxTransferDone() const { return value_ & Interrupt::OnTxTransferDone; }
        void setTxTransferDone() { value_ |= Interrupt::OnTxTransferDone; }
        void clearTxTransferDone() { value_ &= ~Interrupt::OnTxTransferDone; }

        bool isRxTransferDone() const { return value_ & Interrupt::OnRxTransferDone; }
        void setRxTransferDone() { value_ |= Interrupt::OnRxTransferDone; }
        void clearRxTransferDone() { value_ &= ~Interrupt::OnRxTransferDone; }
    };


    // The problem, in a nutshell:
    // * We have a define that looks like:
    //   #define UART       ((Uart   *)0x400E0800U)
    // * We want a constexpr variable of that value
    // * The C-style cast becomes a reintepret_cast<>, which is not allowed in a constexpr variable
    //    (see https://en.cppreference.com/w/cpp/language/constant_expression#Converted_constant_expression)
    // * We can cast in a constexpr function, however
    // 1) We need to strip the cast off, so that we can make a std::intptr_t variable of just the address
    // 2) Then we store that in an object that has a constexpr function operator overload for that type
    // 3) We can then store that object in a constexpr variable

    // Use STRIP_OFF_CAST(x) where x is a define that looks like ((Xxx *)0xDEADBEEFU)
    // The path looks like this:
    // #define BEEF0 ((Xxx *)0xDEADBEEFU)
    // constexpr std::intptr_t Beef0_addr = STRIP_OFF_CAST(BEEF0);
    // becomes: constexpr std::intptr_t Beef0_addr = _STRIP_LEVEL_I((Xxx *)0xDEADBEEFU);
    // becomes: constexpr std::intptr_t Beef0_addr = _STRIP_CAST_OFF(Xxx *)0xDEADBEEFU;
    // becomes: constexpr std::intptr_t Beef0_addr = 0xDEADBEEFU;
    #define _STRIP_CAST_OFF(x)
    #define _STRIP_LEVEL_I(x) _STRIP_CAST_OFF x
    #define STRIP_OFF_CAST(x) _STRIP_LEVEL_I x

    // Now if we want the output to be the type, a comma, then the value, we use:
    // constexpr RegisterPtr<SEPARATE_OFF_CAST(BEEF0)> Beef0_reg;
    // becomes: constexpr RegisterPtr<Xxx *, 0xDEADBEEFU> Beef0_reg;
    #define _SEPARATE_CAST_OFF(x) x ,
    #define _SEPARATE_LEVEL_I(x) _SEPARATE_CAST_OFF x
    #define SEPARATE_OFF_CAST(x) _SEPARATE_LEVEL_I x

    template<class T, std::intptr_t val>
    struct RegisterPtr {
        constexpr T const value() const { return reinterpret_cast<T>(val); }
        constexpr T const operator->() const { return reinterpret_cast<T>(val); }
        constexpr operator T const() const { return reinterpret_cast<T>(val); }
    };

} // namespace Motate

#endif //MOTATECOMMON_H_ONCE
