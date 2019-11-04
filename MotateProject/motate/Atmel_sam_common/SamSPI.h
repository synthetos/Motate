/*
 Atmel_sam_common/SamSPI.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2018  Robert Giseburt

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

#ifndef SAMSPI_H_ONCE
#define SAMSPI_H_ONCE

#include "sam.h"

#include "MotatePins.h"
#include "SamCommon.h"
#include <type_traits>

#include "SamSPIInternal.h"
#include "SamSPIDMA.h"

namespace Motate {
    template<int8_t spiPeripheralNumber>
    struct _SPIHardware : Motate::SPI_internal::SPIInfo<spiPeripheralNumber>
    {
        using this_type_t = _SPIHardware<spiPeripheralNumber>;
        using info = Motate::SPI_internal::SPIInfo<spiPeripheralNumber>;

        static_assert(info::exists,
                "Only _SPIHardware<0> or _SPIHardware<1> is valid on this processor.");

        using info::spi;
        using info::peripheralId;
        static constexpr auto spiIRQ = info::IRQ;
        static constexpr auto spiPeripheralNum = spiPeripheralNumber;

        std::function<void(Interrupt::Type)> _spiInterruptHandler;

        DMA<SPI_tag, spiPeripheralNumber> dma {_spiInterruptHandler};

        static std::function<void()> _spiInterruptHandlerJumper;
        _SPIHardware() {
            SamCommon::enablePeripheralClock(peripheralId);

            // Softare reset of SPI module
            spi->SPI_CR = SPI_CR_SWRST;
            disable();
        };

        void init() {
            // Set last transfer
            spi->SPI_CR = SPI_CR_LASTXFER;

            // Set Mode Register to Master mode
            spi->SPI_MR |= SPI_MR_MSTR;

            // Mode Fault Detection Disabled
            spi->SPI_MR |= SPI_MR_MODFDIS;

            // Ensure Fixed Peripheral Select
            spi->SPI_MR &= (~SPI_MR_PS);

            // Disable all interrupts
            spi->SPI_IDR = 0x7FF;

            // setup interrupt handlers BEFORE setting up DMA, in case it causes an interrupt
            _spiInterruptHandlerJumper = [&]() {
                if (_spiInterruptHandler) {
                    _spiInterruptHandler(getInterruptCause());
                } else {
#if IN_DEBUGGER == 1
                    __asm__("BKPT");
#endif
                }
            };

            dma.reset();
        };

        // This is to be called by the device, once it detects a "decoded" CS pin
        void setUsingCSDecoder(bool decoder) {
            if (decoder) {
                spi->SPI_MR |= SPI_MR_PCSDEC;
            } else {
                spi->SPI_MR &= ~SPI_MR_PCSDEC;
            }
        }

        void enable() {
            spi->SPI_CR = SPI_CR_SPIEN ;
        };

        void disable() {
            spi->SPI_CR = SPI_CR_SPIDIS;
        };

        void deassert() {
            spi->SPI_CR = SPI_CR_LASTXFER;
        }

        bool setChannel(const uint8_t channel, const bool deassert_after = false) {
            // if we are transmitting, we cannot switch
            while (!(spi->SPI_SR & SPI_SR_TXEMPTY)) {
                ;
            }

            auto csr_hold = spi->SPI_CSR[channel];
            csr_hold &= ~(SPI_CSR_CSAAT | SPI_CSR_CSNAAT);
            if (deassert_after) {
                csr_hold |= SPI_CSR_CSNAAT;
            } else {
                csr_hold |= SPI_CSR_CSAAT;
            }
            spi->SPI_CSR[channel] = csr_hold;

            spi->SPI_MR = (spi->SPI_MR & ~SPI_MR_PCS_Msk) | SPI_MR_PCS(channel);

            enable();
            return true;
        }

        void setChannelOptions(const uint8_t channel, const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns) {
            // We derive the baud from the master clock with a divider.
            // We want the closest match *below* the value asked for. It's safer to bee too slow.
            uint32_t new_otions = 0;

            uint32_t divider = SamCommon::getPeripheralClockFreq() / baud;
            if (divider > 255) {
                divider = 255;
            } else if (divider < 1) {
                divider = 1;
            }

            new_otions |= SPI_CSR_SCBR(divider);

            if (options & kSPIPolarityReversed) {
                new_otions |= SPI_CSR_CPOL;
            }

            // We can't really mix these ... they must already be compatible
            // But if we are using a debugger we can throw a wrench in and catch it
            if (!(options & kSPIClockPhaseReversed)) {
                new_otions |= SPI_CSR_NCPHA;
            }

            switch (options & kSPIBitsMask) {
                case kSPI9Bit:
                    new_otions |= SPI_CSR_BITS_9_BIT;
                    break;
                case kSPI10Bit:
                    new_otions |= SPI_CSR_BITS_10_BIT;
                    break;
                case kSPI11Bit:
                    new_otions |= SPI_CSR_BITS_11_BIT;
                    break;
                case kSPI12Bit:
                    new_otions |= SPI_CSR_BITS_12_BIT;
                    break;
                case kSPI13Bit:
                    new_otions |= SPI_CSR_BITS_13_BIT;
                    break;
                case kSPI14Bit:
                    new_otions |= SPI_CSR_BITS_14_BIT;
                    break;
                case kSPI15Bit:
                    new_otions |= SPI_CSR_BITS_15_BIT;
                    break;
                case kSPI16Bit:
                    new_otions |= SPI_CSR_BITS_16_BIT;
                    break;

                case kSPI8Bit:
                default:
                    new_otions |= SPI_CSR_BITS_8_BIT;
                    break;
            }

            // min_between_cs_delay_ns = DLYBCS
            // cs_to_sck_delay_ns = DLYBS
            // between_word_delay_ns = DLYBCT

            // these are in mupliples of MCLK (master clock, a.k.a. SystemCoreClock)
            // we want to round up, not down, so we divide by 100,000,000 instead of
            // 1,000,000,000, then add 5 and then further divide by 10.

            uint32_t dlybcs = (((min_between_cs_delay_ns*SamCommon::getPeripheralClockFreq())/100000000)+5)/10;

            if (dlybcs > 0xff) {
                // Break into the debugger
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // SPI dlybcs is too high!
#endif
            }

            // Always use the larger delay
            spi->SPI_MR = (spi->SPI_MR & ~SPI_MR_DLYBCS_Msk) | SPI_MR_DLYBCS(dlybcs);

            uint32_t dlybs = (((cs_to_sck_delay_ns*SamCommon::getPeripheralClockFreq())/100000000)+5)/10;
            if (dlybs > 0xff) {
                // Break into the debugger
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // SPI dlybs is too high!
#endif
            }
            new_otions |= SPI_CSR_DLYBS(dlybs);

            uint32_t dlybct = (((between_word_delay_ns*SamCommon::getPeripheralClockFreq())/100000000)+5)/10;
            if (dlybct > 0xff) {
                // Break into the debugger
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // SPI dlybct is too high!
#endif
            }
            new_otions |= SPI_CSR_DLYBCT(dlybct);

            // We'll drive CS low after we're done, so we always want this:
            new_otions |= SPI_CSR_CSAAT;

            spi->SPI_CSR[channel] = new_otions;
        };

        /* TEMPORARILY REMOVING DIRECT READ/WRITE/TRANSFER. Can be brought back later */
