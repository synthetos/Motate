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

namespace Motate {

    _MAKE_MOTATE_PIN( 110, C, 'C', 0);	// PWM_0
    _MAKE_MOTATE_PIN( 111, C, 'C', 1);	// PWM_1
    _MAKE_MOTATE_PIN( 112, C, 'C', 2);	// PWM_2
    _MAKE_MOTATE_PIN( 113, C, 'C', 3);	// PWM_3
    _MAKE_MOTATE_PIN( 114, C, 'C', 4);	// PWM_4
    _MAKE_MOTATE_PIN( 115, C, 'C', 5);	// PWM_5
    _MAKE_MOTATE_PIN( 116, D, 'D', 0);	// PWM_6
    _MAKE_MOTATE_PIN( 117, D, 'D', 1);	// PWM_7
    _MAKE_MOTATE_PIN( 118, D, 'D', 2);	// PWM_8
    _MAKE_MOTATE_PIN( 119, D, 'D', 3);	// PWM_9
    _MAKE_MOTATE_PIN( 120, D, 'D', 4);	// PWM_10
    _MAKE_MOTATE_PIN( 121, D, 'D', 5);	// PWM_11
    _MAKE_MOTATE_PIN( 122, E, 'E', 0);	// PWM_12
    _MAKE_MOTATE_PIN( 123, E, 'E', 1);	// PWM_13
    _MAKE_MOTATE_PIN( 124, E, 'E', 2);	// PWM_14
    _MAKE_MOTATE_PIN( 125, E, 'E', 3);	// PWM_15
    _MAKE_MOTATE_PIN( 126, E, 'E', 4);	// PWM_16
    _MAKE_MOTATE_PIN( 127, E, 'E', 5);	// PWM_17 + COOLANT_LED
    _MAKE_MOTATE_PIN( 128, F, 'F', 0);	// PWM_18
    _MAKE_MOTATE_PIN( 129, F, 'F', 1);	// PWM_19
    _MAKE_MOTATE_PIN( 130, F, 'F', 2);	// PWM_20
    _MAKE_MOTATE_PIN( 131, F, 'F', 3);	// PWM_21


    _MAKE_MOTATE_PIN( 150, A, 'A', 5);	// SPINDLE_LED

    _MAKE_MOTATE_PIN( 151, A, 'A', 6);	// X_MIN

    // *******************************
    // PWM Pins -- note that for each of these, there must be a "plain" pin above defined

    _MAKE_MOTATE_PWM_PIN( 'C',  0, Timer<0>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'C',  1, Timer<0>, /*Channel:*/ 1, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'C',  2, Timer<0>, /*Channel:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'C',  3, Timer<0>, /*Channel:*/ 3, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'C',  4, Timer<1>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'C',  5, Timer<1>, /*Channel:*/ 1, /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN( 'D',  0, Timer<2>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'D',  1, Timer<2>, /*Channel:*/ 1, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'D',  2, Timer<2>, /*Channel:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'D',  3, Timer<2>, /*Channel:*/ 3, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'D',  4, Timer<3>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'D',  5, Timer<3>, /*Channel:*/ 1, /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN( 'E',  0, Timer<4>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'E',  1, Timer<4>, /*Channel:*/ 1, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'E',  2, Timer<4>, /*Channel:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'E',  3, Timer<4>, /*Channel:*/ 3, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'E',  4, Timer<5>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'E',  5, Timer<5>, /*Channel:*/ 1, /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN( 'F',  0, Timer<6>, /*Channel:*/ 0, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'F',  1, Timer<6>, /*Channel:*/ 1, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'F',  2, Timer<6>, /*Channel:*/ 2, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN( 'F',  3, Timer<6>, /*Channel:*/ 3, /*Inverted:*/ false);


    // *******************************
    // USART Pins -- note that for each of these, there must be a "plain" pin above defined
    // Note that the first number is NOT the "Pin Number", but the internal usart number.
    // These should never have to change, but may have to be commented out if the pins are
    // not defined above.

    _MAKE_MOTATE_UART_RX_PIN( 0, 'C', 2);
    _MAKE_MOTATE_UART_TX_PIN( 0, 'C', 3);
//    _MAKE_MOTATE_UART_RX_PIN( 1, 'C', 6);
//    _MAKE_MOTATE_UART_TX_PIN( 1, 'C', 7);
//    _MAKE_MOTATE_UART_RX_PIN( 2, 'D', 2);
//    _MAKE_MOTATE_UART_TX_PIN( 2, 'D', 3);
//    _MAKE_MOTATE_UART_RX_PIN( 3, 'D', 6);
//    _MAKE_MOTATE_UART_TX_PIN( 3, 'D', 7);
//    _MAKE_MOTATE_UART_RX_PIN( 4, 'E', 2);
//    _MAKE_MOTATE_UART_TX_PIN( 4, 'E', 3);
//    _MAKE_MOTATE_UART_RX_PIN( 5, 'E', 6);
//    _MAKE_MOTATE_UART_TX_PIN( 5, 'E', 7);
//    _MAKE_MOTATE_UART_RX_PIN( 6, 'F', 2);
//    _MAKE_MOTATE_UART_TX_PIN( 6, 'F', 3);

} // namespace Motate


// We're putting this in to make the autocomplete work for XCode,
// since it doesn't understand the special syntax coming up.
#ifdef XCODE_INDEX
#include <TinyGv8-pinout.h>
#endif

#ifdef MOTATE_BOARD
#define MOTATE_BOARD_PINOUT < MOTATE_BOARD-pinout.h >
#include MOTATE_BOARD_PINOUT
#else
#error Unknown board layout
#endif

#endif

// motate_pin_assignments_h
