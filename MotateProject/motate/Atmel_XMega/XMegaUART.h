/*
 XMegaUART.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2015 - 2016 Robert Giseburt
 Copyright (c) 2015 - 2016 Alden S. Hart Jr.

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

#ifndef XMEGAUART_H_ONCE
#define XMEGAUART_H_ONCE

#include "MotatePins.h"
#include "MotateBuffer.h"
#include "stdlib.h"
#include "math.h"
#include "xmega.h"

//#pragma GCC optimize (0)

namespace Motate {

    struct UARTMode {

        static constexpr uint16_t NoParity           =      0; // Default
        static constexpr uint16_t EvenParity         = 1 << 0;
        static constexpr uint16_t OddParity          = 1 << 1;

        static constexpr uint16_t OneStopBit         =      0; // Default
        static constexpr uint16_t TwoStopBits        = 1 << 2;

        static constexpr uint16_t As8Bit             =      0; // Default
        static constexpr uint16_t As9Bit             = 1 << 3;
//        static constexpr uint16_t As10Bit            = 1 << 4;

        // Some careful hand math will show that 8N1 == 0
        static constexpr uint16_t As8N1              = As8Bit | NoParity | OneStopBit;

        static constexpr uint16_t RTSCTSFlowControl  = 1 << 5;
        static constexpr uint16_t XonXoffFlowControl = 1 << 6;

        // TODO: Add polarity inversion and bit reversal options
    };

    struct UARTInterrupt {
        static constexpr uint16_t Off               = 0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr uint16_t Unknown           = 0;

        static constexpr uint16_t OnTxReady         = 1<<1;
        static constexpr uint16_t OnTransmitReady   = 1<<1;
        static constexpr uint16_t OnTxDone          = 1<<2;
        static constexpr uint16_t OnTransmitDone    = 1<<2;

        static constexpr uint16_t OnRxReady         = 1<<3;
        static constexpr uint16_t OnReceiveReady    = 1<<3;
        static constexpr uint16_t OnRxDone          = 1<<3;
        static constexpr uint16_t OnReceiveDone     = 1<<3;

        static constexpr uint16_t OnIdle            = 0; // Unsupported

        /* Set priority levels here as well: */
        static constexpr uint16_t PriorityHighest   = 1<<5; // Alias for PriorityHigh
        static constexpr uint16_t PriorityHigh      = 1<<5;
        static constexpr uint16_t PriorityMedium    = 1<<6;
        static constexpr uint16_t PriorityLow       = 1<<7;
        static constexpr uint16_t PriorityLowest    = 1<<7; // Alias for PriorityLow

        static constexpr uint16_t PriorityMask    = ((1<<8) - (1<<5));
    };

    // Convenience template classes for specialization:

    template<pin_number rxPinNumber, pin_number txPinNumber>
    using IsValidUART = typename std::enable_if<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum
	>::type;

    static const char kUARTXOn  = 0x11;
    static const char kUARTXOff = 0x13;


    struct _UARTHardwareProxy {
        virtual void uartInterruptHandler(const uint16_t interruptCause) = 0;
    };

    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.

    template<uint8_t uartPeripheralNumber>
    struct _UARTHardware {
        static USART_t& uart_proxy;
        static const uint8_t uartPeripheralNum = uartPeripheralNumber;

        static uint16_t _txReadyPriority;

        static _UARTHardwareProxy *proxy;

        void init() {
//            dx->usart = (USART_t *)pgm_read_word(&cfgUsart[idx].usart);
//            dx->port = (PORT_t *)pgm_read_word(&cfgUsart[idx].port);
//            uint8_t baud = (uint8_t)pgm_read_byte(&cfgUsart[idx].baud);
//            if (baud == XIO_BAUD_UNSPECIFIED) { baud = XIO_BAUD_DEFAULT; }
//            xio_set_baud_usart(dx, baud);						// usart must be bound first
//            dx->port->DIRCLR = (uint8_t)pgm_read_byte(&cfgUsart[idx].inbits);
//            dx->port->DIRSET = (uint8_t)pgm_read_byte(&cfgUsart[idx].outbits);
//            dx->port->OUTCLR = (uint8_t)pgm_read_byte(&cfgUsart[idx].outclr);
//            dx->port->OUTSET = (uint8_t)pgm_read_byte(&cfgUsart[idx].outset);
//            dx->usart->CTRLB = (USART_TXEN_bm | USART_RXEN_bm);	// enable tx and rx
//            dx->usart->CTRLA = CTRLA_RXON_TXON;					// enable tx and rx IRQs
//
//            dx->port->USB_CTS_PINCTRL = PORT_OPC_TOTEM_gc | PORT_ISC_BOTHEDGES_gc;
//            dx->port->INTCTRL = USB_CTS_INTLVL;		// see xio_usart.h for setting
//            dx->port->USB_CTS_INTMSK = USB_CTS_bm;


            disable();
        };

        _UARTHardware() {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };

        void enable() {
            uart_proxy.CTRLB |= USART_RXEN_bm | USART_TXEN_bm;
        };

        void disable () {
            uart_proxy.CTRLB &= ~(USART_RXEN_bm | USART_TXEN_bm);
        };

#ifdef ALLOW_ODD_BAUDS
        inline int32_t _calcBSEL(const int32_t baud, const int32_t tmpBSCALE, const int32_t tmpCLK2X)  {
            return (int32_t)
            ((tmpCLK2X == 0) ?
            (
             (tmpBSCALE < 0) ?
             (
              ((((float)F_CPU / (16.0 * (float)baud)) - 1.0)/pow(2.0,tmpBSCALE))
              ) :
             (
              (((float)F_CPU / (pow(2.0,tmpBSCALE) * 16.0 * baud)) - 1.0)
              )
             ) :
            (
             (tmpBSCALE < 0) ?
             (
              ((((float)F_CPU / (8.0 * (float)baud)) - 1.0)/pow(2.0,tmpBSCALE))
              ) :
             (
              (((float)F_CPU / (pow(2.0,tmpBSCALE) * 8.0 * baud)) - 1.0)
              )
             ) + 0.5);
        };

        inline int32_t _calcBaud(const int32_t tmpBSEL, const int32_t tmpBSCALE, const int32_t tmpCLK2X)  {
            return (int32_t)
            ((tmpCLK2X == 0) ?
             (
              (tmpBSCALE < 0) ?
              (
               ((float)F_CPU / (16.0 * ((pow(2.0,tmpBSCALE) * (float)tmpBSEL) + 1.0)))
               ) :
              (
               ((float)F_CPU / (pow(2.0,tmpBSCALE) * 16.0 * ((float)tmpBSEL + 1.0)))
               )
              ) :
             (
              (tmpBSCALE < 0) ?
              (
               ((float)F_CPU / (8.0 * ((pow(2.0,tmpBSCALE) * (float)tmpBSEL) + 1.0)))
               ) :
              (
               ((float)F_CPU / (pow(2.0,tmpBSCALE) * 8.0 * ((float)tmpBSEL + 1.0)))
               )
              ) + 0.5);
        };

        inline float _calcError(const int32_t baud, const int32_t tmpBaud)  {
            return (float)(tmpBaud - baud)/((float)baud);
        };
#endif

        void setOptions(const int32_t baud, const uint16_t options, const bool fromConstructor=false)  {
            disable();

            // Using int16_t for most of these so as to avoid conversions.
            // BSEL is valid from 0 to 4096
            // BSCALE is valid from -7 to 7
            // CLK2X is 0 or 1
            int16_t bestBSEL = 0;
            int16_t bestBSCALE = 0;
            int16_t bestCLK2X = 0;

            if (baud == 1200) {
                bestBSEL   = 3331;
                bestBSCALE = -1;
                bestCLK2X  = 0;
            }
            else if (baud == 2400) {
                bestBSEL   = 3329;
                bestBSCALE = -2;
                bestCLK2X  = 0;
            }
            else if (baud == 4800) {
                bestBSEL   = 3325;
                bestBSCALE = -3;
                bestCLK2X  = 0;
            }
            else if (baud == 9600) {
                bestBSEL   = 3317;
                bestBSCALE = -4;
                bestCLK2X  = 0;
            }
            else if (baud == 14400) {
                bestBSEL   = 2206;
                bestBSCALE = -4;
                bestCLK2X  = 0;
            }
            else if (baud == 19200) {
                bestBSEL   = 3301;
                bestBSCALE = -5;
                bestCLK2X  = 0;
            }
            else if (baud == 28800) {
                bestBSEL   = 1095;
                bestBSCALE = -4;
                bestCLK2X  = 0;
            }
            else if (baud == 38400) {
                bestBSEL   = 3269;
                bestBSCALE = -6;
                bestCLK2X  = 0;
            }
            else if (baud == 57600) {
                bestBSEL   = 2158;
                bestBSCALE = -6;
                bestCLK2X  = 0;
            }
            else if (baud == 115200) {
                bestBSEL   = 1047;
                bestBSCALE = -6;
                bestCLK2X  = 0;
            }
            else if (baud == 230400) {
                bestBSEL   = 983;
                bestBSCALE = -7;
                bestCLK2X  = 0;
            }
#ifdef ALLOW_ODD_BAUDS
            else {
                int32_t tmpBaud = 0;
                int16_t tmpBSEL = 0;
                int16_t tmpBSCALE = 7;
                int16_t tmpCLK2X = 0;
                float tmpError = 0;

                float bestError = 100000;

                do {
                    do {
                        tmpBSEL = _calcBSEL(baud, tmpBSCALE, tmpCLK2X);

                        if (tmpBSEL == -1) {
                            tmpBSCALE--;
                            continue;
                        }

                        tmpBaud = _calcBaud(tmpBSEL, tmpBSCALE, tmpCLK2X);

                        // We want 4 digits of precision, and we don't want to have to use float
                        // (and potentially bloat the code if it's not used in the project proper)
                        // so we'll bitshift by 10 bits to get a multiply of 1024.8
                        tmpError = _calcError(baud, tmpBaud);

                        if (fabs(bestError)>fabs(tmpError)) {
                            bestBSEL = tmpBSEL;
                            bestBSCALE = tmpBSCALE;
                            bestCLK2X = tmpCLK2X;
                            bestError = tmpError;
                        }
                        tmpBSCALE--;
                    } while (tmpBSCALE > -8);

                    tmpBSCALE=7;
                    tmpCLK2X++;
                } while (tmpCLK2X < 2);
            }
#endif
            /*
             bestBSEL   = 33;
             bestBSCALE = -1;
             bestCLK2X = 0;
             */

            uart_proxy.BAUDCTRLA = (uint8_t)bestBSEL;
            uart_proxy.BAUDCTRLB = (bestBSCALE << USART_BSCALE0_bp)|(bestBSEL >> 8);
            uart_proxy.CTRLB = (uart_proxy.CTRLB & ~USART_CLK2X_bm) | (bestCLK2X?USART_CLK2X_bm:0);


            /**********
             * From the excel spreadsheet included with AVR1307:
                bestBSEL = 0
                bestBSCALE = 0
                bestCLK2X = 0
                bestError = 1
                
                tmpBaud = 0
                tmpBSEL = 0
                tmpBscale = 7
                tmpCLK2X = 0
                tmpError = 0
                Do
                    Do
                        tmpBSEL = BSELCalc(CPUclk, tmpBscale, Baud, tmpCLK2X)
                        If (IsError(tmpBSEL) = False) Then
                            tmpBaud = BaudRateCalc(CPUclk, tmpBscale, tmpBSEL, tmpCLK2X)
                            If (IsError(tmpBaud) = False) Then
                                tmpError = Round(((tmpBaud - Baud) / Baud), 4)
                            Else
                                tmpError = CVErr(xlErrNA)
                            End If
                        Else
                            tmpError = CVErr(xlErrNA)
                        End If
                        
                        If (IsError(tmpError) = False) Then
                            If (Abs(bestError) > Abs(tmpError)) Then
                                bestBSEL = tmpBSEL
                                bestBSCALE = tmpBscale
                                bestCLK2X = tmpCLK2X
                                bestError = tmpError
                            End If
                        End If
                        tmpBscale = tmpBscale - 1
                    Loop While (tmpBscale > -8)
                    tmpBscale = 7
                    tmpCLK2X = tmpCLK2X + 1
                Loop While (tmpCLK2X < 2)
             
             ---
             
                Function BSELCalc(Fper, Bscale, FBaud, Clk2x)
                    If (Clk2x = 0) Then
                        If (Bscale < 0) Then
                            bselval = (1 / (2 ^ Bscale)) * ((Fper / (16 * FBaud)) - 1)
                        Else
                            bselval = ((Fper / ((2 ^ Bscale) * 16 * FBaud)) - 1)
                        End If
                    ElseIf (Clk2x = 1) Then
                        If (Bscale < 0) Then
                            bselval = (1 / (2 ^ Bscale)) * ((Fper / (8 * FBaud)) - 1)
                        Else
                            bselval = ((Fper / ((2 ^ Bscale) * 8 * FBaud)) - 1)
                        End If
                    Else
                        bselval = CVErr(xlErrNA)
                    End If
                    
                    If (bselval >= 0) And (bselval <= 4095) Then
                        BSELCalc = Round(bselval, 0)
                    Else
                        BSELCalc = CVErr(xlErrNA)
                    End If
                End Function

             ---
             
                Function BaudRateCalc(Fper, Bscale, Bsel, Clk2x)
                    If (Clk2x = 0) Then
                        If (Bscale < 0) Then
                            Baud = (Fper / (16 * (((2 ^ Bscale) * Bsel) + 1)))
                        Else
                            Baud = (Fper / ((2 ^ Bscale) * 16 * (Bsel + 1)))
                        End If
                    ElseIf (Clk2x = 1) Then
                        If (Bscale < 0) Then
                            Baud = (Fper / (8 * (((2 ^ Bscale) * Bsel) + 1)))
                        Else
                            Baud = (Fper / ((2 ^ Bscale) * 8 * (Bsel + 1)))
                        End If
                    Else
                       Baud = CVErr(xlErrNA)
                    End If
                    BaudRateCalc = Baud
                End Function

             **********/

            uint8_t tmpCTRLC = uart_proxy.CTRLC;

            if (options & UARTMode::As9Bit) {
                tmpCTRLC = (tmpCTRLC & ~(USART_CHSIZE_gm)) | USART_CHSIZE_9BIT_gc;
            } else {
                tmpCTRLC = (tmpCTRLC & ~(USART_CHSIZE_gm)) | USART_CHSIZE_8BIT_gc;
            }

            if (options & (UARTMode::EvenParity|UARTMode::EvenParity)) { // USART_PMODE_t
                tmpCTRLC = (tmpCTRLC & ~(USART_PMODE_gm)) | USART_PMODE_EVEN_gc;
            } else if (options & (UARTMode::EvenParity|UARTMode::OddParity)) { // USART_PMODE_t
                tmpCTRLC = (tmpCTRLC & ~(USART_PMODE_gm)) | USART_PMODE_ODD_gc;
            } else {
                tmpCTRLC = (tmpCTRLC & ~(USART_PMODE_gm)) | USART_PMODE_DISABLED_gc;
            }

            if (options & (UARTMode::EvenParity|UARTMode::TwoStopBits)) { // bool
                tmpCTRLC |= USART_SBMODE_bm;
            } else {
                tmpCTRLC &= ~USART_SBMODE_bm;
            }

            uart_proxy.CTRLC = tmpCTRLC;

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & (UARTInterrupt::OnRxDone | UARTInterrupt::OnTxDone | UARTInterrupt::OnTxReady | UARTInterrupt::OnIdle)) {
                    // TODO
                }

                if (interrupts & UARTInterrupt::OnRxReady) {
                    if (interrupts & UARTInterrupt::PriorityHigh) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_HI_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityMedium) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_MED_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityLow) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_LO_gc;
                    }
                    else {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_OFF_gc;
                    }
                }

                if (interrupts & UARTInterrupt::OnTxDone) {
                    if (interrupts & UARTInterrupt::PriorityHigh) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_TXCINTLVL_gm) | USART_TXCINTLVL_HI_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityMedium) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_TXCINTLVL_gm) | USART_TXCINTLVL_MED_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityLow) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_TXCINTLVL_gm) | USART_TXCINTLVL_LO_gc;
                    }
                    else {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_TXCINTLVL_gm) | USART_TXCINTLVL_OFF_gc;
                    }
                }

                if (interrupts & UARTInterrupt::OnTxReady) {
                    // Store the last priority set to restore it later...
                    if (interrupts & UARTInterrupt::PriorityMask) {
                        _txReadyPriority = interrupts & UARTInterrupt::PriorityMask;
                    }


                    if (interrupts & UARTInterrupt::PriorityHigh) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_HI_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityMedium) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_MED_gc;
                    }
                    else if (interrupts & UARTInterrupt::PriorityLow) {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_LO_gc;
                    }
                    else {
                        uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
                    }
                }

                // TODO -- enable interrupts
            } else {
                uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_RXCINTLVL_gm) | USART_RXCINTLVL_OFF_gc;
                uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_TXCINTLVL_gm) | USART_TXCINTLVL_OFF_gc;
                uart_proxy.CTRLA = (uart_proxy.CTRLA & ~USART_DREINTLVL_gm) | USART_DREINTLVL_OFF_gc;
            }
        };

        void setInterruptTxReady(bool value) {
            if (value) {
                this->setInterrupts(UARTInterrupt::OnTxReady | _txReadyPriority);
            } else {
                this->setInterrupts(UARTInterrupt::OnTxReady | UARTInterrupt::Off);
            }
        };


        uint16_t getInterruptCause() __attribute__ (( noinline )) {
            // TODO
            return 0;
        }

        // TODO: support 9-bit reads
        int16_t readByte() {
            if (uart_proxy.STATUS & USART_RXCIF_bm) {
                return uart_proxy.DATA;
            }
            return -1;
        };

        // TODO: support 9-bit writes
        int16_t writeByte(const char value) {
            if (uart_proxy.STATUS & USART_DREIF_bm) {
                uart_proxy.DATA = value;
                return 1;
            }
            return -1;
        };

        void flush() {
            // Wait for the buffer to be empty
            do {} while ((uart_proxy.STATUS & USART_TXCIF_bm) == 0);
        };
    };


    template<>
    struct _UARTHardware<-1> {}; // trap specialization


    template<pin_number rxPinNumber, pin_number txPinNumber>
    using UARTGetPeripheralNum = typename std::conditional<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum,
        /* True:  */ _UARTHardware<UARTTxPin<txPinNumber>::uartNum>,
        /* False: */ _UARTHardware<-1>
    >::type;





    template<pin_number rxPinNumber, pin_number txPinNumber>
    struct UART {
        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;

        _UARTHardware< UARTGetPeripheralNum<rxPinNumber, txPinNumber>::uartPeripheralNum > hardware;
        const inline uint8_t uartPeripheralNum() { return hardware.uartNum; };

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
            hardware.init();
            init(baud, options, /*fromConstructor =*/ true);
        };

        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            setOptions(baud, options, fromConstructor);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false)  {
            hardware.setOptions(baud, options, fromConstructor);
        };

        int16_t readByte() {
            return hardware.readByte();
        };

        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
        int16_t read(const uint8_t *buffer, const uint16_t length) {
            int16_t total_read = 0;
            int16_t to_read = length;
            const uint8_t *read_ptr = buffer;

            // BLOCKING!!
            while (to_read > 0) {
                int16_t ret = readByte();

                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
            };

            return total_read;
        };

        int16_t writeByte(uint8_t data) {
            //hardware.flush();
            return hardware.writeByte(data);
        };

        void flush() {
            // Wait for the buffer to be empty...
            hardware.flush();
        };

        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
        int16_t write(const char* data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            const char* out_ptr = data;
            int16_t to_write = length;

            if (length==0 && *out_ptr==0) {
                return 0;
            }

            if (length == 0) {
                to_write = 0x7FFF;
            }

            do {
                int16_t ret = hardware.writeByte(*out_ptr);

                if (ret > 0) {
                    out_ptr++;
                    total_written++;
                    to_write--;

                    if (length==0 && *out_ptr==0) {
                        break;
                    }
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write);

            if (autoFlush && total_written > 0)
                flush();

            return total_written;
        };

    	template<uint16_t _size>
        int16_t write(Motate::Buffer<_size> &data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            int16_t to_write = length;

            do {
                int16_t value = data.peek();
                if (value < 0) // no more data
                    break;

                int16_t ret = hardware.writeByte(value);
                if (ret > 0) {
                    data.pop();
                    to_write--;
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write != 0);

            if (autoFlush && total_written > 0)
                flush();

            return total_written;
        };
    };

    template<uint8_t uartPeripheralNumber, pin_number rtsPinNumber, pin_number ctsPinNumber, typename rxBufferClass, typename txBufferClass>
    struct _BufferedUARTHardware : _UARTHardware<uartPeripheralNumber>, _UARTHardwareProxy {
        rxBufferClass rxBuffer;
        txBufferClass txBuffer;

        OutputPin<rtsPinNumber> rtsPin;
        IRQPin<ctsPinNumber> ctsPin;

        uint32_t txDelayAfterResume = 3;
        uint32_t txDelayUntilTime   = 0;

        bool _rtsCtsFlowControl    = false;
        bool _xonXoffFlowControl   = false;
        volatile bool _xonXoffCanSend       = true;
        volatile char _xonXoffStartStop     = kUARTXOn;
        volatile bool _xonXoffStartStopSent = true;

        typedef _UARTHardware<uartPeripheralNumber> parent;

        _BufferedUARTHardware() {
            parent::proxy = this;
        };

        void init() {
            parent::init();
            parent::setInterrupts(UARTInterrupt::OnRxReady | UARTInterrupt::OnTxReady | UARTInterrupt::PriorityLowest);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            parent::setOptions(baud, options, fromConstructor);

            if (options & UARTMode::RTSCTSFlowControl && IsIRQPin<ctsPinNumber>() && !rtsPin.isNull()) {
                _rtsCtsFlowControl = true;
                parent::setInterruptTxReady(!canSend());
                ctsPin.setInterrupts(kPinInterruptOnChange|kPinInterruptPriorityLow);
            } else {
                _rtsCtsFlowControl = false;
            }
            if (options & UARTMode::XonXoffFlowControl) {
                _xonXoffFlowControl = true;
            } else {
                _xonXoffFlowControl = false;
                _xonXoffStartStopSent = true;
            }
        }

        void stopRx() {
            if (_rtsCtsFlowControl) {
                rtsPin = true;
            }
            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOff) {
                _xonXoffStartStop = kUARTXOff;
                _xonXoffStartStopSent = false;
                parent::setInterruptTxReady(true);
            }
        };

        void startRx() {
            if (_rtsCtsFlowControl) {
                rtsPin = false;
            }
            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOn) {
                _xonXoffStartStop = kUARTXOn;
                _xonXoffStartStopSent = false;
                parent::setInterruptTxReady(true);
            }
        };

        bool canSend() {
            if (_rtsCtsFlowControl) {
                return !((bool)ctsPin);
            }
            if (_xonXoffFlowControl) {
                return _xonXoffCanSend;
            }
            return true;
        };

        void setTxDelayAfterResume(uint32_t newDelay) { txDelayAfterResume = newDelay; };
        bool isConnected() {
            if (!IsIRQPin<ctsPinNumber>())
                return true; // There's no way to tell!

            // If we have a cts pin, then will will use it to know if we're allowed to send,
            // which gives us a reasonable guess, at least.
            return !ctsPin;
        };

        int16_t readByte() {
            if (rxBuffer.available() > (rxBuffer.size() - 4)) {
                startRx();
            }
            return rxBuffer.read();
        };

        int16_t writeByte(const uint8_t data) {
            int16_t ret = txBuffer.write(data);
            parent::setInterruptTxReady(canSend());
            return ret;
        };

        virtual void uartInterruptHandler(const uint16_t interruptCause) { // should be final, but Xcode barfs on the formatting
            if ((interruptCause & (UARTInterrupt::OnTxReady /*| UARTInterrupt::OnTxDone*/))) {
                if (txDelayUntilTime && SysTickTimer.getValue() < txDelayUntilTime)
                    return;

                txDelayUntilTime = 0;
                if (_xonXoffFlowControl) {
                    if (_xonXoffStartStopSent == false) {
                        parent::writeByte(_xonXoffStartStop);
                        _xonXoffStartStopSent = true;
                        return;
                    }
                }
                int16_t value = txBuffer.read();
                if (value >= 0) {
                    parent::writeByte(value);
                }
            }
            if (txBuffer.isEmpty() || txBuffer.isLocked()) {
                // This is tricky: If it's write locked, we have to bail, and SHUT OFF TxReady interrupts.
                // On the ARM, it won't return to the main code as long as there's a pending interrupt,
                // and the txReady interrupt will continue to fire, causing deadlock.
                parent::setInterruptTxReady(false);
            }

            if ((interruptCause & UARTInterrupt::OnRxReady) && !rxBuffer.isFull()) {
                int16_t value = parent::readByte();
                if (_xonXoffFlowControl) {
                    if (value == kUARTXOn) {
                        _xonXoffCanSend = true;
                        return;
                    } else if (value == kUARTXOff) {
                        _xonXoffCanSend = false;
                        return;
                    }
                }
                // We don't double check to ensure value is not -1 -- should we?
                rxBuffer.write(value);
                if (rxBuffer.available() < 4) {
                    stopRx();
                }
            }
        };

        void pinChangeInterrupt() {
            txDelayUntilTime = SysTickTimer.getValue() + txDelayAfterResume;
            parent::setInterruptTxReady(canSend());
        };

        void flush() {
            // Wait for the buffer to be empty...
            while(!txBuffer.isEmpty());
        };
    };


    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1, typename rxBufferClass = Buffer<128>, typename txBufferClass = rxBufferClass>
    struct BufferedUART {
        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;

        //        typedef _BufferedUARTHardware< UARTGetPeripheralNum<rxPinNumber, txPinNumber>::uartPeripheralNum, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass > hardware_t;
        //        hardware_t hardware;

        _BufferedUARTHardware< UARTGetPeripheralNum<rxPinNumber, txPinNumber>::uartPeripheralNum, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass > hardware;

        const uint8_t uartPeripheralNum() { return hardware.uartPeripheralNum; };

        BufferedUART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
            hardware.init();
            init(baud, options, /*fromConstructor =*/ true);
        };

        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            setOptions(baud, options, fromConstructor);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

        int16_t readByte() {
            return hardware.readByte();
        };

        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
        int16_t read(const uint8_t *buffer, const uint16_t length) {
            int16_t total_read = 0;
            int16_t to_read = length;
            const uint8_t *read_ptr = buffer;

            // BLOCKING!!
            while (to_read > 0) {
                int16_t ret = hardware.readByte();

                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
            };

            return total_read;
        };

        int16_t writeByte(uint8_t data) {
            return hardware.writeByte(data);
        };

        void flush() {
            hardware.flush();
        };

        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
        int16_t write(const char* data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            const char* out_ptr = data;
            int16_t to_write = length;

            if (length==0 && *out_ptr==0) {
                return 0;
            }

            do {
                int16_t ret = hardware.writeByte(*out_ptr);

                if (ret > 0) {
                    out_ptr++;
                    total_written++;
                    to_write--;

                    if (length==0 && *out_ptr==0) {
                        break;
                    }
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write);
            
            if (autoFlush && total_written > 0)
                flush();
            
            return total_written;
        };
        
        template<uint16_t _size>
        int16_t write(Motate::Buffer<_size> &data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            int16_t to_write = length;
            
            do {
                int16_t value = data.peek();
                if (value < 0) // no more data
                    break;
                
                int16_t ret = hardware.writeByte(value);
                if (ret > 0) {
                    data.pop();
                    to_write--;
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write != 0);
            
            if (autoFlush && total_written > 0)
                flush();
            
            return total_written;
        };
        
        void pinChangeInterrupt() {
            hardware.pinChangeInterrupt();
        };
        
        //	// Placeholder for user code.
        //	static void interrupt() __attribute__ ((weak));
    };
    
    //    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber, pin_number ctsPinNumber, typename rxBufferClass, typename txBufferClass>
    //        typename BufferedUART<rxPinNumber, txPinNumber, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass>::hardware_t BufferedUART<rxPinNumber, txPinNumber, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass>::hardware;
    
}

//#pragma GCC reset_options

#endif /* end of include guard: XMEGAUART_H_ONCE */