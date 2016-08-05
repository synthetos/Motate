/*
 * motate_chip_pin_functions.h - SAM3X8x specific version
 * This file is part of the TinyG project
 *
 * Copyright (c) 2015 Robert Giseburt
 * Copyright (c) 2015 Alden S. Hart Jr.
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

#ifndef Motate_motate_chip_pin_functions_h
#define Motate_motate_chip_pin_functions_h

namespace Motate {

    // PWM Pin assignments
    // Stupid preprocessor! Workaround for the comma in the TimerChannel name:
#define _MOTATE_TEMP_TC(t,c) Motate::TimerChannel<t,c>
    _MAKE_MOTATE_PWM_PIN('A',  0, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  2, _MOTATE_TEMP_TC(1,0),            /*Peripheral:*/ A, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  3, _MOTATE_TEMP_TC(1,1),            /*Peripheral:*/ A, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  5, _MOTATE_TEMP_TC(2,0),            /*Peripheral:*/ A, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  6, _MOTATE_TEMP_TC(2,1),            /*Peripheral:*/ A, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  8, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('A',  9, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('A', 12, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 13, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('A', 19, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('A', 20, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 21, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN('B', 12, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('B', 13, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('B', 14, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('B', 15, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('B', 16, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('B', 17, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('B', 18, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('B', 19, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('B', 25, _MOTATE_TEMP_TC(0,0),            /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('B', 27, _MOTATE_TEMP_TC(0,1),            /*Peripheral:*/ B, /*Inverted:*/ false);

// SAM3X8C doesn't have a PORT C or D
#ifdef PIOC
    _MAKE_MOTATE_PWM_PIN('C',  2, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  3, Motate::PWMTimer<0>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C',  4, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  5, Motate::PWMTimer<1>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C',  6, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  7, Motate::PWMTimer<2>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C',  8, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  9, Motate::PWMTimer<3>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C', 18, Motate::PWMTimer<6>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C', 19, Motate::PWMTimer<5>, /*Peripheral:*/ B, /*Inverted:*/ true); // INVERTED!
    _MAKE_MOTATE_PWM_PIN('C', 21, Motate::PWMTimer<4>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 22, Motate::PWMTimer<5>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 23, Motate::PWMTimer<6>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 24, Motate::PWMTimer<7>, /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 25, _MOTATE_TEMP_TC(6,0),            /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 26, _MOTATE_TEMP_TC(6,1),            /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 28, _MOTATE_TEMP_TC(7,0),            /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 29, _MOTATE_TEMP_TC(7,1),            /*Peripheral:*/ B, /*Inverted:*/ false);
#endif

#ifdef PIOD
    _MAKE_MOTATE_PWM_PIN('D',  7, _MOTATE_TEMP_TC(8, 0),           /*Peripheral:*/ B, /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D',  8, _MOTATE_TEMP_TC(8, 1),           /*Peripheral:*/ B, /*Inverted:*/ false);
#endif

#undef _MOTATE_TEMP_TC//(t,c)

    // SPI Pin assignements
    _MAKE_MOTATE_SPI_CS_PIN('A', 28, /* SPINum:*/ 0, /* Peripheral:*/ A, /* CS Index:*/ 0);
    _MAKE_MOTATE_SPI_CS_PIN('B', 20, /* SPINum:*/ 0, /* Peripheral:*/ B, /* CS Index:*/ 1);
    _MAKE_MOTATE_SPI_CS_PIN('B', 21, /* SPINum:*/ 0, /* Peripheral:*/ B, /* CS Index:*/ 2);
    _MAKE_MOTATE_SPI_CS_PIN('B', 23, /* SPINum:*/ 0, /* Peripheral:*/ B, /* CS Index:*/ 3);

    _MAKE_MOTATE_SPI_MISO_PIN('A', 25, /* SPINum:*/ 0, /* Peripheral */ A);
    _MAKE_MOTATE_SPI_MOSI_PIN('A', 26, /* SPINum:*/ 0, /* Peripheral */ A);
    _MAKE_MOTATE_SPI_SCK_PIN ('A', 27, /* SPINum:*/ 0, /* Peripheral */ A);

    // UART Pin Assignments
    // NOTE: UART 4 is UART hardware
    //       UARTs 0,1 and 2 are USART0 and up (note the S in USART!)

    _MAKE_MOTATE_UART_RX_PIN( 'A', 8, /* UART number: */ 4+0, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'A', 9, /* UART number: */ 4+0, /* Peripheral */ A);

    _MAKE_MOTATE_UART_RX_PIN( 'A', 10, /* UART number: */ 0, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'A', 11, /* UART number: */ 0, /* Peripheral */ A);
    _MAKE_MOTATE_UART_RTS_PIN('B', 25, /* UART number: */ 0, /* Peripheral */ A);
    _MAKE_MOTATE_UART_CTS_PIN('B', 26, /* UART number: */ 0, /* Peripheral */ A);

    _MAKE_MOTATE_UART_RX_PIN( 'A', 12, /* UART number: */ 1, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'A', 13, /* UART number: */ 1, /* Peripheral */ A);
    _MAKE_MOTATE_UART_RTS_PIN('A', 14, /* UART number: */ 1, /* Peripheral */ A);
    _MAKE_MOTATE_UART_CTS_PIN('A', 15, /* UART number: */ 1, /* Peripheral */ A);

    _MAKE_MOTATE_UART_RX_PIN( 'B', 21, /* UART number: */ 2, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'B', 20, /* UART number: */ 2, /* Peripheral */ A);
    _MAKE_MOTATE_UART_RTS_PIN('B', 22, /* UART number: */ 2, /* Peripheral */ A);
    _MAKE_MOTATE_UART_CTS_PIN('B', 23, /* UART number: */ 2, /* Peripheral */ A);

#if defined(PIOD) && defined(PIOF)
    _MAKE_MOTATE_UART_RX_PIN( 'D',  5, /* UART number: */ 3, /* Peripheral */ B);
    _MAKE_MOTATE_UART_TX_PIN( 'D',  4, /* UART number: */ 3, /* Peripheral */ B);
    _MAKE_MOTATE_UART_RTS_PIN('F',  5, /* UART number: */ 3, /* Peripheral */ A);
    _MAKE_MOTATE_UART_CTS_PIN('F',  4, /* UART number: */ 3, /* Peripheral */ A);
#endif

    // ADC Pin assignments
    _MAKE_MOTATE_ADC_PIN('A',  2, /* ADC number:*/  0);
    _MAKE_MOTATE_ADC_PIN('A',  3, /* ADC number:*/  1);
    _MAKE_MOTATE_ADC_PIN('A',  4, /* ADC number:*/  2);
    _MAKE_MOTATE_ADC_PIN('A',  6, /* ADC number:*/  3);
    _MAKE_MOTATE_ADC_PIN('A', 16, /* ADC number:*/  7);
    _MAKE_MOTATE_ADC_PIN('A', 22, /* ADC number:*/  4);
    _MAKE_MOTATE_ADC_PIN('A', 23, /* ADC number:*/  5);
    _MAKE_MOTATE_ADC_PIN('A', 24, /* ADC number:*/  6);
    _MAKE_MOTATE_ADC_PIN('B', 12, /* ADC number:*/  8);
    _MAKE_MOTATE_ADC_PIN('B', 13, /* ADC number:*/  9);
    _MAKE_MOTATE_ADC_PIN('B', 17, /* ADC number:*/ 10);
    _MAKE_MOTATE_ADC_PIN('B', 18, /* ADC number:*/ 11);
    _MAKE_MOTATE_ADC_PIN('B', 19, /* ADC number:*/ 12);
    _MAKE_MOTATE_ADC_PIN('B', 20, /* ADC number:*/ 13);
    _MAKE_MOTATE_ADC_PIN('B', 21, /* ADC number:*/ 14);


}

#endif
