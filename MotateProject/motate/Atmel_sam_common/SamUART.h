/*
 SamUART.h - Library for the Motate system
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

#ifndef SAMUART_H_ONCE
#define SAMUART_H_ONCE

#include <MotateUART.h>
#include <MotateBuffer.h>
#include <type_traits>
#include <algorithm> // for std::max, etc.
#include <functional>

#include "SamCommon.h" // pull in defines and fix them
#include "SamDMA.h" // pull in defines and fix them

namespace Motate {
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

    enum class USART_MODE_t : uint32_t {
        USART_NORMAL    = 0x0 << US_MR_USART_MODE_Pos,
        RS485           = 0x1 << US_MR_USART_MODE_Pos,
        HW_HANDSHAKING  = 0x2 << US_MR_USART_MODE_Pos,
        IS07816_T_0     = 0x4 << US_MR_USART_MODE_Pos,
        IS07816_T_1     = 0x5 << US_MR_USART_MODE_Pos,
        IRDA            = 0x8 << US_MR_USART_MODE_Pos,
        LIN_MASTER      = 0xA << US_MR_USART_MODE_Pos,
        LIN_SLAVE       = 0xB << US_MR_USART_MODE_Pos,
        SPI_MASTER      = 0xE << US_MR_USART_MODE_Pos,
        SPI_SLAVE       = 0xF << US_MR_USART_MODE_Pos
    };

    enum class USCLKS_t : uint32_t {
        MCK = 0x0 << US_MR_USCLKS_Pos,
        DIV = 0x1 << US_MR_USCLKS_Pos,
        SCK = 0x3 << US_MR_USCLKS_Pos
    };

    enum class CHRL_t : uint32_t  {
        CH_5_BIT = 0x0 << US_MR_CHRL_Pos,
        CH_6_BIT = 0x1 << US_MR_CHRL_Pos,
        CH_7_BIT = 0x2 << US_MR_CHRL_Pos,
        CH_8_BIT = 0x3 << US_MR_CHRL_Pos
    };

    enum class PAR_t : uint32_t {
        EVEN        = 0x0 << US_MR_PAR_Pos,
        ODD         = 0x1 << US_MR_PAR_Pos,
        SPACE       = 0x2 << US_MR_PAR_Pos,
        MARK        = 0x3 << US_MR_PAR_Pos,
        NO          = 0x4 << US_MR_PAR_Pos,
        MULTIDROP   = 0x6 << US_MR_PAR_Pos
    };

    enum class NBSTOP_t : uint32_t {
        STOP_1_BIT      = 0x0 << US_MR_NBSTOP_Pos,
        STOP_1_5_BIT    = 0x1 << US_MR_NBSTOP_Pos,
        STOP_2_BIT      = 0x2 << US_MR_NBSTOP_Pos
    };

    enum class CHMODE_t : uint32_t {
        CHMODE_NORMAL       = 0x0 << US_MR_CHMODE_Pos,
        AUTOMATIC           = 0x1 << US_MR_CHMODE_Pos,
        LOCAL_LOOPBACK      = 0x2 << US_MR_CHMODE_Pos,
        REMOTE_LOOPBACK     = 0x3 << US_MR_CHMODE_Pos
    };

    // USART peripherals
    template<uint8_t uartPeripheralNumber>
    struct _USARTHardware {

        static constexpr Usart * const usart() {
            switch (uartPeripheralNumber) {
                case (0): return USART0;
                case (1): return USART1;
                case (2): return USART2;
            };
        };
        static constexpr uint32_t peripheralId() {
            switch (uartPeripheralNumber) {
                case (0): return ID_USART0;
                case (1): return ID_USART1;
                case (2): return ID_USART2;
            };
            //return (uartPeripheralNumber == 0) ? ID_USART0_DONT_CONFLICT : ID_USART1_DONT_CONFLICT;
        };
        static constexpr IRQn_Type usartIRQ() {
            switch (uartPeripheralNumber) {
                case (0): return USART0_IRQn;
                case (1): return USART1_IRQn;
                case (2): return USART2_IRQn;
            };
//            return (uartPeripheralNumber == 0) ? USART0_IRQn : USART1_IRQn;
        };

        static constexpr const uint8_t uartPeripheralNum=uartPeripheralNumber;

        std::function<void(uint16_t)> _uartInterruptHandler;

        DMA<Usart *, uartPeripheralNumber> dma_ {_uartInterruptHandler};
        constexpr const DMA<Usart *, uartPeripheralNumber> *dma() { return &dma_; };

        typedef _USARTHardware<uartPeripheralNumber> this_type_t;

        static std::function<void()> _uartInterruptHandlerJumper;

        _USARTHardware()
        {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };



        void init() {
            // init is called once after reset, so clean up after a reset
            SamCommon::enablePeripheralClock(peripheralId());

            // Reset and disable TX and RX
            usart()->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;

            // Disable interrupts
            usart()->US_IDR = US_IDR_TXRDY | US_IDR_RXRDY;

            // reset PCR to zero
            usart()->US_IDR = 0xffffffff; // disable all the things
            dma()->reset();

            _uartInterruptHandlerJumper = [&]() {
                auto interruptCause = getInterruptCause();
                if (_uartInterruptHandler) {
                    _uartInterruptHandler(interruptCause);
                }
            };

            setInterrupts(UARTInterrupt::PriorityLow);
            setInterruptTxTransferDone(false);
            setInterruptRxTransferDone(false);
        };

        void enable() { usart()->US_CR = US_CR_TXEN | US_CR_RXEN; };
        void disable () { usart()->US_CR = US_CR_TXDIS | US_CR_RXDIS; };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            // Oversampling is either 8 or 16. Depending on the baud, we may need to select 8x in
            // order to get the error low.

            // For all of the speeds up to and including 230400, 16x multiplier worked fine in testing.
            // All yielded a <1% error in final baud.
            usart()->US_BRGR = US_BRGR_CD((((SamCommon::getPeripheralClockFreq() * 10) / (16 * baud)) + 5)/10) | US_BRGR_FP(0);
            usart()->US_MR &= ~US_MR_OVER;


            if (options & UARTMode::RTSCTSFlowControl) {
                usart()->US_MR = (usart()->US_MR & ~US_MR_USART_MODE_Msk) | US_MR_USART_MODE_HW_HANDSHAKING;
            } else {
                usart()->US_MR = (usart()->US_MR & ~US_MR_USART_MODE_Msk) | US_MR_USART_MODE_NORMAL;
            }

            if (options & UARTMode::TwoStopBits) {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_NBSTOP_Msk)) | US_MR_NBSTOP_2_BIT;
            } else {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_NBSTOP_Msk)) | US_MR_NBSTOP_1_BIT;
            }

            if (options & UARTMode::As9Bit) {
                usart()->US_MR |= US_MR_MODE9;
            } else {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_MODE9|US_MR_CHRL_Msk)) | static_cast<uint32_t>(CHRL_t::CH_8_BIT);
            }
            if (options & UARTMode::EvenParity) {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_EVEN;
            } else if (options & UARTMode::OddParity) {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_ODD;
            } else {
                usart()->US_MR = (usart()->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_NO;
            }

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & UARTInterrupt::OnRxDone) {
                    usart()->US_IER = US_IER_RXRDY;
                } else {
                    usart()->US_IDR = US_IDR_RXRDY;
                }
                if (interrupts & UARTInterrupt::OnTxDone) {
                    usart()->US_IER = US_IER_TXRDY;
                } else {
                    usart()->US_IDR = US_IDR_TXRDY;
                }

                if (interrupts & UARTInterrupt::OnRxTransferDone) {
                    dma()->startRxDoneInterrupts();
                } else {
                    dma()->stopRxDoneInterrupts();
                }
                if (interrupts & UARTInterrupt::OnTxTransferDone) {
                    dma()->startTxDoneInterrupts();
                } else {
                    dma()->stopTxDoneInterrupts();
                }


                /* Set interrupt priority */
                if (interrupts & UARTInterrupt::PriorityHighest) {
                    NVIC_SetPriority(usartIRQ(), 0);
                }
                else if (interrupts & UARTInterrupt::PriorityHigh) {
                    NVIC_SetPriority(usartIRQ(), 3);
                }
                else if (interrupts & UARTInterrupt::PriorityMedium) {
                    NVIC_SetPriority(usartIRQ(), 7);
                }
                else if (interrupts & UARTInterrupt::PriorityLow) {
                    NVIC_SetPriority(usartIRQ(), 11);
                }
                else if (interrupts & kInterruptPriorityLowest) {
                    NVIC_SetPriority(usartIRQ(), 15);
                }

                NVIC_EnableIRQ(usartIRQ());
            } else {

                NVIC_DisableIRQ(usartIRQ());
            }
        };

        void setInterruptHandler(std::function<void(uint16_t)> &&handler) {
            _uartInterruptHandler = std::move(handler);
        }

        void _setInterruptTxReady(bool value) {
            if (value) {
                usart()->US_IER = US_IER_TXRDY;
            } else {
                usart()->US_IDR = US_IDR_TXRDY;
            }
        };

        void setInterruptRxReady(bool value) {
            if (value) {
                usart()->US_IER = US_IER_RXRDY;
            } else {
                usart()->US_IDR = US_IDR_RXRDY;
            }
        };

        void _setInterruptCTSChange(bool value) {
            if (value) {
                usart()->US_IER = US_IER_CTSIC;
            } else {
                usart()->US_IDR = US_IDR_CTSIC;
            }
        };

        void setInterruptTxTransferDone(bool value) {
            if (value) {
                dma()->startTxDoneInterrupts();
            } else {
                dma()->stopTxDoneInterrupts();
            }
        };

        void setInterruptRxTransferDone(bool value) {
            if (value) {
                dma()->startRxDoneInterrupts();
            } else {
                dma()->stopRxDoneInterrupts();
            }
        };

        static uint16_t getInterruptCause() { // __attribute__ (( noinline ))
            uint16_t status = UARTInterrupt::Unknown;

            // Notes from experience:
            // This processor will sometimes allow one of these bits to be set,
            // even when there is no interrupt requested, and the setup conditions
            // don't appear to be done.
            // The simple but unfortunate fix is to verify that the Interrupt Mask
            // calls for that interrupt before considering it as a possible interrupt
            // source. This should be a best practice anyway, really. -Giseburt

            auto US_CSR_hold = usart()->US_CSR;
            auto US_IMR_hold = usart()->US_IMR;
            if ((US_IMR_hold & US_IMR_TXRDY) && (US_CSR_hold & US_CSR_TXRDY))
            {
                status |= UARTInterrupt::OnTxReady;
            }
            if (dma()->inTxBufferEmptyInterrupt())
            {
                status |= UARTInterrupt::OnTxTransferDone;
            }
            if ((US_IMR_hold & US_IMR_RXRDY) && (US_CSR_hold & US_CSR_RXRDY))
            {
                status |= UARTInterrupt::OnRxReady;
            }
            if (dma()->inRxBufferFullInterrupt())
            {
                status |= UARTInterrupt::OnRxTransferDone;
            }
            if ((US_IMR_hold & US_IMR_CTSIC) && (US_CSR_hold & US_CSR_CTSIC))
            {
                status |= UARTInterrupt::OnCTSChanged;
            }
            return status;
        }

        int16_t readByte() {
            if (usart()->US_CSR & US_CSR_RXRDY) {
                return (usart()->US_RHR & US_RHR_RXCHR_Msk);
            }
            return -1;
        };

        int16_t writeByte(const char value) {
            if (usart()->US_CSR & US_CSR_TXRDY) {
                usart()->US_THR = US_THR_TXCHR(value);
            }
            return -1;
        };

        void flush() {
            // Wait for the buffer to be empty
            while (!usart()->US_CSR & US_CSR_TXEMPTY) {
                ;
            }
            _tx_paused = false;
        };

        void flushRead() {
            // kill any incoming transfers
            dma()->flushRead();
        };


        // ***** Connection status check (simple)
        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // The USART gives us access to that pin.
            return (usart()->US_CSR & US_CSR_CTS) == 0; // active LOW
        };


        // ***** Handle Tranfers
        bool startRXTransfer(char *buffer, const uint16_t length) {
            const bool handleInterrupts = true;
            const bool includeNext = true;
            return dma()->startRXTransfer(buffer, length, handleInterrupts, includeNext);
        };

        char* getRXTransferPosition() {
            return dma()->getRXTransferPosition();
        };

        bool _tx_paused = false;
        bool startTXTransfer(char *buffer, const uint16_t length) {
            return dma()->startTXTransfer(buffer, length, true);
        };

        char* getTXTransferPosition() {
            if (_tx_paused) { return false; }
            return dma()->getTXTransferPosition();
        };

        void pauseTX() {
            _tx_paused = true;
            dma()->disableTx();
        };

        void resumeTX() {
            _tx_paused = false;
            dma()->enableTx();
        };
    };


    // UART peripheral
    template<uint8_t uartPeripheralNumber>
    struct _UARTHardware {

        static constexpr Uart * const uart() {
            switch (uartPeripheralNumber) {
                case (0): return UART0;
                case (1): return UART1;
                case (2): return UART2;
                case (3): return UART3;
                case (4): return UART4;
            };
        };
        static constexpr uint32_t peripheralId() {
            switch (uartPeripheralNumber) {
                case (0): return ID_UART0;
                case (1): return ID_UART1;
                case (2): return ID_UART2;
                case (3): return ID_UART3;
                case (4): return ID_UART4;
            };
        };
        static constexpr IRQn_Type uartIRQ() {
            switch (uartPeripheralNumber) {
                case (0): return UART0_IRQn;
                case (1): return UART1_IRQn;
                case (2): return UART2_IRQn;
                case (3): return UART3_IRQn;
                case (4): return UART4_IRQn;
            };
        };

        static constexpr const uint8_t uartPeripheralNum=uartPeripheralNumber;

        std::function<void(uint16_t)> _uartInterruptHandler;

        DMA<Uart *, uartPeripheralNumber> dma_ {_uartInterruptHandler};
        constexpr const DMA<Uart *, uartPeripheralNumber> *dma() { return &dma_; };

        typedef _UARTHardware<uartPeripheralNumber> this_type_t;

        static std::function<void()> _uartInterruptHandlerJumper;

        _UARTHardware()
        {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
            _uartInterruptHandlerJumper = [&]() {
                auto interruptCause = getInterruptCause();
                if (_uartInterruptHandler) {
                    _uartInterruptHandler(interruptCause);
                }
            };
        };


        void init() {
            // init is called once after reset, so clean up after a reset
            SamCommon::enablePeripheralClock(peripheralId());

            // Reset and disable TX and RX
            uart()->UART_CR = UART_CR_RSTRX | UART_CR_RSTTX | UART_CR_RXDIS | UART_CR_TXDIS;

            // Disable interrupts
            uart()->UART_IDR = UART_IDR_TXRDY | UART_IDR_RXRDY;

            // reset PCR to zero
            uart()->UART_IDR = 0xffffffff; // disable all the things
            dma()->reset();

            setInterrupts(UARTInterrupt::PriorityLow);
            setInterruptTxTransferDone(false);
            setInterruptRxTransferDone(false);
        };

        void enable() { uart()->UART_CR = UART_CR_TXEN | UART_CR_RXEN; };
        void disable () { uart()->UART_CR = UART_CR_TXDIS | UART_CR_RXDIS; };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            // Oversampling is either 8 or 16. Depending on the baud, we may need to select 8x in
            // order to get the error low.

            uart()->UART_BRGR = UART_BRGR_CD(SamCommon::getPeripheralClockFreq() / (16 * baud));

            // No hardware flow control
            // if (options & UARTMode::RTSCTSFlowControl) {
            // } else {
            // }

            // Only 1 stop bit
            // if (options & UARTMode::TwoStopBits) {
            // } else {
            // }

            // Only 8-bit mode
            // if (options & UARTMode::As9Bit) {
            // } else {
            // }

            if (options & UARTMode::EvenParity) {
                uart()->UART_MR = (uart()->UART_MR & ~(UART_MR_PAR_Msk)) | UART_MR_PAR_EVEN;
            } else if (options & UARTMode::OddParity) {
                uart()->UART_MR = (uart()->UART_MR & ~(UART_MR_PAR_Msk)) | UART_MR_PAR_ODD;
            } else {
                uart()->UART_MR = (uart()->UART_MR & ~(UART_MR_PAR_Msk)) | UART_MR_PAR_NO;
            }

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & UARTInterrupt::OnRxDone) {
                    uart()->UART_IER = UART_IER_RXRDY;
                } else {
                    uart()->UART_IDR = UART_IDR_RXRDY;
                }
                if (interrupts & UARTInterrupt::OnTxDone) {
                    uart()->UART_IER = UART_IER_TXRDY;
                } else {
                    uart()->UART_IDR = UART_IDR_TXRDY;
                }

                dma()->setInterrupts(interrupts);

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

        void setInterruptHandler(std::function<void(uint16_t)> &&handler) {
            _uartInterruptHandler = std::move(handler);
        }

        void _setInterruptTxReady(bool value) {
            if (value) {
                uart()->UART_IER = UART_IER_TXRDY;
            } else {
                uart()->UART_IDR = UART_IDR_TXRDY;
            }
        };

        void setInterruptRxReady(bool value) {
            if (value) {
                uart()->UART_IER = UART_IER_RXRDY;
            } else {
                uart()->UART_IDR = UART_IDR_RXRDY;
            }
        };

        void _setInterruptCTSChange(bool value) {
            if (value) {
            } else {
            }
        };

        void setInterruptTxTransferDone(bool value) {
            if (value) {
                dma()->startTxDoneInterrupts();
            } else {
                dma()->stopTxDoneInterrupts();
            }
        };

        void setInterruptRxTransferDone(bool value) {
            if (value) {
                dma()->startRxDoneInterrupts();
            } else {
                dma()->stopRxDoneInterrupts();
            }
        };

        uint16_t getInterruptCause() { // __attribute__ (( noinline ))
            uint16_t status = UARTInterrupt::Unknown;

            // Notes from experience:
            // This processor will sometimes allow one of these bits to be set,
            // even when there is no interrupt requested, and the setup conditions
            // don't appear to be done.
            // The simple but unfortunate fix is to verify that the Interrupt Mask
            // calls for that interrupt before considering it as a possible interrupt
            // source. This should be a best practice anyway, really. -Giseburt

            auto UART_SR_hold = uart()->UART_SR;
            auto UART_IMR_hold = uart()->UART_IMR;
            if ((UART_IMR_hold & UART_IMR_TXRDY) && (UART_SR_hold & UART_SR_TXRDY))
            {
                status |= UARTInterrupt::OnTxReady;
            }
            if (dma()->inTxBufferEmptyInterrupt())
            {
                status |= UARTInterrupt::OnTxTransferDone;
            }
            if ((UART_IMR_hold & UART_IMR_RXRDY) && (UART_SR_hold & UART_SR_RXRDY))
            {
                status |= UARTInterrupt::OnRxReady;
            }
            if (dma()->inRxBufferFullInterrupt())
            {
                status |= UARTInterrupt::OnRxTransferDone;
            }
            return status;
        }

        int16_t readByte() {
            while (!(uart()->UART_SR & UART_SR_RXRDY)) { ; }
            return (uart()->UART_RHR & UART_RHR_RXCHR_Msk);
        };

        int16_t writeByte(const char value) {
            while (!(uart()->UART_SR & UART_SR_TXRDY)) { ; }
            uart()->UART_THR = UART_THR_TXCHR(value);
            return 1;
        };

        void flush() {
            // Wait for the buffer to be empty
            while (!(uart()->UART_SR & UART_SR_TXEMPTY)) {
                ;
            }
        };

        void flushRead() {
            // kill any incoming transfers
            dma()->flushRead();
        };


        // ***** Connection status check (simple)
        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // read the CTS pin
            return true; // assume always for now
        };


        // ***** Handle Tranfers
        bool startRXTransfer(char *buffer, const uint16_t length) {
            const bool handleInterrupts = true;
            const bool includeNext = true;
            return dma()->startRXTransfer(buffer, length, handleInterrupts, includeNext);
        };

        char* getRXTransferPosition() {
            return dma()->getRXTransferPosition();
        };

        bool _tx_paused = false;
        bool startTXTransfer(char *buffer, const uint16_t length) {
            if (_tx_paused) { return false; }
            return dma()->startTXTransfer(buffer, length, true);
        };

        char* getTXTransferPosition() {
            return dma()->getTXTransferPosition();
        };

        void pauseTX() {
            _tx_paused = true;
            dma()->disableTx();
        };

        void resumeTX() {
            _tx_paused = false;
            dma()->enableTx();
        };
};

    template<uint8_t uartPeripheralNumber>
    using _USART_Or_UART = typename std::conditional< (uartPeripheralNumber < 4), _USARTHardware<uartPeripheralNumber>, _UARTHardware<uartPeripheralNumber-4>>::type;

    template<pin_number rxPinNumber, pin_number txPinNumber>
    using UARTGetHardware = typename std::conditional<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum,
        /* True:  */ _USART_Or_UART<UARTTxPin<txPinNumber>::uartNum>,
        /* False: */ _UARTHardware<0xff> // static_assert below should prevent this
    >::type;

    template<pin_number rtsPinNumber, pin_number rxPinNumber>
    constexpr const bool isRealAndCorrectRTSPin() {
        return IsUARTRTSPin<rtsPinNumber>() && (UARTRTSPin<rtsPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum);
    }
    template<pin_number ctsPinNumber, pin_number rxPinNumber>
    constexpr const bool isRealAndCorrectCTSPin() {
        return IsUARTCTSPin<ctsPinNumber>() && (UARTCTSPin<ctsPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum);
    }

