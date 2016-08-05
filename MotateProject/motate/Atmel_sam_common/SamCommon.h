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




    // DMA template - MUST be specialized
    template<typename periph_t>
    struct DMA {
        DMA() = delete;
    };


// PDC peripherals -- if we have a PDC (deduced using PERIPH_PTSR_RXTEN)
#if defined(PERIPH_PTSR_RXTEN)

#pragma mark DMA_PDC implementation

    // DMA_PDC template - - MUST be specialized
    template<typename periph_t>
    struct DMA_PDC_hardware {
        DMA_PDC_hardware() = delete;
    };


    // generic DMA_PDC object.
    template<typename periph_t>
    struct DMA_PDC : DMA_PDC_hardware<periph_t> {
        using DMA_PDC_hardware<periph_t>::pdc;
        using DMA_PDC_hardware<periph_t>::startRxDoneInterrupts;
        using DMA_PDC_hardware<periph_t>::stopRxDoneInterrupts;
        using DMA_PDC_hardware<periph_t>::startTxDoneInterrupts;
        using DMA_PDC_hardware<periph_t>::stopTxDoneInterrupts;
        using DMA_PDC_hardware<periph_t>::inTxBufferEmptyInterrupt;
        using DMA_PDC_hardware<periph_t>::inRxBufferEmptyInterrupt;

        typedef typename DMA_PDC_hardware<periph_t>::buffer_t buffer_t;

        DMA_PDC(periph_t p) : DMA_PDC_hardware<periph_t>{p} {};

        void reset() {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS; // disable all the things
            pdc()->PERIPH_RPR = 0;
            pdc()->PERIPH_RNPR = 0;
            pdc()->PERIPH_RCR = 0;
            pdc()->PERIPH_RNCR = 0;
            pdc()->PERIPH_TPR = 0;
            pdc()->PERIPH_TNPR = 0;
            pdc()->PERIPH_TCR = 0;
            pdc()->PERIPH_TNCR = 0;
        };

        void disableRx() {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTDIS; // disable for setup
        };
        void enableRx() {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTEN;  // enable again
        };
        void setRx(void * const buffer, const uint32_t length) {
            pdc()->PERIPH_RPR = (uint32_t)buffer;
            pdc()->PERIPH_RCR = length;
        };
        void setNextRx(void * const buffer, const uint32_t length) {
            pdc()->PERIPH_RNPR = (uint32_t)buffer;
            pdc()->PERIPH_RNCR = length;
        };
        void flushRead() {
            pdc()->PERIPH_RNCR = 0;
            pdc()->PERIPH_RCR = 0;
        };
        uint32_t leftToRead(bool include_next = false) {
            if (include_next) {
                return pdc()->PERIPH_RCR + pdc()->PERIPH_RNCR;
            }
            return pdc()->PERIPH_RCR;
        };
        uint32_t leftToReadNext() {
            return pdc()->PERIPH_RNCR;
        };
        bool doneReading(bool include_next = false) {
            return leftToRead(include_next) == 0;
        };
        bool doneReadingNext() {
            return leftToReadNext() == 0;
        };
        buffer_t getRXTransferPosition() {
            return (buffer_t)pdc()->PERIPH_RPR;
        };

        // Bundle it all up
        bool startRXTransfer(void * const buffer, const uint32_t length, bool handle_interrupts = true, bool include_next = false) {
            if (doneReading()) {
                if (handle_interrupts) { stopRxDoneInterrupts(); }
                setRx(buffer, length);
                if (length != 0) {
                    if (handle_interrupts) { startRxDoneInterrupts(); }
                    enableRx();
                    return true;
                }
                return false;
            }
            else if (include_next && doneReadingNext()) {
                setNextRx(buffer, length);
                return true;
            }
            return false;
        }


        void disableTx() {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTDIS; // disable for setup
        };
        void enableTx() {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTEN;  // enable again
        };
        void setTx(void * const buffer, const uint32_t length) {
            pdc()->PERIPH_TPR = (uint32_t)buffer;
            pdc()->PERIPH_TCR = length;
        };
        void setNextTx(void * const buffer, const uint32_t length) {
            pdc()->PERIPH_TNPR = (uint32_t)buffer;
            pdc()->PERIPH_TNCR = length;
        };
        uint32_t leftToWrite(bool include_next = false) {
            if (include_next) {
                return pdc()->PERIPH_TCR + pdc()->PERIPH_TNCR;
            }
            return pdc()->PERIPH_TCR;
        };
        uint32_t leftToWriteNext() {
            return pdc()->PERIPH_TNCR;
        };
        bool doneWriting(bool include_next = false) {
            return leftToWrite(include_next) == 0;
        };
        bool doneWritingNext() {
            return leftToWriteNext() == 0;
        };
        buffer_t getTXTransferPosition() {
            return (buffer_t)pdc()->PERIPH_TPR;
        };


        // Bundle it all up
        bool startTXTransfer(void * const buffer, const uint32_t length, bool handle_interrupts = true, bool include_next = false) {
            if (doneWriting()) {
                if (handle_interrupts) { stopTxDoneInterrupts(); }
                setTx(buffer, length);
                if (length != 0) {
                    if (handle_interrupts) { startTxDoneInterrupts(); }
                    enableTx();
                    return true;
                }
                return false;
            }
            else if (include_next && doneWritingNext()) {
                setNextTx(buffer, length);
                return true;
            }
            return false;
        }
    };


