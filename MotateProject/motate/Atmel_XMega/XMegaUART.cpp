/*
 XMegaUART.h - Library for the Motate system
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

#include "Atmel_XMega/XMegaUART.h"

// See the explanation of MY_ISR in XMegaPins.cpp

#  define MY_ISR(vector, ...) \
    extern "C" void vector (void) __attribute__ ((used, externally_visible)) __VA_ARGS__; \
    void vector (void)

namespace Motate {
    template<>
    _UARTHardwareProxy* _UARTHardware<0>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<0>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<0>::uart_proxy = USARTC0;

    template<>
    _UARTHardwareProxy* _UARTHardware<1>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<1>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<1>::uart_proxy = USARTC1;

    template<>
    _UARTHardwareProxy* _UARTHardware<2>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<2>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<2>::uart_proxy = USARTD0;

    template<>
    _UARTHardwareProxy* _UARTHardware<3>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<3>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<3>::uart_proxy = USARTD1;

    template<>
    _UARTHardwareProxy* _UARTHardware<4>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<4>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<4>::uart_proxy = USARTE0;

    template<>
    _UARTHardwareProxy* _UARTHardware<5>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<5>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<5>::uart_proxy = USARTE1;

    template<>
    _UARTHardwareProxy* _UARTHardware<6>::proxy = nullptr;
    template<>
    uint16_t _UARTHardware<6>::_txReadyPriority = 0;
    template<>
    USART_t& _UARTHardware<6>::uart_proxy = USARTF0;
    // This is odd -- USARTF1 is defined in the header, even has an address, but the
    // datasheets agree that there's not USARTF1, even on the newer A3U. Oh well.
    //    template<>
    //    _UARTHardwareProxy* _UARTHardware<7>::proxy;
    //    template<>
    //    uint16_t _UARTHardware<7>::_txReadyPriority = 0;
    //    template<>
    //    USART_t& _UARTHardware<7>::uart_proxy = USARTF1;
}

#define _MAKE_MOTATE_UART_ISRS(hwNum, uLetter, uNum) \
 \
    ISR(USART##uLetter##uNum##_RXC_vect) \
    { \
        if (Motate::_UARTHardware<hwNum>::proxy != 0) { \
            Motate::_UARTHardware<hwNum>::proxy->uartInterruptHandler(Motate::UARTInterrupt::OnRxReady); \
            return; \
        } \
        else { \
            do {} while(1); \
        } \
    } \
 \
    ISR(USART##uLetter##uNum##_TXC_vect) \
    { \
        if (Motate::_UARTHardware<hwNum>::proxy != 0) { \
            Motate::_UARTHardware<hwNum>::proxy->uartInterruptHandler(Motate::UARTInterrupt::OnTxDone); \
            return; \
        } \
        else { \
            do {} while(1); \
        } \
    } \
 \
    ISR(USART##uLetter##uNum##_DRE_vect) \
    { \
        if (Motate::_UARTHardware<hwNum>::proxy != 0) { \
            Motate::_UARTHardware<hwNum>::proxy->uartInterruptHandler(Motate::UARTInterrupt::OnTxReady); \
            return; \
        } \
        else { \
            do {} while(1); \
        } \
    }

_MAKE_MOTATE_UART_ISRS(0, C, 0);
_MAKE_MOTATE_UART_ISRS(1, C, 1);
_MAKE_MOTATE_UART_ISRS(2, D, 0);
_MAKE_MOTATE_UART_ISRS(3, D, 1);
_MAKE_MOTATE_UART_ISRS(4, E, 0);
_MAKE_MOTATE_UART_ISRS(5, E, 1);
_MAKE_MOTATE_UART_ISRS(6, F, 0);

#endif // __AVR_XMEGA__