//    template<uint8_t uartPeripheralNumber>
//    struct _UARTHardwareProxy {
//    };


//    template<uint8_t uartPeripheralNumber, pin_number rtsPinNumber, pin_number ctsPinNumber, typename rxBufferClass, typename txBufferClass>
//    struct _BufferedUARTHardware : _UARTHardware<uartPeripheralNumber> {
//        rxBufferClass rxBuffer;
//        txBufferClass txBuffer;
//
//        OutputPin<rtsPinNumber> rtsPin;
//        IRQPin<ctsPinNumber> ctsPin;
//
//        uint32_t txDelayAfterResume = 3;
//        uint32_t txDelayUntilTime   = 0;
//
//        bool _rtsCtsFlowControl    = false;
//        bool _xonXoffFlowControl   = false;
//        volatile bool _xonXoffCanSend       = true;
//        volatile char _xonXoffStartStop     = kUARTXOn;
//        volatile bool _xonXoffStartStopSent = true;
//
//        typedef _UARTHardware<uartPeripheralNumber> parent;
//
//        _BufferedUARTHardware() {
//        };
//
//        void init() {
//            parent::init();
//            parent::setInterrupts(UARTInterrupt::OnRxReady | UARTInterrupt::PriorityLowest);
//            _UARTHardwareProxy<uartPeripheralNumber>::uartInterruptHandler = [&](uint16_t interruptCause) { // use a closure
//                uartInterruptHandler(interruptCause);
//            };
//        };
//
//        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
//            parent::setOptions(baud, options, fromConstructor);
//
//            if (options & UARTMode::RTSCTSFlowControl && IsIRQPin<ctsPinNumber>() && !rtsPin.isNull()) {
//                _rtsCtsFlowControl = true;
////                parent::setInterruptTxReady(!canSend());
//                ctsPin.setInterrupts(kPinInterruptOnChange);
//            } else {
//                _rtsCtsFlowControl = false;
//            }
////            if (options & UARTMode::XonXoffFlowControl) {
////                _xonXoffFlowControl = true;
////            } else {
////                _xonXoffFlowControl = false;
////                _xonXoffStartStopSent = true;
////            }
//        }
//
////        void stopRx() {
////            if (_rtsCtsFlowControl) {
////                rtsPin = true;
////            }
////            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOff) {
////                _xonXoffStartStop = kUARTXOff;
////                _xonXoffStartStopSent = false;
////                parent::setInterruptTxReady(true);
////            }
////        };
////
////        void startRx() {
////            if (_rtsCtsFlowControl) {
////                rtsPin = false;
////            }
////            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOn) {
////                _xonXoffStartStop = kUARTXOn;
////                _xonXoffStartStopSent = false;
////                parent::setInterruptTxReady(true);
////            }
////        };
//
////        bool canSend() {
////            if (_rtsCtsFlowControl) {
////                return !ctsPin;
////            }
////            if (_xonXoffFlowControl) {
////                return _xonXoffCanSend;
////            }
////            return true;
////        };
//
//        void setTxDelayAfterResume(uint32_t newDelay) { txDelayAfterResume = newDelay; };
//
//        int16_t readByte() {
//            return rxBuffer.read();
//        };
//
//        int16_t writeByte(const uint8_t data) {
//            int16_t ret = txBuffer.write(data);
//            return ret;
//        };
//
//        void uartInterruptHandler(uint16_t interruptCause) {
////            if ((interruptCause & (UARTInterrupt::OnTxReady /*| UARTInterrupt::OnTxDone*/))) {
////                if (txDelayUntilTime && SysTickTimer.getValue() < txDelayUntilTime)
////                    return;
////                txDelayUntilTime = 0;
////                if (_xonXoffFlowControl) {
////                    if (_xonXoffStartStopSent == false) {
////                        parent::writeByte(_xonXoffStartStop);
////                        _xonXoffStartStopSent = true;
////                        return;
////                    }
////                }
////                int16_t value = txBuffer.read();
////                if (value >= 0) {
////                    parent::writeByte(value);
////                }
////            }
////            if (txBuffer.isEmpty() || txBuffer.isLocked()) {
////                // This is tricky: If it's write locked, we have to bail, and SHUT OFF TxReady interrupts.
////                // On the ARM, it won't return to the main code as long as there's a pending interrupt,
////                // and the txReady interrupt will continue to fire, causing deadlock.
////                parent::setInterruptTxReady(false);
////            }
////
////            if ((interruptCause & UARTInterrupt::OnRxReady) && !rxBuffer.isFull()) {
////                int16_t value = parent::readByte();
////                if (_xonXoffFlowControl) {
////                    if (value == kUARTXOn) {
////                        _xonXoffCanSend = true;
////                        return;
////                    } else if (value == kUARTXOff) {
////                        _xonXoffCanSend = false;
////                        return;
////                    }
////                }
////                // We don't double check to ensure value is not -1 -- should we?
////                rxBuffer.write(value);
////                if (rxBuffer.available() < 4) {
////                    stopRx();
////                }
////            }
//        };
//
////        void pinChangeInterrupt() {
////            txDelayUntilTime = SysTickTimer.getValue() + txDelayAfterResume;
////            parent::setInterruptTxReady(canSend());
////        };
//
//        void flush() {
//            // Wait for the buffer to be empty...
//            while(!txBuffer.isEmpty());
//        };
//    };



