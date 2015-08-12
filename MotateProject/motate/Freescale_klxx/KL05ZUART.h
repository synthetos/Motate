/*
 KL05ZUART.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 Robert Giseburt

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

#ifndef KL05ZUART_H_ONCE
#define KL05ZUART_H_ONCE

#include "MotatePins.h"
#include "MotateBuffer.h"
#include <type_traits>
#include <algorithm> // for std::max, etc.

namespace Motate {

    struct UARTMode {

        static constexpr uint16_t NoParity           =      0; // Default
        static constexpr uint16_t EvenParity         = 1 << 0;
        static constexpr uint16_t OddParity          = 1 << 1;

        static constexpr uint16_t OneStopBit         =      0; // Default
        static constexpr uint16_t TwoStopBits        = 1 << 2;

        static constexpr uint16_t As8Bit             =      0; // Default
        static constexpr uint16_t As9Bit             = 1 << 3;
        static constexpr uint16_t As10Bit            = 1 << 4;

        // Some careful hand math will show that 8N1 == 0
        static constexpr uint16_t As8N1              = As8Bit | NoParity | OneStopBit;

        static constexpr uint16_t RTSCTSFlowControl  = 1 << 5;
        static constexpr uint16_t XonXoffFlowControl = 1 << 6;

        // TODO: Add polarity inversion and bit reversal options
    };

    struct UARTInterrupt {
        static constexpr uint16_t Off              = 0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr uint16_t Unknown           = 0;

        static constexpr uint16_t OnTxReady         = 1<<1;
        static constexpr uint16_t OnTransmitReady   = 1<<1;
        static constexpr uint16_t OnTxDone          = 1<<1;
        static constexpr uint16_t OnTransmitDone    = 1<<1;

        static constexpr uint16_t OnRxReady         = 1<<2;
        static constexpr uint16_t OnReceiveReady    = 1<<2;
        static constexpr uint16_t OnRxDone          = 1<<2;
        static constexpr uint16_t OnReceiveDone     = 1<<2;

        static constexpr uint16_t OnIdle            = 1<<3;

        /* Set priority levels here as well: */
        static constexpr uint16_t PriorityHighest   = 1<<5;
        static constexpr uint16_t PriorityHigh      = 1<<6;
        static constexpr uint16_t PriorityMedium    = 1<<7;
        static constexpr uint16_t PriorityLow       = 1<<8;
        static constexpr uint16_t PriorityLowest    = 1<<9;
    };

    // Convenience template classes for specialization:

    template<pin_number rxPinNumber, pin_number txPinNumber>
    using IsValidUART = typename std::enable_if<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::moduleId == UARTRxPin<rxPinNumber>::moduleId
	>::type;

    static const char kUARTXOn  = 0x11;
    static const char kUARTXOff = 0x13;

    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.

    template<uint8_t uartPeripheralNumber>
    struct _UARTHardware {
        static const uint8_t uartPeripheralNum=0xFF;

        // BITBANG HERE!
    };

    template<>
    struct _UARTHardware<0u> {
        // Template specialize this for KLxx with multiple UARTs
        struct uart_proxy_t {
            static volatile char &BDH() { return (char &)(UART0->BDH); };
            static volatile char &BDL() { return (char &)(UART0->BDL); };

            static volatile char &C1() { return (char &)(UART0->C1); };
            static volatile char &C2() { return (char &)(UART0->C2); };
            static volatile char &S1() { return (char &)(UART0->S1); };
            static volatile char &S2() { return (char &)(UART0->S2); };
            static volatile char &C3() { return (char &)(UART0->C3); };
            static volatile char &D () { return (char &)(UART0->D);  };
            static volatile char &C4() { return (char &)(UART0->C4); };
            static volatile char &C5() { return (char &)(UART0->C5); };
        };

        static uart_proxy_t uart_proxy;

        static constexpr const IRQn_Type uartIRQ() { return UART0_IRQn; };

        static constexpr const uint8_t uartPeripheralNum=0;

        //        typedef _UARTHardware<0u, rxPinNumber, txPinNumber> this_type_t;

        void init() {
            // Enable the UART0 Clock Gate
            SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;

            /* Select the clock source */
            /* 0b01 = MCGFLLCLK
             * 0b10 = OSCERCLK
             * 0b11 = MCGIRCLK
             */
            SIM->SOPT2 = (SIM->SOPT2 & ~SIM_SOPT2_UART0SRC_MASK) | SIM_SOPT2_UART0SRC(0b01);

            disable();
        };

        _UARTHardware() {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };

        void enable() {
            uart_proxy.C2() |= (uint8_t)(UART0_C2_TE_MASK | UART0_C2_RE_MASK);
        };

        void disable () {
            uart_proxy.C2() &= (uint8_t)~(UART0_C2_TE_MASK | UART0_C2_RE_MASK);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            // This all breaks down to the SBR ("Baud Rate Modulo Divisor" -- how does SBR stand for that?)
            // formula, which sets the baud rate. One key component is the OSR ("Over Sampling Ratio"), which
            // is a number between 3 and 31, where the actual rate of oversampling is OSR+1.
            //
            // Final formula:
            //   baud = clock / ((OSR+1) × SBR).
            // Higher OSR is better.
            // Manual also says that for 4 <= OSR+1 <= 7, set sampling to "both edges."

            uint32_t best_baud_diff = 0xFFFFFF; // Start it high, we'll change in in a second...
            uint32_t sbr_value = 0;

            uint32_t oversample_rate = 4;
            for (uint32_t test_oversample_rate = 4; test_oversample_rate <= 32; test_oversample_rate++) {
                uint32_t temp_sbr_value = SystemCoreClock / (baud * test_oversample_rate);
                uint32_t calculated_baud = SystemCoreClock / (temp_sbr_value * test_oversample_rate);

                uint32_t baud_diff = calculated_baud > baud ? calculated_baud - baud : baud - calculated_baud;

                if (baud_diff <= best_baud_diff) {
                    best_baud_diff = baud_diff;
                    oversample_rate = test_oversample_rate;
                    sbr_value = temp_sbr_value;
                }
            }

            // If the OSR is between 4x and 8x then both
            // edge sampling MUST be turned on.
            if (/*(oversample_rate > 3) && */(oversample_rate < 9))
                uart_proxy.C5() |= (uint8_t)UART0_C5_BOTHEDGE_MASK;

            // Setup OSR value
            uart_proxy.C4() = ((uint8_t)uart_proxy.C4() & ~UART0_C4_OSR_MASK ) | UART0_C4_OSR(oversample_rate-1);

            /* Save off the current value of the uartx_BDH except for the SBR field */
            uart_proxy.BDH() = ((uint8_t)uart_proxy.BDH() & ~(UART0_BDH_SBR_MASK | UART0_BDH_SBNS_MASK) )
            | UART0_BDH_SBR( sbr_value >> 8 )
            | (options & UARTMode::TwoStopBits ? UART0_BDH_SBNS_MASK : 0);
            uart_proxy.BDL() = (uint8_t)(sbr_value & UART0_BDL_SBR_MASK);

            if (options & UARTMode::As10Bit) {
                uart_proxy.C4() |= (uint8_t)UART0_C4_M10_MASK;
            } else if (options & UARTMode::As9Bit) {
                uart_proxy.C1() |= (uint8_t)UART0_C1_M_MASK;
                uart_proxy.C4() &= (uint8_t)~UART0_C4_M10_MASK;
            } else {
                uart_proxy.C1() &= (uint8_t)~UART0_C1_M_MASK;
                uart_proxy.C4() &= (uint8_t)~UART0_C4_M10_MASK;
            }

            if (options & (UARTMode::EvenParity|UARTMode::OddParity)) {
                // Do we need to enforce 9-bit mode here? The manual is ambiguous...
                uart_proxy.C1() = ((uint8_t)uart_proxy.C1() & ~UART0_C1_PT_MASK) | UART0_C1_PE_MASK | (options & UARTMode::EvenParity ? 0 : UART0_C1_PT_MASK);
            } else {
                uart_proxy.C1() &= (uint8_t)~UART0_C1_PE_MASK;
            }

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & (UARTInterrupt::OnRxDone | UARTInterrupt::OnTxDone | UARTInterrupt::OnTxReady | UARTInterrupt::OnIdle)) {
                    uart_proxy.C2() &= ~(UART0_C2_RIE_MASK | UART0_C2_TCIE_MASK | UART0_C2_TIE_MASK | UART0_C2_ILIE_MASK);
                }

                if (interrupts & UARTInterrupt::OnRxDone) {
                    uart_proxy.C2() |= UART0_C2_RIE_MASK;
                }
                if (interrupts & UARTInterrupt::OnTxDone) {
                    uart_proxy.C2() |= UART0_C2_TCIE_MASK;
                }

                if (interrupts & UARTInterrupt::OnTxReady) {
                    uart_proxy.C2() |= UART0_C2_TIE_MASK;
                }
                if (interrupts & UARTInterrupt::OnIdle) {
                    uart_proxy.C2() |= UART0_C2_ILIE_MASK;
                }


                /* Set interrupt priority */
                if (interrupts & UARTInterrupt::PriorityHighest) {
                    NVIC_SetPriority(uartIRQ(), 0);
                }
                else if (interrupts & UARTInterrupt::PriorityHigh) {
                    NVIC_SetPriority(uartIRQ(), 3);
                }
                else if (interrupts & UARTInterrupt::PriorityMedium) {
                    NVIC_SetPriority(uartIRQ(), 7);
                }
                else if (interrupts & UARTInterrupt::PriorityLow) {
                    NVIC_SetPriority(uartIRQ(), 11);
                }
                else if (interrupts & kInterruptPriorityLowest) {
                    NVIC_SetPriority(uartIRQ(), 15);
                }

                NVIC_EnableIRQ(uartIRQ());
            } else {

                NVIC_DisableIRQ(uartIRQ());
            }
        };

        void setInterruptTxReady(bool value) {
            if (value) {
                uart_proxy.C2() |= UART0_C2_TIE_MASK;
            } else {
                uart_proxy.C2() &= ~UART0_C2_TIE_MASK;
            }
        };


        static uint16_t getInterruptCause() __attribute__ (( noinline )) {
            uint16_t status = UARTInterrupt::Unknown;
            uint8_t s1 = uart_proxy.S1();
            if (s1 & UART0_S1_TDRE_MASK) {
                status |= UARTInterrupt::OnTxReady;
            }
            if (s1 & UART0_S1_TC_MASK) {
                status |= UARTInterrupt::OnTxDone;
            }
            if (s1 & UART0_S1_RDRF_MASK) {
                status |= UARTInterrupt::OnRxDone;
            }
            if (s1 & UART0_S1_IDLE_MASK) {
                status |= UARTInterrupt::OnIdle;
            }
            return status;
        }

        int16_t readByte() {
            // uart_proxy.S1 |= (uint8_t)UART0_S1_OR_MASK;
            if (uart_proxy.S1() & (uint8_t)UART0_S1_RDRF_MASK) {
                return uart_proxy.D();
            }
            return -1;
        };

        int16_t writeByte(const char value) {
            // uart_proxy.S1 |= (uint8_t)UART0_S1_OR_MASK;
            if (uart_proxy.S1() & (uint8_t)UART0_S1_TDRE_MASK) {
                uart_proxy.D() = value;
                return 1;
            }
            return -1;
        };

        void flush() {
            // Wait for the buffer to be empty
            while(!(uart_proxy.S1() & (uint8_t)UART0_S1_TDRE_MASK)) {;}
        };
    };


    template<pin_number rxPinNumber, pin_number txPinNumber>
    using UARTGetPeripheralNum = typename std::conditional<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum,
        /* True:  */ _UARTHardware<0>,
        /* False: */ _UARTHardware<0xff>
    >::type;





    template<pin_number rxPinNumber = ReversePinLookup<'B',  2>::number, pin_number txPinNumber = ReversePinLookup<'B',  1>::number>
    struct UART {
        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;

        static _UARTHardware< UARTGetPeripheralNum<rxPinNumber, txPinNumber>::uartPeripheralNum > hardware;
        const inline uint8_t uartPeripheralNum() { return hardware.moduleId; };

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
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
            hardware.flush();
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

    struct _UARTHardwareProxy {
        virtual void uartInterruptHandler(uint16_t interruptCause) = 0;
    };

    extern _UARTHardwareProxy *uart0HardwareProxy;

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
            uart0HardwareProxy = this;
        };

        void init() {
            parent::init();
            parent::setInterrupts(UARTInterrupt::OnRxReady | UARTInterrupt::PriorityLowest);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            parent::setOptions(baud, options, fromConstructor);

            if (options & UARTMode::RTSCTSFlowControl && IsIRQPin<ctsPinNumber>() && !rtsPin.isNull()) {
                _rtsCtsFlowControl = true;
                parent::setInterruptTxReady(!canSend());
                ctsPin.setInterrupts(kPinInterruptOnChange);
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
                return !ctsPin;
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

        virtual void uartInterruptHandler(uint16_t interruptCause) { // should be final, but Xcode barfs on the formatting
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


    template<pin_number rxPinNumber = ReversePinLookup<'B',  2>::number, pin_number txPinNumber = ReversePinLookup<'B',  1>::number, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1, typename rxBufferClass = Buffer<128>, typename txBufferClass = rxBufferClass>
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

#endif /* end of include guard: KL05ZUART_H_ONCE */