/*
 SamUARTInternal.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2018 Robert Giseburt

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

#ifndef SAMUART_H_ONCE
#error This file should ONLY be included from SamUART.h, and never included directly
#endif

// for the IDE to pickup these defines
#include "SamCommon.h" // pull in defines and fix them

namespace Motate::UART_internal {

#pragma mark UARTInfo<n> definitions

    template<int8_t uartPeripheralNumber>
    struct UARTInfo {
        static constexpr bool exists = false;
    }; // UARTInfo <generic>


    #ifdef UART0
    // This is for the Sam4e
    #define HAS_UART0

    template<>
    struct UARTInfo<0>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART0)> uart { };

        static constexpr uint32_t peripheralId = ID_UART0;
        static constexpr IRQn_Type IRQ = UART0_IRQn;

        #ifdef PDC_UART0
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART0)> pdc { };
        #endif  // PDC_UART0
    }; // UARTInfo <0>
    #undef UART0
    #undef ID_UART0
    #undef PDC_UART0

    #else
    #ifdef UART
    // This is for the Sam3x

    template<>
    struct UARTInfo<0>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART)> uart { };

        static constexpr uint32_t peripheralId = ID_UART;
        static constexpr IRQn_Type IRQ = UART_IRQn;

        #ifdef PDC_UART0
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART)> pdc { };
        #endif  // PDC_UART0
    }; // UARTInfo <0>
    #undef UART
    #undef ID_UART
    #undef PDC_UART

    #endif  // ifdef UART
    #endif  // ifdef UART0 (else)

    #ifdef UART1
    template<>
    struct UARTInfo<1>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART1)> uart { };

        static constexpr uint32_t peripheralId = ID_UART1;
        static constexpr IRQn_Type IRQ = UART1_IRQn;

        #ifdef PDC_UART1
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART1)> pdc { };
        #endif  // PDC_UART1
    }; // UARTInfo <1>
    #undef UART1
    #undef ID_UART1
    #undef PDC_UART1
    #endif // UART1

    #ifdef UART2
    template<>
    struct UARTInfo<2>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART2)> uart { };

        static constexpr uint32_t peripheralId = ID_UART2;
        static constexpr IRQn_Type IRQ = UART2_IRQn;

        #ifdef PDC_UART2
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART2)> pdc { };
        #endif  // PDC_UART2
    }; // UARTInfo <2>
    #undef UART2
    #undef ID_UART2
    #undef PDC_UART2
    #endif // UART2

    #ifdef UART3
    template<>
    struct UARTInfo<3>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART3)> uart { };

        static constexpr uint32_t peripheralId = ID_UART3;
        static constexpr IRQn_Type IRQ = UART3_IRQn;

        #ifdef PDC_UART3
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART3)> pdc { };
        #endif  // PDC_UART3
    }; // UARTInfo <3>
    #undef UART3
    #undef ID_UART3
    #undef PDC_UART3
    #endif // UART3

    #ifdef UART4
    template<>
    struct UARTInfo<4>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(UART4)> uart { };

        static constexpr uint32_t peripheralId = ID_UART4;
        static constexpr IRQn_Type IRQ = UART4_IRQn;

        #ifdef PDC_UART4
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_UART4)> pdc { };
        #endif  // PDC_UART4
    }; // UARTInfo <4>
    #undef UART4
    #undef ID_UART4
    #undef PDC_UART4
    #endif // UART4

    static constexpr Uart * const uart(const uint8_t uartPeripheralNumber) {
        switch (uartPeripheralNumber) {
            case (0): return UARTInfo<0>::uart;
            case (1): return UARTInfo<1>::uart;
            case (2): return UARTInfo<2>::uart;
            case (3): return UARTInfo<3>::uart;
            case (4): return UARTInfo<4>::uart;
        };
        return nullptr;
    };

#pragma mark USARTInfo<n> definitions

    template<int8_t usartPeripheralNumber>
    struct USARTInfo {
        static constexpr bool exists = false;
    }; // USARTInfo <generic>


    #ifdef USART0
    template<>
    struct USARTInfo<0>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(USART0)> usart { };

        static constexpr uint32_t peripheralId = ID_USART0;
        static constexpr IRQn_Type IRQ = USART0_IRQn;

        #ifdef PDC_USART0
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_USART0)> pdc { };
        #endif  // PDC_USART0
    }; // USARTInfo <0>
    #undef USART0
    #undef ID_USART0
    #undef PDC_USART0
    #endif // USART0


    #ifdef USART1

    template<>
    struct USARTInfo<1>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(USART1)> usart { };

        static constexpr uint32_t peripheralId = ID_USART1;
        static constexpr IRQn_Type IRQ = USART1_IRQn;

        #ifdef PDC_USART1
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_USART1)> pdc { };
        #endif  // PDC_USART1
    }; // USARTInfo <1>
    #undef USART1
    #undef ID_USART1
    #undef PDC_USART1
    #endif  // ifdef USART1

    #ifdef USART2
    template<>
    struct USARTInfo<2>
    {
        static constexpr bool exists = true;

        static constexpr RegisterPtr<SEPARATE_OFF_CAST(USART2)> usart { };

        static constexpr uint32_t peripheralId = ID_USART2;
        static constexpr IRQn_Type IRQ = USART2_IRQn;

        #ifdef PDC_USART2
        static constexpr bool has_pdc = true;
        static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_USART2)> pdc { };
        #endif  // PDC_USART2
    }; // USARTInfo <2>
    #undef USART2
    #undef ID_USART2
    #undef PDC_USART2
    #endif // USART2
} // namespace Motate::UART_internal
