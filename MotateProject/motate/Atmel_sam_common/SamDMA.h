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

// These two go outside the guard, so they can happen even though they
// (eventually) include this file.
//#include "MotateUART.h" // pull in definitions of UART enums
//#include "MotateTimers.h" // pull in definitions of PWM enums
#include "SamCommon.h" // pull in defines and fix them
#include "MotateCommon.h"


#ifndef SAMDMA_H_ONCE
#define SAMDMA_H_ONCE

#include <functional>  // for std::function
#include <type_traits> // for std::alignment_of and std::remove_pointer

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
    struct DMA_PDC : DMA_PDC_hardware<periph_t, periph_num>
    {
        typedef DMA_PDC_hardware<periph_t, periph_num> _hw;
        using _hw::pdc;
        using _hw::startRxDoneInterrupts;
        using _hw::stopRxDoneInterrupts;
        using _hw::startTxDoneInterrupts;
        using _hw::stopTxDoneInterrupts;
        using _hw::inTxBufferEmptyInterrupt;
        using _hw::inRxBufferFullInterrupt;
        using _hw::readByte;

        typedef typename _hw::buffer_t buffer_t;

        // We don't handle interrupts here, but this is part of the interface, so we silently deal with it
        void setInterrupts(const uint16_t interrupts) const {
            stopRxDoneInterrupts();
            stopTxDoneInterrupts();
        };

        void reset() const
        {
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

        void disableRx() const
        {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTDIS; // disable for setup
        };
        void enableRx() const
        {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_RXTEN;  // enable
        };
        void setRx(void * const buffer, const uint32_t length) const
        {
            pdc()->PERIPH_RPR = (uint32_t)buffer;
            pdc()->PERIPH_RCR = length;
        };
        void setNextRx(void * const buffer, const uint32_t length) const
        {
            pdc()->PERIPH_RNPR = (uint32_t)buffer;
            pdc()->PERIPH_RNCR = length;
        };
        void flushRead() const {
            pdc()->PERIPH_RNCR = 0;
            pdc()->PERIPH_RCR = 0;
        };
        uint32_t leftToRead(bool include_next = false) const
        {
            if (pdc()->PERIPH_RPR == 0) { return 0; }
            if (include_next) {
                return pdc()->PERIPH_RCR + pdc()->PERIPH_RNCR;
            }
            return pdc()->PERIPH_RCR;
        };
        uint32_t leftToReadNext() const
        {
            if (pdc()->PERIPH_RNPR == 0) { return 0; }
            return pdc()->PERIPH_RNCR;
        };
        bool doneReading(bool include_next = false) const
        {
            return leftToRead(include_next) == 0;
        };
        bool doneReadingNext() const {
            return leftToReadNext() == 0;
        };
        buffer_t getRXTransferPosition() const
        {
            return (buffer_t)pdc()->PERIPH_RPR;
        };

        // Bundle it all up
        bool startRXTransfer(void * const buffer,
                             const uint32_t length,
                             bool handle_interrupts = true,
                             bool include_next = false
                             ) const
        {
            if (0 == length) { return false; }

            if (doneReading()) {
                if (handle_interrupts) { stopRxDoneInterrupts(); }
//                disableRx(); // make it stand still first

                setRx(buffer, length);

                enableRx();
                if (handle_interrupts) { startRxDoneInterrupts(); }
            }
            // check to see if they overlap, in which case we're extending the region
            else if ((pdc()->PERIPH_RPR >= (uint32_t)buffer) &&
                     (pdc()->PERIPH_RPR < ((uint32_t)buffer + length))
                    )
            {
                if (handle_interrupts) { stopRxDoneInterrupts(); }

                // they overlap, we need to compute the new length
                decltype(pdc()->PERIPH_RPR) pos_save;
                do {
                    pos_save = pdc()->PERIPH_RPR;

                    // new_length = (start_pos + length) - current_positon
                    pdc()->PERIPH_RCR = ((uint32_t)buffer + length) - pos_save;

                    // catch rare case where it advances while we were computing
                } while (pdc()->PERIPH_RPR > pos_save);

                enableRx();
                if (handle_interrupts) { startRxDoneInterrupts(); }
            }
            // otherwise, we set the next region, if requested. We DON'T attempt to extend it.
            else if (include_next && doneReadingNext()) {
                setNextRx(buffer, length);
                return true;
            }

            return (length > 0);
        }


        void disableTx() const
        {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTDIS; // disable for setup
        };
        void enableTx() const
        {
            pdc()->PERIPH_PTCR = PERIPH_PTCR_TXTEN;  // enable again
        };
        void setTx(void * const buffer, const uint32_t length) const
        {
            pdc()->PERIPH_TPR = (uint32_t)buffer;
            pdc()->PERIPH_TCR = length;
        };
        void setNextTx(void * const buffer, const uint32_t length) const
        {
            pdc()->PERIPH_TNPR = (uint32_t)buffer;
            pdc()->PERIPH_TNCR = length;
        };
        uint32_t leftToWrite(bool include_next = false) const
        {
            if (include_next) {
                return pdc()->PERIPH_TCR + pdc()->PERIPH_TNCR;
            }
            return pdc()->PERIPH_TCR;
        };
        uint32_t leftToWriteNext() const
        {
            return pdc()->PERIPH_TNCR;
        };
        bool doneWriting(bool include_next = false) const
        {
            return leftToWrite(include_next) == 0;
        };
        bool doneWritingNext() const
        {
            return leftToWriteNext() == 0;
        };
        buffer_t getTXTransferPosition() const
        {
            return (buffer_t)pdc()->PERIPH_TPR;
        };


        // Bundle it all up
        bool startTXTransfer(void * const buffer, const uint32_t length, bool handle_interrupts = true, bool include_next = false) const
        {
            if (doneWriting()) {
                stopTxDoneInterrupts();
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
    struct DMA_PDC_hardware<Usart*, uartPeripheralNumber>
    {
        // this is identical to in SamUART
        static constexpr Usart * const usart()
        {
            return (uartPeripheralNumber == 0) ? USART0 : USART1;
        };

        static constexpr Pdc * const pdc()
        {
            return (uartPeripheralNumber == 0) ? PDC_USART0 : PDC_USART1;
        };

        typedef char* buffer_t ;

        void startRxDoneInterrupts(const bool include_next = false) const {
            usart()->US_IER = include_next ? US_IER_RXBUFF : US_IER_ENDRX;
        };
        void stopRxDoneInterrupts(const bool include_next = false) const {
            usart()->US_IDR = include_next ? US_IDR_RXBUFF : US_IDR_ENDRX;
        };
        void startTxDoneInterrupts(const bool include_next = true) const {
            usart()->US_IER = include_next ? US_IER_TXBUFE : US_IDR_ENDTX;
        };
        void stopTxDoneInterrupts(const bool include_next = true) const {
            usart()->US_IDR = include_next ? US_IDR_TXBUFE : US_IDR_ENDTX;
        };

        int16_t readByte() const {
            if (!(usart()->US_CSR & US_CSR_RXRDY)) { return -1; }
            return (usart()->US_RHR & US_RHR_RXCHR_Msk);
        }

        bool inRxBufferFullInterrupt() const
        {
            // we check if the interupt is enabled, then read it if it is
            if ((usart()->US_IMR & US_IMR_RXBUFF) &&
                (usart()->US_CSR & US_CSR_RXBUFF)
                )
            {
                return true;
            }
            if ((usart()->US_IMR & US_IMR_ENDRX) &&
                (usart()->US_CSR & US_CSR_ENDRX)
                )
            {
                return true;
            }
            return false;
        }

        bool inTxBufferEmptyInterrupt() const
        {
            // we check if the interupt is enabled, then read it if it is
            if ((usart()->US_IMR & US_IMR_TXBUFE) &&
                (usart()->US_CSR & US_CSR_TXBUFE)
                )
            {
                return true;
            }
            if ((usart()->US_IMR & US_IMR_ENDTX) &&
                (usart()->US_CSR & US_CSR_ENDTX)
                )
            {
                return true;
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Usart*, periph_num> : DMA_PDC<Usart*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        // we take the handler, but we don't actually handle interrupts for the peripheral
        // so we ignore it
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_PDC<Usart*, periph_num>{} {};
    };
#endif // USART + PDC

// We're deducing if there's a UART and it has a PDC
// Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_PDC_UART0

#pragma mark DMA_PDC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_PDC_hardware<Uart*, uartPeripheralNumber> {
        static constexpr Uart * const uart()
        {
            return (uartPeripheralNumber == 0) ? UART0 : UART1;
        };

        typedef char* buffer_t ;

        static constexpr Pdc * const pdc()
        {
            return (uartPeripheralNumber == 0) ? PDC_UART0 : PDC_UART1;
        };

        void startRxDoneInterrupts(const bool include_next = false) const {
            uart()->UART_IER = include_next ? UART_IER_RXBUFF : UART_IER_ENDRX;
        };
        void stopRxDoneInterrupts(const bool include_next = false) const {
            uart()->UART_IDR = include_next ? UART_IDR_RXBUFF : UART_IDR_ENDRX;
        };
        void startTxDoneInterrupts(const bool include_next = true) const {
            uart()->UART_IER = include_next ? UART_IER_TXBUFE : UART_IER_ENDTX;
        };
        void stopTxDoneInterrupts(const bool include_next = true) const {
            uart()->UART_IDR = include_next ? UART_IDR_TXBUFE : UART_IDR_ENDTX;
        };

        int16_t readByte() const {
            if (!(uart()->UART_SR & UART_SR_RXRDY)) { return -1; }
            return (uart()->UART_RHR & UART_RHR_RXCHR_Msk);
        }

        bool inRxBufferFullInterrupt() const
        {
            // we check if the interupt is enabled, then read it if it is
            if ((uart()->UART_IMR & UART_IMR_RXBUFF) &&
                (uart()->UART_SR & UART_SR_RXBUFF)
                )
            {
                return true;
            }
            if ((uart()->UART_IMR & UART_IMR_ENDRX) &&
                (uart()->UART_SR & UART_SR_ENDRX)
                )
            {
                return true;
            }

            return false;
        }

        bool inTxBufferEmptyInterrupt() const
        {
            // we check if the interupt is enabled, then read it if it is
            if ((uart()->UART_IMR & UART_IMR_TXBUFE) &&
                (uart()->UART_SR & UART_SR_TXBUFE)
                )
            {
                return true;
            }
            if ((uart()->UART_IMR & UART_IMR_ENDTX) &&
                (uart()->UART_SR & UART_SR_ENDTX)
                )
            {
                return true;
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Uart*, periph_num> : DMA_PDC<Uart*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        // we take the handler, but we don't actually handle interrupts for the peripheral
        // so we ignore it
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_PDC<Uart*, periph_num>{} {};
    };
#endif // UART + PDC

#endif // if has PDC


} // end namespace Motate



// Now if we have an XDMAC, we use it
#if defined(XDMAC)

namespace Motate {

    // DMA_XDMAC_hardware template - - MUST be specialized
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_hardware {
        DMA_XDMAC_hardware() = delete;
    };
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_RX_hardware {
        DMA_XDMAC_RX_hardware() = delete;
    };
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_TX_hardware {
        DMA_XDMAC_TX_hardware() = delete;
    };

    struct DMA_XDMAC_common {
        static constexpr uint32_t peripheralId() { return ID_XDMAC; };
        static constexpr Xdmac * const xdma() { return XDMAC; };
        static constexpr IRQn_Type xdmaIRQ() { return XDMAC_IRQn; };

        void setInterrupts(const uint16_t interrupts) const
        {
            if (interrupts != Interrupt::Off) {
//                if (interrupts & Interrupt::OnTxTransferDone) {
//                    startTxDoneInterrupts();
//                } else {
//                    stopTxDoneInterrupts();
//                }
//
//                if (interrupts & Interrupt::OnRxTransferDone) {
//                    startRxDoneInterrupts();
//                } else {
//                    stopRxDoneInterrupts();
//                }


                /* Set interrupt priority */
                if (interrupts & Interrupt::PriorityHighest) {
                    NVIC_SetPriority(xdmaIRQ(), 0);
                }
                else if (interrupts & Interrupt::PriorityHigh) {
                    NVIC_SetPriority(xdmaIRQ(), 3);
                }
                else if (interrupts & Interrupt::PriorityMedium) {
                    NVIC_SetPriority(xdmaIRQ(), 7);
                }
                else if (interrupts & Interrupt::PriorityLow) {
                    NVIC_SetPriority(xdmaIRQ(), 11);
                }
                else if (interrupts & Interrupt::PriorityLowest) {
                    NVIC_SetPriority(xdmaIRQ(), 15);
                }

                NVIC_EnableIRQ(xdmaIRQ());
            } else {

                NVIC_DisableIRQ(xdmaIRQ());
            }
        };

    };

    struct _XDMAInterrupt {
        const uint32_t channel_mask; // Pin uses "mask" so we use a different name. "pc" for pinChange
        const std::function<void(void)> interrupt_handler;
        _XDMAInterrupt *next;

        _XDMAInterrupt(const _XDMAInterrupt &) = delete; // delete the copy constructor, we only allow moves
        _XDMAInterrupt &operator=(const _XDMAInterrupt &) = delete; // delete the assigment operator, we only allow moves

        // Note we MOVE this interrupt function...
        _XDMAInterrupt(const uint32_t _channel_num, const std::function<void(void)> &&_interrupt, _XDMAInterrupt *&_first) : channel_mask{(uint32_t)(1<<_channel_num)}, interrupt_handler{std::move(_interrupt)}, next{nullptr}
        {
            if (interrupt_handler) { // std::function returns false if the function isn't valid
                if (_first == nullptr) {
                    _first = this;
                    return;
                }

                _XDMAInterrupt *i = _first;
                while (i->next != nullptr) {
                    i = i->next;
                }
                i->next = this;
            }
        };
    };

    extern _XDMAInterrupt *_first_xdmac_interrupt;

    // NOTE, we have 23 channels, and less than 23 peripheral types using this,
    // so we'll assign channels uniquely, but otherwise arbitrarily from lowest
    // to highest. If using XDMAC directly, beware and use the highest channels
    // first.

    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_TX : virtual DMA_XDMAC_TX_hardware<periph_t, periph_num> {
        typedef DMA_XDMAC_TX_hardware<periph_t, periph_num> _hw;

        using _hw::xdma;
        using _hw::xdmaTxPeripheralId;
        using _hw::xdmaTxChannelNumber;
        using _hw::xdmaTxChannel;
        using _hw::xdmaPeripheralTxAddress;
        using _hw::xdmaIRQ;
        using _hw::peripheralId;

        typedef typename _hw::buffer_t buffer_t;
        static constexpr uint32_t buffer_width = std::alignment_of< typename std::remove_pointer<buffer_t>::type >::value;


        const std::function<void(uint16_t)> &_xdmaInterruptHandler;

        _XDMAInterrupt _tx_interrupt {xdmaTxChannelNumber(), [&](){
            if (_xdmaInterruptHandler) {
                _xdmaInterruptHandler(Interrupt::OnTxTransferDone);
            }
        }, _first_xdmac_interrupt};

        // we'll hold a reference to the handler, the peripheral owns the one it's passing
        constexpr DMA_XDMAC_TX(const std::function<void(uint16_t)> &handler) : _xdmaInterruptHandler{handler} {};

        void resetTX() const
        {
            // init is called once after reset, so clean up after a reset
            SamCommon::enablePeripheralClock(peripheralId());

            // disable the channels
            disableTx();

            // Configure the Tx
            // ASSUMPTIONS:
            //  * Tx is memory to peripheral
            //  * Not doing memory-to-memory or peripheral-to-peripheral (for now)
            //  * Single Block, Single Microblock transfers (for now)
            //  * All peripherals are using a FIFO for Rx and Tx
            //
            // If ANY of those assumptions are wrong, this code must change!!

            // Configure Tx channel
            xdmaTxChannel()->XDMAC_CSA = 0;
            xdmaTxChannel()->XDMAC_CDA = (uint32_t)xdmaPeripheralTxAddress();
            xdmaTxChannel()->XDMAC_CC =
            XDMAC_CC_TYPE_PER_TRAN | // between memory and a peripheral
            XDMAC_CC_MBSIZE_SINGLE | // burst size of one "unit" at a time
            XDMAC_CC_DSYNC_MEM2PER | // memory->peripheral
            XDMAC_CC_CSIZE_CHK_1   | // chunk size of one "unit" at a time
            XDMAC_CC_DWIDTH( (buffer_width >> 1) ) | // data width (based on alignment size of base type of buffer_t)
            XDMAC_CC_SIF_AHB_IF0   | // source is RAM   (info cryptically extracted from Table 18-3 of the datasheep)
            XDMAC_CC_DIF_AHB_IF1   | // destination is peripheral (info cryptically extracted from Table 18-3 of the datasheep)
            XDMAC_CC_SAM_INCREMENTED_AM  | // the source address increments as written
            XDMAC_CC_DAM_FIXED_AM        | // destination address doesn't change (FIFO)
            XDMAC_CC_PERID(xdmaTxPeripheralId()) // and finally, set the peripheral identifier
            ;
            // Datasheep says to clear these out explicitly:
            //            xdmaTxChannel()->XDMAC_CNDC = 0; // no "next descriptor"
            //            xdmaTxChannel()->XDMAC_CBC = 0;  // ???
            //            xdmaTxChannel()->XDMAC_CDS_MSP = 0; // striding is disabled
            //            xdmaTxChannel()->XDMAC_CSUS = 0;
            //            xdmaTxChannel()->XDMAC_CDUS = 0;
            //            xdmaTxChannel()->XDMAC_CUBC = 0;

            // enable interrupts for these channels (must still be masked individually
            xdma()->XDMAC_GIE |= (1<<xdmaTxChannelNumber());
        };


        void disableTx() const
        {
            xdma()->XDMAC_GD = XDMAC_GID_ID0 << xdmaTxChannelNumber();
        };
        void enableTx() const
        {
            xdma()->XDMAC_GE = XDMAC_GIE_IE0 << xdmaTxChannelNumber();
        };
        void setTx(void * const buffer, const uint32_t length) const
        {
            xdmaTxChannel()->XDMAC_CSA = (uint32_t)buffer;
            xdmaTxChannel()->XDMAC_CUBC = length;
        };
        void setNextTx(void * const buffer, const uint32_t length) const
        {
            //            pdc()->PERIPH_TNPR = (uint32_t)buffer;
            //            pdc()->PERIPH_TNCR = length;
        };
        uint32_t leftToWrite(bool include_next = false) const
        {
            //            if (include_next) {
            //            }
            SamCommon::sync();
            return xdmaTxChannel()->XDMAC_CUBC;
        };
        uint32_t leftToWriteNext() const
        {
            return 0;
            //            return pdc()->PERIPH_TNCR;
        };
        bool doneWriting(bool include_next = false) const
        {
            return leftToWrite(include_next) == 0;
        };
        bool doneWritingNext() const
        {
            return leftToWriteNext() == 0;
        };
        buffer_t getTXTransferPosition() const
        {
            return (buffer_t)xdmaTxChannel()->XDMAC_CSA;
        };


        // Bundle it all up
        bool startTXTransfer(void * const buffer, const uint32_t length, bool handle_interrupts = true, bool include_next = false) const
        {
            if (doneWriting()) {
                disableTx();
                if (handle_interrupts) { stopTxDoneInterrupts(); }
                setTx(buffer, length);
                if (length != 0) {
                    if (handle_interrupts) { startTxDoneInterrupts(); }
                    enableTx();
                    return true;
                }
                return false;
            }
//            else if (include_next && doneWritingNext()) {
//                setNextTx(buffer, length);
//                return true;
//            }
            return false;
        };


        void startTxDoneInterrupts() const { xdmaTxChannel()->XDMAC_CIE = XDMAC_CIE_BIE; };
        void stopTxDoneInterrupts() const { xdmaTxChannel()->XDMAC_CID = XDMAC_CID_BID; };

        // XDMAC_Handler is handled with _tx_interrupt and _rx_interupt. They use
        // the std::function _xdmaInterruptHandler, which get's set by the peripheral
        // in the constexpr constructor.

        // These two get called from the peripheral interrupt handler,
        // and must always return false
        constexpr bool inTxBufferEmptyInterrupt() const { return false; };
    };

    // DMA_XDMAC is split into two halves, TX and RX. Each or both can be inherited from.
    template<typename periph_t, uint8_t periph_num>
    struct DMA_XDMAC_RX : virtual DMA_XDMAC_RX_hardware<periph_t, periph_num> {
        typedef DMA_XDMAC_RX_hardware<periph_t, periph_num> _hw;

        using _hw::xdma;
        using _hw::xdmaRxPeripheralId;
        using _hw::xdmaRxChannelNumber;
        using _hw::xdmaRxChannel;
        using _hw::xdmaPeripheralRxAddress;
        using _hw::xdmaIRQ;
        using _hw::peripheralId;

        typedef typename _hw::buffer_t buffer_t;
        static constexpr uint32_t buffer_width = std::alignment_of< typename std::remove_pointer<buffer_t>::type >::value;

        const std::function<void(uint16_t)> &_xdmaInterruptHandler;

        _XDMAInterrupt _rx_interrupt {xdmaRxChannelNumber(), [&](){
            if (_xdmaInterruptHandler) {
                _xdmaInterruptHandler(Interrupt::OnRxTransferDone);
            }
        }, _first_xdmac_interrupt};

        constexpr DMA_XDMAC_RX(const std::function<void(uint16_t)> &handler) : _xdmaInterruptHandler{handler} {};

        void resetRX() const
        {
            // init is called once after reset, so clean up after a reset
            SamCommon::enablePeripheralClock(peripheralId());

            // disable the channels
            disableRx();

            // Configure the Rx
            // ASSUMPTIONS:
            //  * Rx is from peripheral to memory
            //  * Not doing memory-to-memory or peripheral-to-peripheral (for now)
            //  * Single Block, Single Microblock transfers (for now)
            //  * All peripherals are using a FIFO for Rx and Tx
            //
            // If ANY of those assumptions are wrong, this code must change!!

            // Configure Rx channel
            xdmaRxChannel()->XDMAC_CSA = (uint32_t)xdmaPeripheralRxAddress();
            xdmaRxChannel()->XDMAC_CDA = 0;
            xdmaRxChannel()->XDMAC_CC =
            XDMAC_CC_TYPE_PER_TRAN | // between memory and a peripheral
            XDMAC_CC_MBSIZE_SINGLE | // burst size of one "unit" at a time
            XDMAC_CC_DSYNC_PER2MEM | // peripheral->memory
            XDMAC_CC_CSIZE_CHK_1   | // chunk size of one "unit" at a time
            XDMAC_CC_DWIDTH( (buffer_width >> 1) ) | // data width (based on alignment size of base type of buffer_t)
            XDMAC_CC_SIF_AHB_IF1   | // source is peripheral (info cryptically extracted from Table 18-3 of the datasheep)
            XDMAC_CC_DIF_AHB_IF0   | // destination is RAM   (info cryptically extracted from Table 18-3 of the datasheep)
            XDMAC_CC_SAM_FIXED_AM  | // the source address doesn't change (FIFO)
            XDMAC_CC_DAM_INCREMENTED_AM | // destination address increments as read
            XDMAC_CC_PERID(xdmaRxPeripheralId()) // and finally, set the peripheral identifier
            ;
            // Datasheep says to clear these out explicitly:
            //            xdmaRxChannel()->XDMAC_CNDC = 0; // no "next descriptor"
            //            xdmaRxChannel()->XDMAC_CBC = 0;  // ???
            //            xdmaRxChannel()->XDMAC_CDS_MSP = 0; // striding is disabled
            //            xdmaRxChannel()->XDMAC_CSUS = 0;
            //            xdmaRxChannel()->XDMAC_CDUS = 0;
            //            xdmaRxChannel()->XDMAC_CUBC = 0;

            // enable interrupts for these channels (must still be masked individually
            xdma()->XDMAC_GIE |= (1<<xdmaRxChannelNumber());
        };

        void disableRx() const
        {
            xdma()->XDMAC_GD = XDMAC_GID_ID0 << xdmaRxChannelNumber();
        };
        void enableRx() const
        {
            xdma()->XDMAC_GE = XDMAC_GIE_IE0 << xdmaRxChannelNumber();
        };
        void setRx(void * const buffer, const uint32_t length) const
        {
            xdmaRxChannel()->XDMAC_CDA = (uint32_t)buffer;
            xdmaRxChannel()->XDMAC_CUBC = length;
        };
        void setNextRx(void * const buffer, const uint32_t length) const
        {
            //            pdc()->PERIPH_RNPR = (uint32_t)buffer;
            //            pdc()->PERIPH_RNCR = length;
        };
        void flushRead() const
        {
            xdmaRxChannel()->XDMAC_CUBC = 0;
            SamCommon::sync();
        };
        uint32_t leftToRead(bool include_next = false) const
        {
            //            if (include_next) {
            //            }
            SamCommon::sync();
            return xdmaRxChannel()->XDMAC_CUBC;
        };
        uint32_t leftToReadNext() const
        {
            return 0;
            //            return pdc()->PERIPH_RNCR;
        };
        bool doneReading(bool include_next = false) const
        {
            return leftToRead(include_next) == 0;
        };
        bool doneReadingNext() const
        {
            return leftToReadNext() == 0;
        };
        buffer_t getRXTransferPosition() const
        {
            // we'll request a flush, but NOT wait for it
            xdma()->XDMAC_GSWF = (1<<xdmaRxChannelNumber());
            return (buffer_t)xdmaRxChannel()->XDMAC_CDA;
        };

        // Bundle it all up
        bool startRXTransfer(void * const buffer,
                             const uint32_t length,
                             bool handle_interrupts = true,
                             bool include_next = false
                             ) const
        {
            if (doneReading()) {
                disableRx();
                if (handle_interrupts) { stopRxDoneInterrupts(); }
                setRx(buffer, length);
            }
            // check to see if they overlap, in which case we're extending the region
            else if ((xdmaRxChannel()->XDMAC_CDA >= (uint32_t)buffer) &&
                     (xdmaRxChannel()->XDMAC_CDA < ((uint32_t)buffer + length))
                     )
            {
                // they overlap, we need to compute the new length
                disableRx(); // make it stand still first
                if (handle_interrupts) { stopRxDoneInterrupts(); }

                // new_length = (start_pos + length) - current_positon
                xdmaRxChannel()->XDMAC_CUBC = ((uint32_t)buffer + length) - xdmaRxChannel()->XDMAC_CDA;
            }
            // otherwise, we set the next region, if requested. We DON'T attempt to extend it.
            else if (include_next && doneReadingNext()) {
                setNextRx(buffer, length);
                return true;
            } else {
                length = 0; // we didn't add anything, this prevents turning things back on
            }

            if (length != 0) {
                if (handle_interrupts) { startRxDoneInterrupts(); }
                enableRx();
                return true;
            }

            return length > 0;

        };


        void startRxDoneInterrupts() const { xdmaRxChannel()->XDMAC_CIE = XDMAC_CIE_BIE; };
        void stopRxDoneInterrupts() const { xdmaRxChannel()->XDMAC_CID = XDMAC_CID_BID; };

        // XDMAC_Handler is handled with _tx_interrupt and _rx_interupt. They use
        // the std::function _xdmaInterruptHandler, which gets set by the peripheral
        // in the constexpr constructor.
        
        
        // These two get called from the peripheral interrupt handler,
        // and must always return false
        constexpr bool inRxBufferFullInterrupt() const { return false; };
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
        constexpr DMA_XDMAC_hardware() {};
        typedef char* buffer_t;

        // this is identical to in SamUART
        static constexpr Usart * const usart()
        {
            return Motate::usart(uartPeripheralNumber);
        };
    };

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<Usart*, uartPeripheralNumber> : virtual DMA_XDMAC_hardware<Usart*, uartPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Usart*, uartPeripheralNumber>::usart;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            switch (uartPeripheralNumber) {
                case (0): return  7;
                case (1): return  9;
                case (2): return 11;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaTxChannelNumber()
        {
            switch (uartPeripheralNumber) {
                case (0): return  0;
                case (1): return  2;
                case (2): return  4;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaTxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaTxChannelNumber();
        };
        static constexpr void * const xdmaPeripheralTxAddress()
        {
            return &usart()->US_THR;
        };
    };

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<Usart*, uartPeripheralNumber> : virtual DMA_XDMAC_hardware<Usart*, uartPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Usart*, uartPeripheralNumber>::usart;

        static constexpr uint8_t const xdmaRxPeripheralId()
        {
            switch (uartPeripheralNumber) {
                case (0): return  8;
                case (1): return 10;
                case (2): return 12;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaRxChannelNumber()
        {
            switch (uartPeripheralNumber) {
                case (0): return  1;
                case (1): return  3;
                case (2): return  5;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaRxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaRxChannelNumber();
        };
        static constexpr void * const xdmaPeripheralRxAddress()
        {
            return &usart()->US_RHR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Usart*, periph_num> : virtual DMA_XDMAC_RX<Usart*, periph_num>, virtual DMA_XDMAC_TX<Usart*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_XDMAC_RX<Usart*, periph_num>{handler}, DMA_XDMAC_TX<Usart*, periph_num>{handler} {};

        void setInterrupts(const uint16_t interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    DMA_XDMAC_TX<Uart*, periph_num>::startTxDoneInterrupts();
                } else {
                    DMA_XDMAC_TX<Uart*, periph_num>::stopTxDoneInterrupts();
                }

                if (interrupts & Interrupt::OnRxTransferDone) {
                    DMA_XDMAC_RX<Usart*, periph_num>::startRxDoneInterrupts();
                } else {
                    DMA_XDMAC_RX<Usart*, periph_num>::stopRxDoneInterrupts();
                }
            }
        };

        void reset() const {
            DMA_XDMAC_TX<Usart*, periph_num>::resetTX();
            DMA_XDMAC_RX<Usart*, periph_num>::resetRX();
        };
    };

#endif // USART + XDMAC

    // We're deducing if there's a UART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#ifdef HAS_UART0

#pragma mark DMA_XDMAC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>
    {
        static constexpr Uart * const uart() {
            return Motate::uart(uartPeripheralNumber);
        };

        typedef char* buffer_t;
    };

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<Uart*, uartPeripheralNumber> : virtual DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>::uart;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            switch (uartPeripheralNumber) {
                case (0): return 20;
                case (1): return 22;
                case (2): return 24;
                case (3): return 26;
                case (4): return 28;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaTxChannelNumber()
        {
            switch (uartPeripheralNumber) {
                case (0): return  6;
                case (1): return  8;
                case (2): return 10;
                case (3): return 12;
                case (4): return 14;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaTxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaTxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &uart()->UART_THR;
        };
    };

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<Uart*, uartPeripheralNumber> : virtual DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>::uart;

        static constexpr uint8_t const xdmaRxPeripheralId()
        {
            switch (uartPeripheralNumber) {
                case (0): return 21;
                case (1): return 23;
                case (2): return 25;
                case (3): return 27;
                case (4): return 29;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaRxChannelNumber()
        {
            switch (uartPeripheralNumber) {
                case (0): return  7;
                case (1): return  9;
                case (2): return 11;
                case (3): return 13;
                case (4): return 15;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaRxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaRxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralRxAddress()
        {
            return &uart()->UART_RHR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Uart*, periph_num> : DMA_XDMAC_RX<Uart*, periph_num>, DMA_XDMAC_TX<Uart*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_XDMAC_RX<Uart*, periph_num>{handler}, DMA_XDMAC_TX<Uart*, periph_num>{handler} {};

        void setInterrupts(const uint16_t interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    DMA_XDMAC_TX<Uart*, periph_num>::startTxDoneInterrupts();
                } else {
                    DMA_XDMAC_TX<Uart*, periph_num>::stopTxDoneInterrupts();
                }

                if (interrupts & Interrupt::OnRxTransferDone) {
                    DMA_XDMAC_RX<Uart*, periph_num>::startRxDoneInterrupts();
                } else {
                    DMA_XDMAC_RX<Uart*, periph_num>::stopRxDoneInterrupts();
                }
            }
        };

        void reset() const {
            DMA_XDMAC_TX<Uart*, periph_num>::resetTX();
            DMA_XDMAC_RX<Uart*, periph_num>::resetRX();
        };
    };
#endif // UART + XDMAC


    // We're deducing if there's a UART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#if defined(HAS_SPI0) && !defined(CAN_SPI_PDC_DMA)
#pragma mark DMA_XDMAC SPI implementation

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>
    {
        static constexpr Spi * const spi() {
            return Motate::spi(spiPeripheralNumber);
        };

        typedef char* buffer_t;
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<Spi*, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::spi;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 1;
                case (1): return 3;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaTxChannelNumber()
        {
            switch (spiPeripheralNumber) {
                case (0): return  18;
                case (1): return  20;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaTxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaTxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &spi()->SPI_TDR;
        };
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<Spi*, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::spi;

        static constexpr uint8_t const xdmaRxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 2;
                case (1): return 4;
            };
            return 0;
        };
        static constexpr uint8_t const xdmaRxChannelNumber()
        {
            switch (spiPeripheralNumber) {
                case (0): return 19;
                case (1): return 21;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaRxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaRxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralRxAddress()
        {
            return &spi()->SPI_RDR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Spi*, periph_num> : DMA_XDMAC_RX<Spi*, periph_num>, DMA_XDMAC_TX<Spi*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_XDMAC_RX<Spi*, periph_num>{handler}, DMA_XDMAC_TX<Spi*, periph_num>{handler} {};

        void setInterrupts(const uint16_t interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    DMA_XDMAC_TX<Spi*, periph_num>::startTxDoneInterrupts();
                } else {
                    DMA_XDMAC_TX<Spi*, periph_num>::stopTxDoneInterrupts();
                }

                if (interrupts & Interrupt::OnRxTransferDone) {
                    DMA_XDMAC_RX<Spi*, periph_num>::startRxDoneInterrupts();
                } else {
                    DMA_XDMAC_RX<Spi*, periph_num>::stopRxDoneInterrupts();
                }
            }
        };

        void reset() const {
            DMA_XDMAC_TX<Spi*, periph_num>::resetTX();
            DMA_XDMAC_RX<Spi*, periph_num>::resetRX();
        };
    };
#endif // SPI + XDMAC


    // We're deducing if there's a UART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#if defined(PWM1)
#pragma mark DMA_XDMAC PWM implementation

    template<uint8_t timerNum>
    struct DMA_XDMAC_hardware<Pwm*, timerNum>
    {
        static constexpr Pwm * const pwm() {
            if (timerNum < 8) { return PWM0; }
            else              { return PWM1; }
        };
        static constexpr PwmCh_num * const pwmChan()
        {
            if (timerNum < 8) { return PWM0->PWM_CH_NUM + timerNum; }
            else              { return PWM1->PWM_CH_NUM + (timerNum-8); }
        };

        typedef uint16_t* buffer_t;
    };

    template<uint8_t timerNum>
    struct DMA_XDMAC_TX_hardware<Pwm*, timerNum> : virtual DMA_XDMAC_hardware<Pwm*, timerNum>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Pwm*, timerNum>::pwm;
        
        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            if (timerNum < 8) { return 13; }
            else              { return 39; }
        };
        static constexpr uint8_t const xdmaTxChannelNumber()
        {
            switch (timerNum) {
                case (0): return  16;
                case (1): return  17;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaTxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaTxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &(pwm()->PWM_DMAR);
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Pwm*, periph_num> : DMA_XDMAC_TX<Pwm*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_XDMAC_TX<Pwm*, periph_num>{handler} {};

        void setInterrupts(const uint16_t interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    DMA_XDMAC_TX<Pwm*, periph_num>::startTxDoneInterrupts();
                } else {
                    DMA_XDMAC_TX<Pwm*, periph_num>::stopTxDoneInterrupts();
                }

//                if (interrupts & Interrupt::OnRxTransferDone) {
//                    startRxDoneInterrupts();
//                } else {
//                    stopRxDoneInterrupts();
//                }
            }
        };


        void reset() const {
            DMA_XDMAC_TX<Pwm*, periph_num>::resetTX();
        };

    };
#endif // PWM + XDMAC

} // namespace Motate

#endif // does not have XDMAC

#endif /* end of include guard: SAMDMA_H_ONCE */
