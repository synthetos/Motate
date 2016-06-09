/*
 utility/SamUART.h - Library for the Motate system
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

#if defined(__SAM4E8E__) || defined(__SAM4E16E__) || defined(__SAM4E8C__) || defined(__SAM4E16C__)

#include "SamUART.h"


namespace Motate {

    template<> Usart * const   _USARTHardware<0>::usart           = USART0;
    template<> const uint32_t  _USARTHardware<0>::peripheralId() { return ID_USART0; }
    template<> const IRQn_Type _USARTHardware<0>::uartIRQ         = USART0_IRQn;
    template<> std::function<void(uint16_t)> _USARTHardware<0>::_uartInterruptHandler {};

#ifdef USART1
    template<> Usart * const   _USARTHardware<1>::usart           = USART1;
    template<> const uint32_t  _USARTHardware<1>::peripheralId() { return ID_USART1; }
    template<> const IRQn_Type _USARTHardware<1>::uartIRQ         = USART1_IRQn;
    template<> std::function<void(uint16_t)> _USARTHardware<1>::_uartInterruptHandler {};
#endif

    Uart * const UART0 = UART0_DONT_CONFLICT;
    template<> Uart * const    _UARTHardware<0>::uart            = UART0;
    template<> const uint32_t  _UARTHardware<0>::peripheralId() { return ID_UART0; }
    template<> const IRQn_Type _UARTHardware<0>::uartIRQ         = UART0_IRQn;
    template<> std::function<void(uint16_t)> _UARTHardware<0>::_uartInterruptHandler {};

#ifdef UART1_DONT_CONFLICT
    Uart * const UART1 = UART1_DONT_CONFLICT;
    template<> Uart * const    _UARTHardware<1>::uart            = UART1;
    template<> const uint32_t  _UARTHardware<1>::peripheralId() { return ID_UART1; }
    template<> const IRQn_Type _UARTHardware<1>::uartIRQ         = UART1_IRQn;
    template<> std::function<void(uint16_t)> _UARTHardware<1>::_uartInterruptHandler {};
#endif
}

extern "C" void USART0_Handler(void)  {
    if (Motate::_USARTHardware<0u>::_uartInterruptHandler) {
        Motate::_USARTHardware<0u>::_uartInterruptHandler(Motate::_USARTHardware<0u>::getInterruptCause());
        return;
    }
    __asm__("BKPT");
    //while (1) ;
}

#ifdef USART1
extern "C" void USART1_Handler(void)  {
    if (Motate::_USARTHardware<1u>::_uartInterruptHandler) {
        Motate::_USARTHardware<1u>::_uartInterruptHandler(Motate::_USARTHardware<1u>::getInterruptCause());
        return;
    }
    __asm__("BKPT");
    //while (1) ;
}
#endif


extern "C" void UART0_Handler(void)  {
    if (Motate::_UARTHardware<0>::_uartInterruptHandler) {
        Motate::_UARTHardware<0>::_uartInterruptHandler(Motate::_UARTHardware<0>::getInterruptCause());
        return;
    }
    __asm__("BKPT");
    //while (1) ;
}

#ifdef UART1_DONT_CONFLICT
extern "C" void UART1_Handler(void)  {
    if (Motate::_UARTHardware<1>::_uartInterruptHandler) {
        Motate::_UARTHardware<1>::_uartInterruptHandler(Motate::_UARTHardware<1>::getInterruptCause());
        return;
    }
    __asm__("BKPT");
    //while (1) ;
}
#endif

#endif