#if 0
        static int16_t read(const bool lastXfer = false, uint8_t toSendAsNoop = 0) {
            if (!(spi->SPI_SR & SPI_SR_RDRF)) {
                if (spi->SPI_SR & SPI_SR_TXEMPTY) {
                    spi->SPI_TDR = toSendAsNoop;
                    if (lastXfer) {
                        spi->SPI_CR = SPI_CR_LASTXFER;
                    }
                }
                return -1;
            }

            return spi->SPI_RDR;
        }

        static int16_t write(uint8_t value, const bool lastXfer = false) {
            int16_t throw_away;
            // Let's see what the optimizer does with this...
            return write(value, throw_away, lastXfer);
        };

        static int16_t write(uint8_t value, int16_t &readValue, const bool lastXfer = false) {
            while (!(spi->SPI_SR & SPI_SR_TDRE)) {;}

            if (spi->SPI_SR & SPI_SR_RDRF) {
                readValue = spi->SPI_RDR;
            } else {
                readValue = -1;
            }

            if (spi->SPI_SR & SPI_SR_TDRE) {

                spi->SPI_TDR = value;

                if (lastXfer) {
                    spi->SPI_CR = SPI_CR_LASTXFER;
                }

                return 1;
            }

            return -1;
        };

        int16_t transmit(uint8_t channel, const uint16_t data, const bool lastXfer = false) {
            uint32_t data_with_flags = data;

            if (lastXfer)
                data_with_flags |= SPI_TDR_LASTXFER;

            // NOTE: Assumes we DON'T have an external decoder/multiplexer!
            data_with_flags |= SPI_TDR_PCS(~(1<< channel));

            while (!(spi->SPI_SR & SPI_SR_TDRE))
                ;
            spi->SPI_TDR = data_with_flags;

            while (!(spi->SPI_SR & SPI_SR_RDRF))
                ;

            uint16_t outdata = spi->SPI_RDR;
            return outdata;
        };