// We're deducing if there's a USART and it has a PDC
// Notice that this is BEFORE SamUART.h has fixed the UART/USART defines
#if (defined(USART) || defined(USART0) || defined(USART1) || defined(USART2)) && defined(US_RPR_RXPTR_Pos)

#pragma mark DMA_PDC Usart implementation

    template<>
    struct DMA_PDC_hardware<Usart*> {
        Usart * const usart;
        DMA_PDC_hardware(Usart *u) : usart{u} {};

        typedef char* buffer_t ;

        Pdc * const pdc()
        {
#if defined(USART)
            if (usart == USART) {
                return PDC_USART;
            }
#endif
#if defined(USART0)
            if (usart == USART0) {
                return PDC_USART0;
            }
#endif
#if defined(USART1)
            if (usart == USART1) {
                return PDC_USART1;
            }
#endif
#if defined(USART2)
            if (usart == USART2) {
                return PDC_USART2;
            }
#endif
            return nullptr;
        };

        void startRxDoneInterrupts() { usart->US_IER = US_IER_RXBUFF; };
        void stopRxDoneInterrupts() { usart->US_IDR = US_IDR_RXBUFF; };
        void startTxDoneInterrupts() { usart->US_IER = US_IER_TXBUFE; };
        void stopTxDoneInterrupts() { usart->US_IDR = US_IDR_TXBUFE; };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (usart->US_IMR & US_IMR_RXBUFF) {
                // then we read the status register
                return (usart->US_CSR & US_CSR_RXBUFF);
            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (usart->US_IMR & US_IMR_TXBUFE) {
                // then we read the status register
                return (usart->US_CSR & US_CSR_TXBUFE);
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<>
    struct DMA<Usart*> : DMA_PDC<Usart*> {
        DMA(Usart *u) : DMA_PDC<Usart*>{u} {};
    };
#endif // USART + PDC

// We're deducing if there's a UART and it has a PDC
    // Notice that this is BEFORE SamUART.h has fixed the UART/USART defines
#if (defined(UART) || defined(UART0) || defined(UART1) || defined(UART2))

#pragma mark DMA_PDC Usart implementation

    template<>
    struct DMA_PDC_hardware<Uart*> {
        Uart * const uart;
        DMA_PDC_hardware(Uart *u) : uart{u} {};

        typedef char* buffer_t ;

        Pdc * const pdc()
        {
#if defined(UART)
            if (uart == UART) {
                return PDC_UART;
            }
#endif
#if defined(UART0)
            if (uart == UART0) {
                return PDC_UART0;
            }
#endif
#if defined(UART1)
            if (uart == UART1) {
                return PDC_UART1;
            }
#endif
#if defined(UART2)
            if (uart == UART2) {
                return PDC_UART2;
            }
#endif
            return nullptr;
        };

        void startRxDoneInterrupts() { uart->UART_IER = UART_IER_RXBUFF; };
        void stopRxDoneInterrupts() { uart->UART_IDR = UART_IDR_RXBUFF; };
        void startTxDoneInterrupts() { uart->UART_IER = UART_IER_TXBUFE; };
        void stopTxDoneInterrupts() { uart->UART_IDR = UART_IDR_TXBUFE; };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (uart->UART_IMR & UART_IMR_RXBUFF) {
                // then we read the status register
                return (uart->UART_SR & UART_SR_RXBUFF);
            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (uart->UART_IMR & UART_IMR_TXBUFE) {
                // then we read the status register
                return (uart->UART_SR & UART_SR_TXBUFE);
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<>
    struct DMA<Uart*> : DMA_PDC<Uart*> {
        DMA(Uart *u) : DMA_PDC<Uart*>{u} {};
    };
#endif // UART + PDC

#else // if has PDC -> does not have PDC



#endif // does not have PDC

} // namespace Motate

#endif /* end of include guard: SAMCOMMON_H_ONCE */