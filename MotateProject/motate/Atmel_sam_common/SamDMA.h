/*
 utility/SamDMA.h - Library for the Motate system
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

#ifndef SAMDMA_H_ONCE
#define SAMDMA_H_ONCE

#include "SamCommon.h" // pull in defines and fix them

namespace Motate {


    // DMA template - MUST be specialized
    template<typename periph_t, uint8_t periph_num>
    struct DMA {
        DMA() = delete; // this prevents accidental direct instantiation
    };


    // So far there are two proimary types of DMA that we support:
    // PDC (Peripheral DMA Controller), which is the DMA built into many
    //   peripherals of the sam3x* and sam4e* lines.
    // XDMAC (eXtensible DMA), which is the global DMA controller of the
    //   SAMS70 (and family).


// PDC peripherals -- if we have a PDC (deduced using PERIPH_PTSR_RXTEN)
#ifdef HAS_PDC

#pragma mark DMA_PDC implementation

    // DMA_PDC_hardware template - - MUST be specialized
    template<typename periph_t, uint8_t periph_num>
    struct DMA_PDC_hardware {
        DMA_PDC_hardware() = delete; // this prevents accidental direct instantiation
    };


    // generic DMA_PDC object.
    template<typename periph_t, uint8_t periph_num>
    struct DMA_PDC : DMA_PDC_hardware<periph_t, periph_num> {
        typedef DMA_PDC_hardware<periph_t, periph_num> _hw;
        using _hw::pdc;
        using _hw::startRxDoneInterrupts;
        using _hw::stopRxDoneInterrupts;
        using _hw::startTxDoneInterrupts;
        using _hw::stopTxDoneInterrupts;
        using _hw::inTxBufferEmptyInterrupt;
        using _hw::inRxBufferEmptyInterrupt;

        typedef typename _hw::buffer_t buffer_t;

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
// Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_PDC_USART0

#pragma mark DMA_PDC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_PDC_hardware<Usart*, uartPeripheralNumber> {
        // this is identical to in SamUART
        static constexpr Usart * const usart() {
            return (uartPeripheralNumber == 0) ? USART0 : USART1;
        };

        static constexpr Pdc * const pdc()
        {
            return (uartPeripheralNumber == 0) ? PDC_USART0 : PDC_USART1;
        };

        typedef char* buffer_t ;

        void startRxDoneInterrupts() { usart()->US_IER = US_IER_RXBUFF; };
        void stopRxDoneInterrupts() { usart()->US_IDR = US_IDR_RXBUFF; };
        void startTxDoneInterrupts() { usart()->US_IER = US_IER_TXBUFE; };
        void stopTxDoneInterrupts() { usart()->US_IDR = US_IDR_TXBUFE; };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (usart()->US_IMR & US_IMR_RXBUFF) {
                // then we read the status register
                return (usart()->US_CSR & US_CSR_RXBUFF);
            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (usart()->US_IMR & US_IMR_TXBUFE) {
                // then we read the status register
                return (usart()->US_CSR & US_CSR_TXBUFE);
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Usart*, periph_num> : DMA_PDC<Usart*, periph_num> {
        // nothing to do here.
    };
#endif // USART + PDC

// We're deducing if there's a UART and it has a PDC
// Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_PDC_UART0

#pragma mark DMA_PDC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_PDC_hardware<Uart*, uartPeripheralNumber> {
        static constexpr Uart * const uart() {
            return (uartPeripheralNumber == 0) ? UART0 : UART1;
        };

        typedef char* buffer_t ;

        Pdc * const pdc()
        {
            return (uartPeripheralNumber == 0) ? PDC_UART0 : PDC_UART1;
        };

        void startRxDoneInterrupts() { uart()->UART_IER = UART_IER_RXBUFF; };
        void stopRxDoneInterrupts() { uart()->UART_IDR = UART_IDR_RXBUFF; };
        void startTxDoneInterrupts() { uart()->UART_IER = UART_IER_TXBUFE; };
        void stopTxDoneInterrupts() { uart()->UART_IDR = UART_IDR_TXBUFE; };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (uart()->UART_IMR & UART_IMR_RXBUFF) {
                // then we read the status register
                return (uart()->UART_SR & UART_SR_RXBUFF);
            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
            if (uart()->UART_IMR & UART_IMR_TXBUFE) {
                // then we read the status register
                return (uart()->UART_SR & UART_SR_TXBUFE);
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Uart*, periph_num> : DMA_PDC<Uart*, periph_num> {
        // nothing to do here.
    };
#endif // UART + PDC

#endif // if has PDC

// Now if we have an XDMAC, we use it
#if defined(XDMAC)

    // DMA_XDMAC_hardware template - - MUST be specialized
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_hardware {
        DMA_XDMAC_hardware() = delete;
    };


    // generic DMA_XDMAC object.
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC : DMA_XDMAC_hardware<periph_t, periph_num> {
        typedef DMA_XDMAC_hardware<periph_t, periph_num> _hw;
        using _hw::startRxDoneInterrupts;
        using _hw::stopRxDoneInterrupts;
        using _hw::startTxDoneInterrupts;
        using _hw::stopTxDoneInterrupts;
        using _hw::inTxBufferEmptyInterrupt;
        using _hw::inRxBufferEmptyInterrupt;

        typedef typename _hw::buffer_t buffer_t;

        void reset() {
//            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS; // disable all the things
//            pdc()->PERIPH_RPR = 0;
//            pdc()->PERIPH_RNPR = 0;
//            pdc()->PERIPH_RCR = 0;
//            pdc()->PERIPH_RNCR = 0;
//            pdc()->PERIPH_TPR = 0;
//            pdc()->PERIPH_TNPR = 0;
//            pdc()->PERIPH_TCR = 0;
//            pdc()->PERIPH_TNCR = 0;
        };

        void disableRx() {
//            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTDIS; // disable for setup
        };
        void enableRx() {
//            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTEN;  // enable again
        };
        void setRx(void * const buffer, const uint32_t length) {
//            pdc()->PERIPH_RPR = (uint32_t)buffer;
//            pdc()->PERIPH_RCR = length;
        };
        void setNextRx(void * const buffer, const uint32_t length) {
//            pdc()->PERIPH_RNPR = (uint32_t)buffer;
//            pdc()->PERIPH_RNCR = length;
        };
        void flushRead() {
//            pdc()->PERIPH_RNCR = 0;
//            pdc()->PERIPH_RCR = 0;
        };
        uint32_t leftToRead(bool include_next = false) {
            return 0;
            if (include_next) {
//                return pdc()->PERIPH_RCR + pdc()->PERIPH_RNCR;
            }
//            return pdc()->PERIPH_RCR;
        };
        uint32_t leftToReadNext() {
            return 0;
//            return pdc()->PERIPH_RNCR;
        };
        bool doneReading(bool include_next = false) {
            return leftToRead(include_next) == 0;
        };
        bool doneReadingNext() {
            return leftToReadNext() == 0;
        };
        buffer_t getRXTransferPosition() {
            return nullptr;
//            return (buffer_t)pdc()->PERIPH_RPR;
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
//            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTDIS; // disable for setup
        };
        void enableTx() {
//            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTEN;  // enable again
        };
        void setTx(void * const buffer, const uint32_t length) {
//            pdc()->PERIPH_TPR = (uint32_t)buffer;
//            pdc()->PERIPH_TCR = length;
        };
        void setNextTx(void * const buffer, const uint32_t length) {
//            pdc()->PERIPH_TNPR = (uint32_t)buffer;
//            pdc()->PERIPH_TNCR = length;
        };
        uint32_t leftToWrite(bool include_next = false) {
            return 0;
            if (include_next) {
//                return pdc()->PERIPH_TCR + pdc()->PERIPH_TNCR;
            }
//            return pdc()->PERIPH_TCR;
        };
        uint32_t leftToWriteNext() {
            return 0;
//            return pdc()->PERIPH_TNCR;
        };
        bool doneWriting(bool include_next = false) {
            return leftToWrite(include_next) == 0;
        };
        bool doneWritingNext() {
            return leftToWriteNext() == 0;
        };
        buffer_t getTXTransferPosition() {
            return nullptr;
//            return (buffer_t)pdc()->PERIPH_TPR;
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

    // NOTE: If we have an XDMAC peripheral, we don't have PDC, and it's the
    // only want to DMC all of these peripherals.
    // So we don't need the XDMAC deduction -- it's already done.

    // We're deducing if there's a USART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_USART0

#pragma mark DMA_XDMAC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_hardware<Usart*, uartPeripheralNumber> {
        // this is identical to in SamUART
        static constexpr Usart * const usart() {
            return (uartPeripheralNumber == 0) ? USART0 : USART1;
        };

        typedef char* buffer_t ;

        void startRxDoneInterrupts() { /*usart()->US_IER = US_IER_RXBUFF;*/ };
        void stopRxDoneInterrupts() { /*usart()->US_IDR = US_IDR_RXBUFF;*/ };
        void startTxDoneInterrupts() { /*usart()->US_IER = US_IER_TXBUFE;*/ };
        void stopTxDoneInterrupts() { /*usart()->US_IDR = US_IDR_TXBUFE;*/ };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
