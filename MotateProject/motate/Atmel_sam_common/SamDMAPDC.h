/*
 utility/SamDMAPDC.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2019 Robert Giseburt

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

// This file is included deep in another include file,
// all include dependencies should be handled by now.

#ifndef SAMDMAPDC_H_ONCE
#define SAMDMAPDC_H_ONCE

// Deduce if we have PDC in any of the variuos peripherals
#if defined(PDC_UART0) || defined(PDC_TWI0) || defined(PDC_SPI0)
    #define HAS_PDC
#endif

// PDC peripherals -- if we have a PDC (deduced using PERIPH_PTSR_RXTEN)
#ifdef HAS_PDC

namespace Motate {

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
        void setInterrupts(const Interrupt::Type interrupts) const {
            stopRxDoneInterrupts();
            stopTxDoneInterrupts();
        };

        void reset() const
        {
            pdc->PERIPH_PTCR = PERIPH_PTCR_RXTDIS | PERIPH_PTCR_TXTDIS; // disable all the things
            pdc->PERIPH_RPR = 0;
            pdc->PERIPH_RNPR = 0;
            pdc->PERIPH_RCR = 0;
            pdc->PERIPH_RNCR = 0;
            pdc->PERIPH_TPR = 0;
            pdc->PERIPH_TNPR = 0;
            pdc->PERIPH_TCR = 0;
            pdc->PERIPH_TNCR = 0;
        };

        void disableRx() const
        {
            pdc->PERIPH_PTCR = PERIPH_PTCR_RXTDIS; // disable for setup
        };
        void enableRx() const
        {
            pdc->PERIPH_PTCR = PERIPH_PTCR_RXTEN;  // enable
        };
        void setRx(void * const buffer, const uint32_t length) const
        {
            pdc->PERIPH_RPR = (uint32_t)buffer;
            pdc->PERIPH_RCR = length;
        };
        void setNextRx(void * const buffer, const uint32_t length) const
        {
            pdc->PERIPH_RNPR = (uint32_t)buffer;
            pdc->PERIPH_RNCR = length;
        };
        void flushRead() const {
            pdc->PERIPH_RNCR = 0;
            pdc->PERIPH_RCR = 0;
        };
        uint32_t leftToRead(bool include_next = false) const
        {
            if (pdc->PERIPH_RPR == 0) { return 0; }
            if (include_next) {
                return pdc->PERIPH_RCR + pdc->PERIPH_RNCR;
            }
            return pdc->PERIPH_RCR;
        };
        uint32_t leftToReadNext() const
        {
            if (pdc->PERIPH_RNPR == 0) { return 0; }
            return pdc->PERIPH_RNCR;
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
            return (buffer_t)pdc->PERIPH_RPR;
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

                setRx(buffer, length);

                enableRx();
                if (handle_interrupts) { startRxDoneInterrupts(); }
            }
            // check to see if they overlap, in which case we're extending the region
            else if ((pdc->PERIPH_RPR >= (uint32_t)buffer) &&
                     (pdc->PERIPH_RPR < ((uint32_t)buffer + length))
                    )
            {
                if (handle_interrupts) { stopRxDoneInterrupts(); }

                // they overlap, we need to compute the new length
                decltype(pdc->PERIPH_RPR) pos_save;
                do {
                    pos_save = pdc->PERIPH_RPR;

                    // new_length = (start_pos + length) - current_positon
                    pdc->PERIPH_RCR = ((uint32_t)buffer + length) - pos_save;

                    // catch rare case where it advances while we were computing
                } while (pdc->PERIPH_RPR > pos_save);

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
            pdc->PERIPH_PTCR = PERIPH_PTCR_TXTDIS; // disable for setup
        };
        void enableTx() const
        {
            pdc->PERIPH_PTCR = PERIPH_PTCR_TXTEN;  // enable again
        };
        void setTx(void * const buffer, const uint32_t length) const
        {
            pdc->PERIPH_TPR = (uint32_t)buffer;
            pdc->PERIPH_TCR = length;
        };
        void setNextTx(void * const buffer, const uint32_t length) const
        {
            pdc->PERIPH_TNPR = (uint32_t)buffer;
            pdc->PERIPH_TNCR = length;
        };
        uint32_t leftToWrite(bool include_next = false) const
        {
            if (include_next) {
                return pdc->PERIPH_TCR + pdc->PERIPH_TNCR;
            }
            return pdc->PERIPH_TCR;
        };
        uint32_t leftToWriteNext() const
        {
            return pdc->PERIPH_TNCR;
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
            return (buffer_t)pdc->PERIPH_TPR;
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

} // end namespace Motate

#endif // if has PDC

#endif /* end of include guard: SAMDMAPDC_H_ONCE */
