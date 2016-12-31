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

#include "MotateUART.h"
#include "MotateDebug.h"

namespace Motate {
    template<> std::function<void()> _USARTHardware<0>::_uartInterruptHandlerJumper {};

#ifdef HAS_USART1
    template<> std::function<void()> _USARTHardware<1>::_uartInterruptHandlerJumper {};
#endif

    template<> std::function<void()> _UARTHardware<0>::_uartInterruptHandlerJumper {};

#ifdef HAS_UART1
    template<> std::function<void()> _UARTHardware<1>::_uartInterruptHandlerJumper {};
#endif
#ifdef HAS_UART2
    template<> std::function<void()> _UARTHardware<2>::_uartInterruptHandlerJumper {};
#endif
#ifdef HAS_UART3
    template<> std::function<void()> _UARTHardware<3>::_uartInterruptHandlerJumper {};
#endif

}

#ifdef HAD_UART
extern "C" void UART_Handler(void) __attribute__ ((weak, alias("UART0_Handler")));
#endif

extern "C" void USART0_Handler(void)  {
    if (Motate::_USARTHardware<0u>::_uartInterruptHandlerJumper) {
        Motate::_USARTHardware<0u>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}

#ifdef HAS_USART1
extern "C" void USART1_Handler(void)  {
    if (Motate::_USARTHardware<1u>::_uartInterruptHandlerJumper) {
        Motate::_USARTHardware<1u>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}
#endif


extern "C" void UART0_Handler(void)  {
    if (Motate::_UARTHardware<0>::_uartInterruptHandlerJumper) {
        Motate::_UARTHardware<0>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}

#ifdef HAS_UART1
extern "C" void UART1_Handler(void)  {
    if (Motate::_UARTHardware<1>::_uartInterruptHandlerJumper) {
        Motate::_UARTHardware<1>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}
#endif

#ifdef HAS_UART2
extern "C" void UART2_Handler(void)  {
    if (Motate::_UARTHardware<2>::_uartInterruptHandlerJumper) {
        Motate::_UARTHardware<2>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}
#endif

#ifdef HAS_UART3
extern "C" void UART3_Handler(void)  {
    if (Motate::_UARTHardware<3>::_uartInterruptHandlerJumper) {
        Motate::_UARTHardware<3>::_uartInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
    //while (1) ;
}
#endif