//            if (usart()->US_IMR & US_IMR_RXBUFF) {
//                // then we read the status register
//                return (usart()->US_CSR & US_CSR_RXBUFF);
//            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
//            if (usart()->US_IMR & US_IMR_TXBUFE) {
//                // then we read the status register
//                return (usart()->US_CSR & US_CSR_TXBUFE);
//            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Usart*, periph_num> : DMA_XDMAC<Usart*, periph_num> {
        // nothing to do here.
    };
#endif // USART + XDMAC

    // We're deducing if there's a UART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_UART0

#pragma mark DMA_XDMAC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_hardware<Uart*, uartPeripheralNumber> {
        static constexpr Uart * const uart() {
            return (uartPeripheralNumber == 0) ? UART0 : UART1;
        };

        typedef char* buffer_t ;

        void startRxDoneInterrupts() { /*usart()->US_IER = US_IER_RXBUFF;*/ };
        void stopRxDoneInterrupts() { /*usart()->US_IDR = US_IDR_RXBUFF;*/ };
        void startTxDoneInterrupts() { /*usart()->US_IER = US_IER_TXBUFE;*/ };
        void stopTxDoneInterrupts() { /*usart()->US_IDR = US_IDR_TXBUFE;*/ };

        bool inRxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
//            if (uart()->UART_IMR & UART_IMR_RXBUFF) {
//                // then we read the status register
//                return (uart()->UART_SR & UART_SR_RXBUFF);
//            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() {
            // we check if the interupt is enabled
//            if (uart()->UART_IMR & UART_IMR_TXBUFE) {
//                // then we read the status register
//                return (uart()->UART_SR & UART_SR_TXBUFE);
//            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Uart*, periph_num> : DMA_XDMAC<Uart*, periph_num> {
        // nothing to do here.
    };
#endif // UART + XDMAC

#endif // does not have XDMAC

} // namespace Motate

#endif /* end of include guard: SAMDMA_H_ONCE */
