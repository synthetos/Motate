/*
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2013 Robert Giseburt
 * Copyright (c) 2013 Alden S. Hart Jr.
 *
 * This file is part of the Motate Library.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef motate_pin_assignments_h
#define motate_pin_assignments_h

#include <MotateTimers.h>
#include <MotatePins.h>

namespace Motate {

    // DUE
    _MAKE_MOTATE_PORT32(A, 'A');
    _MAKE_MOTATE_PORT32(B, 'B');


    // Arduino pin name & function
    _MAKE_MOTATE_PIN( 0, B, 'B',  2);	// D0, RX0

    _MAKE_MOTATE_PIN( 1, B, 'B',  1);	// D1, TX0

    _MAKE_MOTATE_PIN( 2, A, 'A', 11);	// D2,

    _MAKE_MOTATE_PIN( 3, B, 'B',  5);	// D3,

    _MAKE_MOTATE_PIN( 4, A, 'A', 10);	// D4,

    _MAKE_MOTATE_PIN( 5, A, 'A', 12);	// D5,

    _MAKE_MOTATE_PIN( 6, B, 'B',  6);	// D6,

    _MAKE_MOTATE_PIN( 7, B, 'B',  7);	// D7,

    _MAKE_MOTATE_PIN( 8, B, 'B', 10);	// D8, LED3

    _MAKE_MOTATE_PIN( 9, B, 'B', 11);	// D9,

    _MAKE_MOTATE_PIN(10, A, 'A',  5);	// D10, SPI_CS

    _MAKE_MOTATE_PIN(11, A, 'A',  7);	// D11, SPI_MOSI

    _MAKE_MOTATE_PIN(12, A, 'A',  6);	// D12, SPI_MISO

    _MAKE_MOTATE_PIN(13, B, 'B',  0);	// D13, SPI_SCK

    _MAKE_MOTATE_PIN(14, B, 'B',  4);	// D14, SDA
    _MAKE_MOTATE_PIN(15, B, 'B',  3);	// D15, SCL

    _MAKE_MOTATE_PIN(16, B, 'B',  8);	// A0, LED1
    _MAKE_MOTATE_PIN(17, B, 'B',  9);	// A1, LED2
    _MAKE_MOTATE_PIN(18, A, 'A',  8);	// A2
    _MAKE_MOTATE_PIN(19, A, 'A',  0);	// A3
    _MAKE_MOTATE_PIN(20, A, 'A',  9);	// A4
    _MAKE_MOTATE_PIN(21, B, 'B', 13);	// A5


    // Not pinned out, but assigned numbers to make compiling work:
    _MAKE_MOTATE_PIN(100, A, 'A',  1);
    _MAKE_MOTATE_PIN(101, A, 'A', 16);
    _MAKE_MOTATE_PIN(102, A, 'A', 17);
    _MAKE_MOTATE_PIN(103, A, 'A', 18);
    _MAKE_MOTATE_PIN(104, B, 'B', 14);

    // PWM Pins (in the order they are in the KL05* Reference Manual, pages 147-149 (rougly pin order):
    _MAKE_MOTATE_PWM_PIN( 'B',  6, Timer<0>, /*Channel:*/ 3, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B',  7, Timer<0>, /*Channel:*/ 2, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'A',  5, Timer<0>, /*Channel:*/ 5, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'A',  6, Timer<0>, /*Channel:*/ 4, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B',  8, Timer<0>, /*Channel:*/ 3, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B',  9, Timer<0>, /*Channel:*/ 2, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B', 10, Timer<0>, /*Channel:*/ 1, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B', 11, Timer<0>, /*Channel:*/ 0, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B',  5, Timer<1>, /*Channel:*/ 1, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'A', 12, Timer<1>, /*Channel:*/ 0, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'B', 13, Timer<1>, /*Channel:*/ 1, /*PinMux:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'A',  0, Timer<1>, /*Channel:*/ 0, /*PinMux:*/ 2, /*Inverted:*/ false);

    // Interrupt pins
    // PTA0/PTA1/PTA7/PTA10/PTA11/PTA12/PTA16/PTA17/PTA18
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A',  0 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A',  1 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A',  7 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 10 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 11 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 12 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 16 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 17 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'A', 18 );

    // PTB0/PTB1/PTB2/PTB3/PTB4/PTB5/PTB6/PTB7/PTB14
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  0 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  1 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  2 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  3 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  4 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  5 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  6 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B',  7 );
    _MAKE_MOTATE_GPIO_IRQ_PIN( 'B', 14 );

    // SPI pins
    _MAKE_MOTATE_SPI_CS_PIN( 'A',  5, /*PinMux:*/ 3);
    //	_MAKE_MOTATE_SPI_CS_PIN( 'A', 19, /*PinMux:*/ 3); // N.C.

    _MAKE_MOTATE_SPI_MISO_PIN( 'A',  6, /*PinMux:*/ 3);
    _MAKE_MOTATE_SPI_MISO_PIN( 'A',  7, /*PinMux:*/ 2);
    //	_MAKE_MOTATE_SPI_MISO_PIN( 'B', 15, /*PinMux:*/ 3); // N.C.
    //	_MAKE_MOTATE_SPI_MISO_PIN( 'B', 16, /*PinMux:*/ 2); // N.C.

    _MAKE_MOTATE_SPI_MOSI_PIN( 'A',  7, /*PinMux:*/ 3);
    //	_MAKE_MOTATE_SPI_MOSI_PIN( 'B', 15, /*PinMux:*/ 2); // N.C.
    //	_MAKE_MOTATE_SPI_MOSI_PIN( 'B', 16, /*PinMux:*/ 3); // N.C.

    _MAKE_MOTATE_SPI_SCK_PIN( 'B',  0, /*PinMux:*/ 3);
    //	_MAKE_MOTATE_SPI_SCK_PIN( 'B', 17, /*PinMux:*/ 3); // N.C.


    // UART pins
    _MAKE_MOTATE_UART_TX_PIN( 'B',  2,  /*PinMux:*/ 3); // Pin 0
    _MAKE_MOTATE_UART_RX_PIN( 'B',  2,  /*PinMux:*/ 2); // Pin 0
    
    _MAKE_MOTATE_UART_TX_PIN( 'B',  1,  /*PinMux:*/ 2); // Pin 1
    _MAKE_MOTATE_UART_RX_PIN( 'B',  1,  /*PinMux:*/ 3); // Pin 1
    
    _MAKE_MOTATE_UART_TX_PIN( 'B',  3,  /*PinMux:*/ 3); // Pin 15
    _MAKE_MOTATE_UART_RX_PIN( 'B',  4,  /*PinMux:*/ 3); // Pin 14
    
} // namespace Motate


// We're putting this in to make the autocomplete work for XCode,
// since it doesn't understand the special syntax coming up.
#ifdef XCODE_INDEX
#include <FRDM-KL05Z-pinout.h>
#endif

#ifdef MOTATE_BOARD
#define MOTATE_BOARD_PINOUT < MOTATE_BOARD-pinout.h >
#include MOTATE_BOARD_PINOUT
#else
#error Unknown board layout
#endif

#endif

// motate_pin_assignments_h
