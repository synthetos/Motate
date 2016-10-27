/*
 * motate_chip_pin_functions.h - SAMS70 specific version
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
#ifdef PIOA
//    _MAKE_MOTATE_PWM_PIN('A',  0, Motate::PWMTimer<0>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A',  0, _MOTATE_TEMP_TC(/*0*/0,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);

//    _MAKE_MOTATE_PWM_PIN('A',  1, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  1, _MOTATE_TEMP_TC(/*0*/0,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN('A',  2, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A',  5, Motate::PWMTimer<8+3>,      /*Peripheral:*/ A,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A',  7, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A',  8, Motate::PWMTimer<8+3>,      /*Peripheral:*/ A,  /*Inverted:*/ true);

//    _MAKE_MOTATE_PWM_PIN('A', 11, Motate::PWMTimer<0>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 11, Motate::PWMTimer<8+0>,      /*Peripheral:*/ C,  /*Inverted:*/ false);

//    _MAKE_MOTATE_PWM_PIN('A', 12, Motate::PWMTimer<1>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 12, Motate::PWMTimer<8+0>,      /*Peripheral:*/ C,  /*Inverted:*/ true);

    _MAKE_MOTATE_PWM_PIN('A', 13, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
//    _MAKE_MOTATE_PWM_PIN('A', 13, Motate::PWMTimer<8+1>,      /*Peripheral:*/ C,  /*Inverted:*/ false);

//    _MAKE_MOTATE_PWM_PIN('A', 14, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 14, Motate::PWMTimer<8+1>,      /*Peripheral:*/ C,  /*Inverted:*/ true);

//    _MAKE_MOTATE_PWM_PIN('A', 15, Motate::PWMTimer<3>,        /*Peripheral:*/ C,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 15, _MOTATE_TEMP_TC(/*0*/1,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);

//    _MAKE_MOTATE_PWM_PIN('A', 16, Motate::PWMTimer<2>,        /*Peripheral:*/ C,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 16, _MOTATE_TEMP_TC(/*0*/1,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);

    _MAKE_MOTATE_PWM_PIN('A', 17, Motate::PWMTimer<3>,        /*Peripheral:*/ C,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 19, Motate::PWMTimer<0>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 20, Motate::PWMTimer<1>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    // _MAKE_MOTATE_PWM_PIN('A', 23, Motate::PWMTimer<0>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 23, Motate::PWMTimer<8+2>,      /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 24, Motate::PWMTimer<1>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 25, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('A', 26, _MOTATE_TEMP_TC(/*0*/2,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 27, _MOTATE_TEMP_TC(/*0*/2,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('A', 30, Motate::PWMTimer<2>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
#endif // PIOA
#ifdef PIOB
    _MAKE_MOTATE_PWM_PIN('B',  0, Motate::PWMTimer<0>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('B',  1, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
//    _MAKE_MOTATE_PWM_PIN('B',  4, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ true); // used by SWD
//    _MAKE_MOTATE_PWM_PIN('B',  5, Motate::PWMTimer<0>,        /*Peripheral:*/ B,  /*Inverted:*/ false); // used by SWD
//    _MAKE_MOTATE_PWM_PIN('B', 12, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ false); // used by SWD
    _MAKE_MOTATE_PWM_PIN('B', 13, Motate::PWMTimer<2>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
#endif // PIOB
#ifdef PIOC
    _MAKE_MOTATE_PWM_PIN('C',  0, Motate::PWMTimer<0>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  1, Motate::PWMTimer<1>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  2, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  3, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
#ifdef TC2
    _MAKE_MOTATE_PWM_PIN('C',  5, _MOTATE_TEMP_TC(/*2*/6,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  6, _MOTATE_TEMP_TC(/*2*/6,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  8, _MOTATE_TEMP_TC(/*2*/7,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C',  9, _MOTATE_TEMP_TC(/*2*/7,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 11, _MOTATE_TEMP_TC(/*2*/8,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 12, _MOTATE_TEMP_TC(/*2*/8,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
#endif // TC2
    _MAKE_MOTATE_PWM_PIN('C', 13, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('C', 15, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 18, Motate::PWMTimer<1>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 19, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('C', 20, Motate::PWMTimer<2>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 21, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('C', 22, Motate::PWMTimer<3>,        /*Peripheral:*/ B,  /*Inverted:*/ false);
#ifdef TC1
    _MAKE_MOTATE_PWM_PIN('C', 23, _MOTATE_TEMP_TC(/*1*/3,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 24, _MOTATE_TEMP_TC(/*1*/3,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 26, _MOTATE_TEMP_TC(/*1*/4,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 27, _MOTATE_TEMP_TC(/*1*/4,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 29, _MOTATE_TEMP_TC(/*1*/5,0),  /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('C', 30, _MOTATE_TEMP_TC(/*1*/5,1),  /*Peripheral:*/ B,  /*Inverted:*/ false);
#endif // TC1
#endif // PIOC
#ifdef PIOD
//    _MAKE_MOTATE_PWM_PIN('D',  0, Motate::PWMTimer<8+0>,      /*Peripheral:*/ B,  /*Inverted:*/ false); // USB_VBUS
    _MAKE_MOTATE_PWM_PIN('D',  1, Motate::PWMTimer<8+0>,      /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D',  2, Motate::PWMTimer<8+1>,      /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D',  3, Motate::PWMTimer<8+1>,      /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D',  4, Motate::PWMTimer<8+2>,      /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D',  5, Motate::PWMTimer<8+2>,      /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D',  6, Motate::PWMTimer<8+3>,      /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D',  7, Motate::PWMTimer<8+3>,      /*Peripheral:*/ B,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D', 10, Motate::PWMTimer<  0>,      /*Peripheral:*/ B,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D', 11, Motate::PWMTimer<  0>,      /*Peripheral:*/ B,  /*Inverted:*/ true);

    _MAKE_MOTATE_PWM_PIN('D', 20, Motate::PWMTimer<0>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D', 21, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D', 22, Motate::PWMTimer<2>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D', 23, Motate::PWMTimer<3>,        /*Peripheral:*/ A,  /*Inverted:*/ true);
    _MAKE_MOTATE_PWM_PIN('D', 24, Motate::PWMTimer<0>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D', 25, Motate::PWMTimer<1>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D', 26, Motate::PWMTimer<2>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
    _MAKE_MOTATE_PWM_PIN('D', 27, Motate::PWMTimer<3>,        /*Peripheral:*/ A,  /*Inverted:*/ false);
#endif // PIOD

#undef _MOTATE_TEMP_TC//(t,c)

    //    // SPI Pin assignements
    _MAKE_MOTATE_SPI_CS_PIN('A', 31, /* SPINum:*/ 0, /* Peripheral:*/ A, /* CS Index:*/ 1);
#ifdef PIOB
    _MAKE_MOTATE_SPI_CS_PIN('B',  2, /* SPINum:*/ 0, /* Peripheral:*/ D, /* CS Index:*/ 0);
#endif // PIOB
#ifdef PIOC
    _MAKE_MOTATE_SPI_CS_PIN('C', 25, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 0);
    _MAKE_MOTATE_SPI_CS_PIN('C', 28, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 1);
    _MAKE_MOTATE_SPI_CS_PIN('C', 29, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 2);
    _MAKE_MOTATE_SPI_CS_PIN('C', 30, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 3);
#endif // PIOC
#ifdef PIOD
    _MAKE_MOTATE_SPI_CS_PIN('D',  0, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 1);
    _MAKE_MOTATE_SPI_CS_PIN('D',  1, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 2);
    _MAKE_MOTATE_SPI_CS_PIN('D',  2, /* SPINum:*/ 1, /* Peripheral:*/ C, /* CS Index:*/ 3);
    _MAKE_MOTATE_SPI_CS_PIN('D', 12, /* SPINum:*/ 0, /* Peripheral:*/ C, /* CS Index:*/ 2);
    _MAKE_MOTATE_SPI_CS_PIN('D', 25, /* SPINum:*/ 0, /* Peripheral:*/ B, /* CS Index:*/ 1);
    _MAKE_MOTATE_SPI_CS_PIN('D', 27, /* SPINum:*/ 0, /* Peripheral:*/ B, /* CS Index:*/ 3);
#endif // PIOD

#ifdef PIOC
    _MAKE_MOTATE_SPI_MISO_PIN('C', 26, /* SPINum:*/ 1, /* Peripheral */ B);
    _MAKE_MOTATE_SPI_MOSI_PIN('C', 27, /* SPINum:*/ 1, /* Peripheral */ B);
    _MAKE_MOTATE_SPI_SCK_PIN ('C', 24, /* SPINum:*/ 1, /* Peripheral */ B);
#endif // PIOC

#ifdef PIOD
    _MAKE_MOTATE_SPI_MISO_PIN('D', 20, /* SPINum:*/ 0, /* Peripheral */ B);
    _MAKE_MOTATE_SPI_MOSI_PIN('D', 21, /* SPINum:*/ 0, /* Peripheral */ B);
    _MAKE_MOTATE_SPI_SCK_PIN ('D', 22, /* SPINum:*/ 0, /* Peripheral */ B);
#endif // PIOD

    //
    // UART Pin Assignments
    // NOTE: UARTs 4,5,6 and 7 (here) are UART0 thru UART4 hardware
    //       UARTs 0,1 and 2 are USART0 and up (note the S in USART!)
    _MAKE_MOTATE_UART_RX_PIN( 'A',  9, /* UART number: */ 4+0, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'A', 10, /* UART number: */ 4+0, /* Peripheral */ A);

    _MAKE_MOTATE_UART_RX_PIN( 'A',  5, /* UART number: */ 4+1, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN( 'A',  4, /* UART number: */ 4+1, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN( 'A',  6, /* UART number: */ 4+1, /* Peripheral */ C);
#ifdef PIOD
    // _MAKE_MOTATE_UART_TX_PIN( 'D', 26, /* UART number: */ 4+1, /* Peripheral */ D); // disabled to allow 4+2 a TX pin

    _MAKE_MOTATE_UART_RX_PIN( 'D', 25, /* UART number: */ 4+2, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN( 'D', 26, /* UART number: */ 4+2, /* Peripheral */ C);

    _MAKE_MOTATE_UART_RX_PIN( 'D', 28, /* UART number: */ 4+3, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'D', 30, /* UART number: */ 4+3, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN( 'D', 31, /* UART number: */ 4+3, /* Peripheral */ B);

    _MAKE_MOTATE_UART_RX_PIN( 'D', 18, /* UART number: */ 4+4, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN( 'D',  3, /* UART number: */ 4+4, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN( 'D', 19, /* UART number: */ 4+4, /* Peripheral */ C);
#endif // PIOD


#ifdef PIOB
    _MAKE_MOTATE_UART_RX_PIN ( 'B',   0, /* UART number: */ 0, /* Peripheral */ C);
    _MAKE_MOTATE_UART_TX_PIN ( 'B',   1, /* UART number: */ 0, /* Peripheral */ C);
    _MAKE_MOTATE_UART_RTS_PIN( 'B',   3, /* UART number: */ 0, /* Peripheral */ C);
    _MAKE_MOTATE_UART_CTS_PIN( 'B',   2, /* UART number: */ 0, /* Peripheral */ C);

    _MAKE_MOTATE_UART_RX_PIN ( 'A',  21, /* UART number: */ 1, /* Peripheral */ A);
    _MAKE_MOTATE_UART_TX_PIN ( 'B',   4, /* UART number: */ 1, /* Peripheral */ D);
    _MAKE_MOTATE_UART_RTS_PIN( 'A',  24, /* UART number: */ 1, /* Peripheral */ A);
    _MAKE_MOTATE_UART_CTS_PIN( 'A',  25, /* UART number: */ 1, /* Peripheral */ A);
#endif // PIOB

#ifdef PIOD
    _MAKE_MOTATE_UART_RX_PIN ( 'D',  15, /* UART number: */ 2, /* Peripheral */ B);
    _MAKE_MOTATE_UART_TX_PIN ( 'D',  16, /* UART number: */ 2, /* Peripheral */ B);
    _MAKE_MOTATE_UART_RTS_PIN( 'D',  18, /* UART number: */ 2, /* Peripheral */ B);
    _MAKE_MOTATE_UART_CTS_PIN( 'D',  19, /* UART number: */ 2, /* Peripheral */ B);
#endif // PIOD

    // ADC Pin assignments CURRRENTLY WRONG
    //    _MAKE_MOTATE_ADC_PIN('A',  2, /* ADC number:*/  0);
    //    _MAKE_MOTATE_ADC_PIN('A',  3, /* ADC number:*/  1);
    //    _MAKE_MOTATE_ADC_PIN('A',  4, /* ADC number:*/  2);
    //    _MAKE_MOTATE_ADC_PIN('A',  6, /* ADC number:*/  3);
    //    _MAKE_MOTATE_ADC_PIN('A', 16, /* ADC number:*/  7);
    //    _MAKE_MOTATE_ADC_PIN('A', 22, /* ADC number:*/  4);
    //    _MAKE_MOTATE_ADC_PIN('A', 23, /* ADC number:*/  5);
    //    _MAKE_MOTATE_ADC_PIN('A', 24, /* ADC number:*/  6);
    //    _MAKE_MOTATE_ADC_PIN('B', 12, /* ADC number:*/  8);
    //    _MAKE_MOTATE_ADC_PIN('B', 13, /* ADC number:*/  9);
    //    _MAKE_MOTATE_ADC_PIN('B', 17, /* ADC number:*/ 10);
    //    _MAKE_MOTATE_ADC_PIN('B', 18, /* ADC number:*/ 11);
    //    _MAKE_MOTATE_ADC_PIN('B', 19, /* ADC number:*/ 12);
    //    _MAKE_MOTATE_ADC_PIN('B', 20, /* ADC number:*/ 13);
    //    _MAKE_MOTATE_ADC_PIN('B', 21, /* ADC number:*/ 14);

// Clock outputs (LIKELY WRONG)
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('A',  6, /*clockNumber*/0, /*peripheralAorB*/B);
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('A', 17, /*clockNumber*/1, /*peripheralAorB*/B);
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('A', 18, /*clockNumber*/2, /*peripheralAorB*/B);
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('A', 21, /*clockNumber*/1, /*peripheralAorB*/B);
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('A', 31, /*clockNumber*/2, /*peripheralAorB*/B);
// #ifdef PIOB
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('B',  3, /*clockNumber*/2, /*peripheralAorB*/B);
//     _MAKE_MOTATE_CLOCK_OUTPUT_PIN('B', 13, /*clockNumber*/0, /*peripheralAorB*/B);
// #endif
}

#endif
