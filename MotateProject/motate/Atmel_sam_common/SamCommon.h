/*
 utility/SamCommon.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2016 Robert Giseburt

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

#ifndef SAMCOMMON_H_ONCE
#define SAMCOMMON_H_ONCE

#include "sam.h"

namespace Motate {

// HERE we do a stupid anti-#define dance, since these defines screw EVERYTHING up
// Also note that we are defining global (inside the Motate namespace) variables
// from a .h file, so they must be (in order of most-preferred to least-preferred):
// constexpr, static, don't, DON't, I mean it, a define

#ifdef UART0
    // This is for the Sam4e
    constexpr Uart * const UART0_DONT_CONFLICT = UART0;
    #undef UART0
    constexpr Uart * const UART0 = UART0_DONT_CONFLICT;
    #define HAS_UART0

    constexpr uint16_t const ID_UART0_DONT_CONFLICT = ID_UART0;
    #undef ID_UART0
    constexpr uint16_t const ID_UART0 = ID_UART0_DONT_CONFLICT;

#ifdef PDC_UART0
    constexpr Pdc * const PDC_UART0_DONT_CONFLICT = PDC_UART0;
    #undef PDC_UART0
    constexpr Pdc * const PDC_UART0 = PDC_UART0_DONT_CONFLICT;
    #define HAS_PDC
    #define HAS_PDC_UART0
#endif // PDC_UART0

#else
#ifdef UART
    // NOTE: We homogenize, and move the names UART -> UART0

    // This is for the Sam3x
    constexpr Uart * const UART0_DONT_CONFLICT = UART;
    #undef UART
    constexpr Uart * const UART0 = UART0_DONT_CONFLICT;
    #define HAS_UART0

    constexpr Uart * const UART1 = nullptr;
    constexpr uint32_t ID_UART0 = ID_UART;
    #undef ID_UART

    // BONUS, make fake ID_UART1 to save ifdefs
    constexpr uint32_t ID_UART1 = 0;

#ifdef PDC_UART
    constexpr Pdc * const PDC_UART0_DONT_CONFLICT = PDC_UART;
    #undef PDC_UART
    constexpr Pdc * const PDC_UART0 = PDC_UART0_DONT_CONFLICT;
    #define HAS_PDC
    #define HAS_PDC_UART0

    // BONUS, make fake PDC_UART1 to save ifdefs
    constexpr Pdc * const PDC_UART1 = nullptr;
#endif // PDC_UART

    constexpr IRQn_Type UART0_IRQn = UART_IRQn;
    constexpr IRQn_Type UART1_IRQn = (IRQn_Type)0u;
#endif // ifdef UART
#endif // ifdef UART0

#ifdef UART1
    constexpr Uart * const UART1_DONT_CONFLICT = UART1;
    #undef UART1
    constexpr Uart * const UART1 = UART1_DONT_CONFLICT;
    #define HAS_UART1

    constexpr uint32_t const ID_UART1_DONT_CONFLICT = ID_UART1;
    #undef ID_UART1
    constexpr uint32_t const ID_UART1 = ID_UART1_DONT_CONFLICT;

#ifdef PDC_UART1
    constexpr Pdc * const PDC_UART1_DONT_CONFLICT = PDC_UART1;
    #undef PDC_UART1
    constexpr Pdc * const PDC_UART1 = PDC_UART1_DONT_CONFLICT;
    #define HAS_PDC
    #define HAS_PDC_UART1
#endif // PDC_UART1

#endif

#ifdef USART0
    // Thi isn't strictly necessary, but preventative and for consistency.
    constexpr Usart * const USART0_DONT_CONFLICT = USART0;
    #undef USART0
    constexpr Usart * const USART0 = USART0_DONT_CONFLICT;
    #define HAS_USART0

    constexpr uint32_t const ID_USART0_DONT_CONFLICT = ID_USART0;
    #undef ID_USART0
    constexpr uint32_t const ID_USART0 = ID_USART0_DONT_CONFLICT;

    
#ifdef PDC_USART0
    constexpr Pdc * const PDC_USART0_DONT_CONFLICT = PDC_USART0;
    #undef PDC_USART0
    constexpr Pdc * const PDC_USART0 = PDC_USART0_DONT_CONFLICT;
    #define HAS_PDC
    #define HAS_PDC_USART0
#endif // PDC_UART1

#endif // ifdef USART0

#ifdef USART1
    // Thi isn't strictly necessary, but preventative and for consistency.
    constexpr Usart * const USART1_DONT_CONFLICT = USART1;
    #undef USART1
    constexpr Usart * const USART1 = USART1_DONT_CONFLICT;
    #define HAS_USART1

    constexpr uint32_t const ID_USART1_DONT_CONFLICT = ID_USART1;
    #undef ID_USART1
    constexpr uint32_t const ID_USART1 = ID_USART1_DONT_CONFLICT;

    
#ifdef PDC_USART1
    constexpr Pdc * const PDC_USART1_DONT_CONFLICT = PDC_USART1;
    #undef PDC_USART1
    constexpr Pdc * const PDC_USART1 = PDC_USART1_DONT_CONFLICT;
    #define HAS_PDC
    #define HAS_PDC_USART1
#endif // PDC_UART1
#endif // ifdef USART1

// Enable -Wunused warning again
#pragma GCC diagnostic pop
    
    struct SamCommon {

        // ToDo: Make this inherited! It's repeated in timer, pins, USB, and SPI.
        static void enablePeripheralClock(uint32_t peripheralId) {
            if (peripheralId < 32) {
                uint32_t id_mask = 1u << ( peripheralId );
                if ((PMC->PMC_PCSR0 & id_mask) != id_mask) {
                    PMC->PMC_PCER0 = id_mask;
                }
#if (SAM3S || SAM3XA || SAM4S || SAM4E || SAM4C || SAM4CM || SAM4CP || SAMG55 || SAMV71 || SAMV70 || SAME70 || SAMS70)
            } else {
                uint32_t id_mask = 1u << ( peripheralId - 32 );
                if ((PMC->PMC_PCSR1 & id_mask) != id_mask) {
                    PMC->PMC_PCER1 = id_mask;
                }
#endif
            }
        };

        static void disablePeripheralClock(uint32_t peripheralId) {
            if (peripheralId < 32) {
                uint32_t id_mask = 1u << ( peripheralId );
                if ((PMC->PMC_PCSR0 & id_mask) == id_mask) {
                    PMC->PMC_PCDR0 = id_mask;
                }
#if (SAM3S || SAM3XA || SAM4S || SAM4E || SAM4C || SAM4CM || SAM4CP || SAMG55 || SAMV71 || SAMV70 || SAME70 || SAMS70)
            } else {
                uint32_t id_mask = 1u << ( peripheralId - 32 );
                if ((PMC->PMC_PCSR1 & id_mask) == id_mask) {
                    PMC->PMC_PCDR1 = id_mask;
                }
#endif
            }
        };
    };

} // namespace Motate

#endif /* end of include guard: SAMCOMMON_H_ONCE */