#endif // temporarily removed read/write/transfer


        void setInterruptHandler(std::function<void(Interrupt::Type)> &&handler) {
            _spiInterruptHandler = std::move(handler);
        }

        Interrupt::Type getInterruptCause() {
            Interrupt::Type status = SPIInterrupt::Unknown;

            // Notes from experience:
            // This processor will sometimes allow one of these bits to be set,
            // even when there is no interrupt requested, and the setup conditions
            // don't appear to be done.
            // The simple but unfortunate fix is to verify that the Interrupt Mask
            // calls for that interrupt before considering it as a possible interrupt
            // source. This should be a best practice anyway, really. -Giseburt

            auto SPI_SR_hold = spi->SPI_SR;
            auto SPI_IMR_hold = spi->SPI_IMR;

            if ((SPI_IMR_hold & SPI_IMR_TDRE) && (SPI_SR_hold & SPI_SR_TDRE))
            {
                status |= SPIInterrupt::OnTxReady;
            }
            if ((SPI_IMR_hold & SPI_IMR_RDRF) && (SPI_SR_hold & SPI_SR_RDRF))
            {
                status |= SPIInterrupt::OnRxReady;
            }

            if (dma.inTxBufferEmptyInterrupt())
            {
                status |= SPIInterrupt::OnTxTransferDone;
            }
            if (dma.inRxBufferFullInterrupt())
            {
                status |= SPIInterrupt::OnRxTransferDone;
            }
            return status;
        }

        void setInterrupts(const Interrupt::Type interrupts) {
            if (interrupts != SPIInterrupt::Off) {

                if (interrupts & SPIInterrupt::OnTxReady) {
                    spi->SPI_IER = SPI_IER_TDRE;
                } else {
                    spi->SPI_IDR = SPI_IDR_TDRE;
                }
                if (interrupts & SPIInterrupt::OnRxReady) {
                    spi->SPI_IER = SPI_IER_RDRF;
                } else {
                    spi->SPI_IDR = SPI_IDR_RDRF;
                }

                if (interrupts & SPIInterrupt::OnRxTransferDone) {
                    dma.startRxDoneInterrupts();
                } else {
                    dma.stopRxDoneInterrupts();
                }
                if (interrupts & SPIInterrupt::OnTxTransferDone) {
                    dma.startTxDoneInterrupts();
                } else {
                    dma.stopTxDoneInterrupts();
                }

                /* Set interrupt priority */
                if (interrupts & SPIInterrupt::PriorityHighest) {
                    NVIC_SetPriority(spiIRQ, 0);
                }
                else if (interrupts & SPIInterrupt::PriorityHigh) {
                    NVIC_SetPriority(spiIRQ, 1);
                }
                else if (interrupts & SPIInterrupt::PriorityMedium) {
                    NVIC_SetPriority(spiIRQ, 2);
                }
                else if (interrupts & SPIInterrupt::PriorityLow) {
                    NVIC_SetPriority(spiIRQ, 3);
                }
                else if (interrupts & SPIInterrupt::PriorityLowest) {
                    NVIC_SetPriority(spiIRQ, 4);
                }

                NVIC_EnableIRQ(spiIRQ);
            } else {

                NVIC_DisableIRQ(spiIRQ);
            }
        };

        void _enableOnTXTransferDoneInterrupt() {
            dma.startTxDoneInterrupts();
        };

        void _disableOnTXTransferDoneInterrupt() {
            dma.stopTxDoneInterrupts();
        };

        void _enableOnRXTransferDoneInterrupt() {
            dma.startRxDoneInterrupts();
        };

        void _disableOnRXTransferDoneInterrupt() {
            dma.stopRxDoneInterrupts();
        };

        uint8_t getMessageSlotsAvailable() {
            uint8_t count = 0;
            if (dma.doneWriting() && dma.doneReading()) { count++; }
//            if (dma.doneWritingNext()) { count++; }
            return count;
        };

        bool doneWriting() {
            return dma.doneWriting();
        };
        bool doneReading() {
            return dma.doneReading();
        };

        // start transfer of message
        bool startTransfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size) {
            bool rx_is_setup = false;
            bool tx_is_setup = false;
            const bool handle_interrupts = false;
            const bool include_next = false;

            Motate::Interrupt::Type interrupts = 0;
            dma.setInterrupts(Interrupt::Off);
            if (rx_buffer != nullptr) {
                rx_is_setup = dma.startRXTransfer(rx_buffer, size, handle_interrupts, include_next);
                interrupts = Interrupt::OnTxTransferDone;
                if (!rx_is_setup) { return false; } // fail early
            } else {
                // Setup to transfer one dummy byte repeatedly
                dma.startRXTransfer(nullptr, size, handle_interrupts, include_next);
            }
            if (tx_buffer != nullptr) {
                tx_is_setup = dma.startTXTransfer(tx_buffer, size, handle_interrupts, include_next);
                interrupts |= Interrupt::OnRxTransferDone;
            } else {
                // Setup to transfer one dummy byte repeatedly
                dma.startTXTransfer(nullptr, size, handle_interrupts, include_next);
            }
            if (rx_is_setup || tx_is_setup) {
                dma.setInterrupts(Interrupt::OnRxTransferDone);
                dma.enable();
                enable();
#ifdef IN_DEBUGGER
            } else {
                __asm__("BKPT"); // no transfer setup
#endif
            }
            return rx_is_setup | tx_is_setup;
        }

        // abort transfer of message
        // TODO

        // get transfer status
        // TODO
    };

    template <pin_number csBit0PinNumber, pin_number csBit1PinNumber, pin_number csBit2PinNumber, pin_number csBit3PinNumber>
    struct SPIChipSelectPinMux {
        // These pins may be null, but if they're not, they must be valid CS pins
        static_assert(SPIChipSelectPin<csBit0PinNumber>::is_real || Pin<csBit0PinNumber>::isNull(),
                      "SPIChipSelectPinMux bit 0 pin is not on a real CS pin.");
        static_assert(SPIChipSelectPin<csBit1PinNumber>::is_real || Pin<csBit1PinNumber>::isNull(),
                      "SPIChipSelectPinMux bit 0 pin is not on a real CS pin.");
        static_assert(SPIChipSelectPin<csBit2PinNumber>::is_real || Pin<csBit2PinNumber>::isNull(),
                      "SPIChipSelectPinMux bit 0 pin is not on a real CS pin.");
        static_assert(SPIChipSelectPin<csBit3PinNumber>::is_real || Pin<csBit3PinNumber>::isNull(),
                      "SPIChipSelectPinMux bit 0 pin is not on a real CS pin.");

        struct _dummyCSPin { static constexpr uint8_t csNumber = 0; };

        template<pin_number n>
        using dummyOrCSPin = typename std::conditional<SPIChipSelectPin<n>::is_real, SPIChipSelectPin<n>, _dummyCSPin>::type;

        // create and initialize the CS pins we use
        dummyOrCSPin<csBit0PinNumber> bit0Pin;
        dummyOrCSPin<csBit1PinNumber> bit1Pin;
        dummyOrCSPin<csBit2PinNumber> bit2Pin;
        dummyOrCSPin<csBit3PinNumber> bit3Pin;

        typedef SPIChipSelectPinMux<csBit0PinNumber, csBit1PinNumber, csBit2PinNumber, csBit3PinNumber> type;

        constexpr uint8_t computeCsValue(uint8_t cs) {
            uint8_t csValue = 0;
            if (cs & (1<<0)) { csValue |= (1<<bit0Pin.csNumber); }
            if (cs & (1<<1)) { csValue |= (1<<bit1Pin.csNumber); }
            if (cs & (1<<2)) { csValue |= (1<<bit2Pin.csNumber); }
            if (cs & (1<<3)) { csValue |= (1<<bit3Pin.csNumber); }
            return csValue;
        };

        // Now provide a subobject that offers corect csNumber and csValue for each muxed output
        // We have three names that are confusing:

        // cs = the number we're going to call this one externally, using the order of the bits we provided
        //  IOW, cs of 3  is what you get when bit0Pin and bit1Pin are HIGH

        struct SPIChipSelect {
            // the internal value provided to the spi hardware (PCS for the Sam chips)
            const uint8_t csValue;
            // the internal cs number used by the hardware. All cs where csNumber is the same MUST share settings.
            const uint8_t csNumber;
            const bool usesDecoder = true;

            SPIChipSelect(SPIChipSelectPinMux * const pm, const uint8_t cs) : csValue{pm->computeCsValue(cs)}, csNumber {(uint8_t)(csValue >> 2)} {
            };

            // delete copy constructor
            SPIChipSelect(const SPIChipSelect &) = delete;

            // build move constructor
            SPIChipSelect(SPIChipSelect && other) : csValue{other.csValue}, csNumber{other.csNumber} {};
        };

        constexpr SPIChipSelect getCS(const uint8_t cs) { return {this, cs}; };
    };


    // SPIGetHardware is just a pass-through for now
    template <pin_number spiMISOPinNumber, pin_number spiMOSIPinNumber, pin_number spiSCKPinNumber>
    using SPIGetHardware = _SPIHardware<SPIMISOPin<spiMISOPinNumber>::spiNum>;
}

#endif /* end of include guard: SAMSPI_H_ONCE */