//    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1, typename rxBufferClass = Buffer<128>, typename txBufferClass = rxBufferClass>
//    struct BufferedUART {
//        UARTRxPin<rxPinNumber> rxPin;
//        UARTTxPin<txPinNumber> txPin;
//
//
//        _BufferedUARTHardware< UARTGetHardware<rxPinNumber, txPinNumber>::uartPeripheralNum, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass > hardware;
//
//
//        const uint8_t uartPeripheralNum() { return hardware.uartPeripheralNum; };
//
//        BufferedUART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
//            hardware.init();
//            init(baud, options, /*fromConstructor =*/ true);
//        };
//
//        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
//            setOptions(baud, options, fromConstructor);
//        };
//
//        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
//            hardware.setOptions(baud, options, fromConstructor);
//        };
//
//
//
//        int16_t readByte() {
//            return hardware.readByte();
//        };
//
//        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
//        int16_t read(const uint8_t *buffer, const uint16_t length) {
//            int16_t total_read = 0;
//            int16_t to_read = length;
//            const uint8_t *read_ptr = buffer;
//
//            // BLOCKING!!
//            while (to_read > 0) {
//                int16_t ret = hardware.readByte();
//
//                if (ret >= 0) {
//                    *read_ptr++ = ret;
//                    total_read++;
//                    to_read--;
//                }
//            };
//
//            return total_read;
//        };
//
//        int16_t writeByte(uint8_t data) {
//            return hardware.writeByte(data);
//        };
//
//        void flush() {
//            hardware.flush();
//        };
//
//        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
//        int16_t write(const char* data, const uint16_t length = 0, bool autoFlush = false) {
//            int16_t total_written = 0;
//            const char* out_ptr = data;
//            int16_t to_write = length;
//
//            if (length==0 && *out_ptr==0) {
//                return 0;
//            }
//
//            do {
//                int16_t ret = hardware.writeByte(*out_ptr);
//
//                if (ret > 0) {
//                    out_ptr++;
//                    total_written++;
//                    to_write--;
//
//                    if (length==0 && *out_ptr==0) {
//                        break;
//                    }
//                } else if (autoFlush) {
//                    flush();
//                } else {
//                    break;
//                }
//            } while (to_write);
//
//            if (autoFlush && total_written > 0)
//                flush();
//
//            return total_written;
//        };
//
//        template<uint16_t _size>
//        int16_t write(Motate::Buffer<_size> &data, const uint16_t length = 0, bool autoFlush = false) {
//            int16_t total_written = 0;
//            int16_t to_write = length;
//
//            do {
//                int16_t value = data.peek();
//                if (value < 0) // no more data
//                    break;
//
//                int16_t ret = hardware.writeByte(value);
//                if (ret > 0) {
//                    data.pop();
//                    to_write--;
//                } else if (autoFlush) {
//                    flush();
//                } else {
//                    break;
//                }
//            } while (to_write != 0);
//
//            if (autoFlush && total_written > 0)
//                flush();
//
//            return total_written;
//        };
//
//        void pinChangeInterrupt() {
//            hardware.pinChangeInterrupt();
//        };
//
//        //	// Placeholder for user code.
//        //	static void interrupt() __attribute__ ((weak));
//    };
}

#endif /* end of include guard: SAMUART_H_ONCE */
