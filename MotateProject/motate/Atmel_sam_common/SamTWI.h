/*
 Atmel_sam_common/SamTWI.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2018 Robert Giseburt

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

#ifndef SAMTWI_H_ONCE
#define SAMTWI_H_ONCE

#include "sam.h"

#include "MotatePins.h"
#include "SamCommon.h"
#include <type_traits>

#include "SamTWIInternal.h"
#include "SamTWIDMA.h"

//NOTE: Currently only supporting master mode!!

namespace Motate {

    template<int8_t twiPeripheralNumber>
    struct _TWIHardware : public Motate::TWI_internal::TWIInfo<twiPeripheralNumber>
    {
        using this_type_t = _TWIHardware<twiPeripheralNumber>;
        using info = Motate::TWI_internal::TWIInfo<twiPeripheralNumber>;
        static constexpr auto twiPeripheralNum = twiPeripheralNumber;

        static_assert(info::exists,
                "Using an unsupported TWI peripheral for this processor.");

        using info::twi;
        using info::peripheralId;
        using info::IRQ;

        std::function<void(const TWIInterruptCause&)> _TWIInterruptHandler;

        std::function<void(Interrupt::Type)> _TWIDMAInterruptHandler;
        static std::function<void(void)> _twiInterruptHandlerJumper;

        DMA<TWI_tag, twiPeripheralNumber> dma{_TWIDMAInterruptHandler};

        _TWIHardware() : _TWIDMAInterruptHandler{[&](Interrupt::Type hint){ this->handleInterrupts(hint); }} {
            _twiInterruptHandlerJumper = [&]() { this->handleInterrupts(); };
            SamCommon::enablePeripheralClock(peripheralId);

            // read the status register (Microchip says so)
            twi->TWIHS_SR;

            // Softare reset of TWI module
            twi->TWIHS_CR = TWIHS_CR_SWRST;

            disable();
        }

        void init() {
            dma.reset();

            // enable();

            setSpeed(); // use default fast I2C

            // always have IRQs enabled for TWI, lowest priority
            setInterrupts(TWIInterrupt::PriorityLowest);
        }

        void handleInterrupts(Interrupt::Type hint = 0) {
            auto interruptCause = getInterruptCause(hint);

            // note that interruptCause may change in prehandleInterrupt!
            prehandleInterrupt(interruptCause);
            // if we ended up handling them, we're done here
            if (interruptCause.isEmpty()) { return; }

            if (_TWIInterruptHandler) {
                _TWIInterruptHandler(interruptCause);
            } else {
                #if IN_DEBUGGER == 1
                __asm__("BKPT");
                #endif
            }
        }

        void enable() {
            twi->TWIHS_CR = TWIHS_CR_MSEN;
        }

        void disable() {
            twi->TWIHS_CR = TWIHS_CR_SVDIS;
            twi->TWIHS_CR = TWIHS_CR_MSDIS;
        }

        bool setAddress(const TWIAddress& address, const TWIInternalAddress& internal_address) {
            // check for transmitting?

            uint8_t  adjusted_address = 0;
            uint32_t adjusted_internal_address = 0;
            uint8_t  adjusted_internal_address_size = 0;

            if (TWIDeviceAddressSize::k10Bit == address.size) {
                if (internal_address.size > TWIInternalAddressSize::k2Bytes) {
                    return false; // we only support 3 total bytes of internal address
                }
                // top two bits (xx) of the 10-bit address ORd with special code 0b011110xx go into the device address
                adjusted_address = 0b01111000 | ((address.address >> 8) & 0b11);
                adjusted_internal_address = (address.address & 0xFF) | ((internal_address.address & 0xFFFF) << 8);
                adjusted_internal_address_size = (uint8_t)internal_address.size + 1;
            } else {
                // 7 bit address
                adjusted_address = 0x7F & address.address;
                adjusted_internal_address = (internal_address.address & 0xFFFFFF);
                adjusted_internal_address_size = (uint8_t)internal_address.size;
            }

            twi->TWIHS_MMR = TWIHS_MMR_DADR(adjusted_address) | TWIHS_MMR_IADRSZ(adjusted_internal_address_size);
            twi->TWIHS_IADR = TWIHS_IADR_IADR(adjusted_internal_address);

            enable();
            return true;
        }

        /* Low level time limit of I2C Fast Mode. */
        static constexpr uint32_t LOW_LEVEL_TIME_LIMIT   = 384000;
        static constexpr uint32_t I2C_FAST_MODE_SPEED    = 400000;
        static constexpr uint32_t TWIHS_CLK_DIVIDER      = 2;
        static constexpr uint32_t TWIHS_CLK_CALC_ARGU    = 3;
        static constexpr uint32_t TWIHS_CLK_DIV_MAX      = 0xFF;
        static constexpr uint32_t TWIHS_CLK_DIV_MIN      = 7;

        void setSpeed(const uint32_t speed = I2C_FAST_MODE_SPEED) {
            uint32_t ckdiv = 0;
            uint32_t c_lh_div;
            uint32_t cldiv, chdiv;
            auto periph_clock = SamCommon::getPeripheralClockFreq();

            /* High-Speed can be only used in slave mode, 400k is the max speed allowed for master */
            if (speed > I2C_FAST_MODE_SPEED) {
                return;//FAIL;
            }

            /* Low level time not less than 1.3us of I2C Fast Mode. */
            if (speed > LOW_LEVEL_TIME_LIMIT) {
                /* Low level of time fixed for 1.3us. */
                cldiv = periph_clock / (LOW_LEVEL_TIME_LIMIT * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;
                chdiv = periph_clock / ((speed + (speed - LOW_LEVEL_TIME_LIMIT)) * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;

                /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
                while ((cldiv > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                    /* Increase clock divider */
                    ckdiv++;
                    /* Divide cldiv value */
                    cldiv /= TWIHS_CLK_DIVIDER;
                }
                /* chdiv must fit in 8 bits, ckdiv must fit in 3 bits */
                while ((chdiv > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                    /* Increase clock divider */
                    ckdiv++;
                    /* Divide cldiv value */
                    chdiv /= TWIHS_CLK_DIVIDER;
                }

                /* set clock waveform generator register */
                twi->TWIHS_CWGR =
                        TWIHS_CWGR_CLDIV(cldiv) | TWIHS_CWGR_CHDIV(chdiv) |
                        TWIHS_CWGR_CKDIV(ckdiv);
            } else {
                c_lh_div = periph_clock / (speed * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;

                /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
                while ((c_lh_div > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                    /* Increase clock divider */
                    ckdiv++;
                    /* Divide cldiv value */
                    c_lh_div /= TWIHS_CLK_DIVIDER;
                }

                /* set clock waveform generator register */
                twi->TWIHS_CWGR =
                        TWIHS_CWGR_CLDIV(c_lh_div) | TWIHS_CWGR_CHDIV(c_lh_div) |
                        TWIHS_CWGR_CKDIV(ckdiv);
            }
        }

        void setInterruptHandler(std::function<void(const TWIInterruptCause&)> &&handler) {
            _TWIInterruptHandler = std::move(handler);
        }

        TWIInterruptCause getInterruptCause(Interrupt::Type hint = 0) {
            TWIInterruptCause status;

            // Notes from experience:
            // This processor will sometimes allow one of these bits to be set,
            // even when there is no interrupt requested, and the setup conditions
            // don't appear to be done.
            // The simple but unfortunate fix is to verify that the Interrupt Mask
            // calls for that interrupt before considering it as a possible interrupt
            // source. This should be a best practice anyway, really. -Giseburt

            auto TWI_SR_hold = twi->TWIHS_SR;
            auto TWI_IMR_hold = twi->TWIHS_IMR;

            if (TWI_SR_hold & TWIHS_SR_TXRDY)
            {
                status.setTxReady();
                if ((TWI_IMR_hold & TWIHS_IMR_TXCOMP) && (TWI_SR_hold & TWIHS_SR_TXCOMP))
                {
                    status.setTxDone();
                }
            }
            if (TWI_SR_hold & TWIHS_SR_RXRDY)
            {
                status.setRxReady();
            }
            if (TWI_SR_hold & TWIHS_SR_NACK)
            {
                status.setNACK();
            }


            if ((hint & Interrupt::OnTxTransferDone))
            {
                status.setTxTransferDone();
            }
            if ((hint & Interrupt::OnRxTransferDone))
            {
                status.setRxTransferDone();
            }
            if ((hint & Interrupt::OnTxError))
            {
                status.setTxError();
            }
            if ((hint & Interrupt::OnRxError))
            {
                status.setRxError();
            }


            return status;
        }

        void enableOnTXReadyInterrupt() {
            twi->TWIHS_IER = TWIHS_IER_TXRDY;
        }
        void disableOnTXReadyInterrupt() {
            twi->TWIHS_IDR = TWIHS_IDR_TXRDY;
        }

        void enableOnNACKInterrupt() {
            twi->TWIHS_IER = TWIHS_IER_NACK;
        }
        void disableOnNACKInterrupt() {
            twi->TWIHS_IDR = TWIHS_IDR_NACK;
        }


        void enableOnTXDoneInterrupt() {
            twi->TWIHS_IER = TWIHS_IER_TXCOMP;
        }
        void disableOnTXDoneInterrupt() {
            twi->TWIHS_IDR = TWIHS_IDR_TXCOMP;
        }

        void enableOnRXReadyInterrupt() {
            twi->TWIHS_IER = TWIHS_IER_RXRDY;
        }
        void disableOnRXReadyInterrupt() {
            twi->TWIHS_IDR = TWIHS_IDR_RXRDY;
        }


        void setInterrupts(const Interrupt::Type interrupts) {
            if (interrupts != TWIInterrupt::Off) {

                if (interrupts & TWIInterrupt::OnTxReady) {
                    enableOnTXReadyInterrupt();
                } else {
                    disableOnTXReadyInterrupt();
                }
                if (interrupts & TWIInterrupt::OnTxDone) {
                    enableOnTXDoneInterrupt();
                } else {
                    disableOnTXDoneInterrupt();
                }

                if (interrupts & TWIInterrupt::OnNACK) {
                    enableOnNACKInterrupt();
                } else {
                    disableOnNACKInterrupt();
                }

                if (interrupts & TWIInterrupt::OnRxReady) {
                    enableOnRXReadyInterrupt();
                } else {
                    disableOnRXReadyInterrupt();
                }

                if (interrupts & TWIInterrupt::OnRxTransferDone) {
                    dma.startRxDoneInterrupts();
                } else {
                    dma.stopRxDoneInterrupts();
                }
                if (interrupts & TWIInterrupt::OnTxTransferDone) {
                    dma.startTxDoneInterrupts();
                } else {
                    dma.stopTxDoneInterrupts();
                }

                /* Set interrupt priority */
                if (interrupts & TWIInterrupt::PriorityHighest) {
                    NVIC_SetPriority(IRQ, 0);
                }
                else if (interrupts & TWIInterrupt::PriorityHigh) {
                    NVIC_SetPriority(IRQ, 1);
                }
                else if (interrupts & TWIInterrupt::PriorityMedium) {
                    NVIC_SetPriority(IRQ, 2);
                }
                else if (interrupts & TWIInterrupt::PriorityLow) {
                    NVIC_SetPriority(IRQ, 3);
                }
                else if (interrupts & TWIInterrupt::PriorityLowest) {
                    NVIC_SetPriority(IRQ, 4);
                }

                // Always have IRQs enabled for TWI
                NVIC_EnableIRQ(IRQ);
            } else {
                // Always have IRQs enabled for TWI
                //NVIC_DisableIRQ(IRQ);
            }
        }

        // void _enableOnTXTransferDoneInterrupt() {
        //     dma.startTxDoneInterrupts();
        // }

        // void _disableOnTXTransferDoneInterrupt() {
        //     dma.stopTxDoneInterrupts();
        // }

        // void _enableOnRXTransferDoneInterrupt() {
        //     dma.startRxDoneInterrupts();
        // }

        // void _disableOnRXTransferDoneInterrupt() {
        //     dma.stopRxDoneInterrupts();
        // }

        uint8_t getMessageSlotsAvailable() {
            uint8_t count = 0;
            if (dma.doneWriting() && dma.doneReading()) { count++; }
            // if (dma.doneWritingNext()) { count++; }
            return count;
        }

        bool doneWriting() {
            return dma.doneWriting();
        }
        bool doneReading() {
            return dma.doneReading();
        }

        enum class InternalState {
            Idle,

            TXDMAStarted,
            TXWaitingForTXReady1,
            TXWaitingForTXReady2,
            TXError,

            RXDMAStarted,
            RXWaitingForRXReady,
            RXWaitingForLastChar,
            RXError,
        } state_ = InternalState::Idle;

        uint8_t *local_buffer_ptr = nullptr;

        // start transfer of message
        bool startTransfer(uint8_t *buffer, const uint16_t size, const bool is_rx) {
            if ((buffer == nullptr) || (state_ != InternalState::Idle) || (size == 0)) { return false; }
            local_buffer_ptr = nullptr;

            bool dma_is_setup = false;
            dma.setInterrupts(Interrupt::Off);
            if (is_rx) {
                // tell the peripheral that we're reading
                twi->TWIHS_MMR |= TWIHS_MMR_MREAD;

                if (size > 2) {
                    const bool handle_interrupts = true;
                    const bool include_next = false;

                    local_buffer_ptr = buffer + (size-2);
                    state_ = InternalState::RXDMAStarted;

                    // Note we set size to size-2 since we have to handle the last two characters "manually"
                    // This happens in prehandleInterrupt()
                    dma_is_setup = dma.startRXTransfer(buffer, size-2, handle_interrupts, include_next);
                    if (!dma_is_setup) { return false; } // fail early
                }
                else {
                    local_buffer_ptr = buffer;
                    state_ = (size == 2) ? InternalState::RXWaitingForRXReady : InternalState::RXWaitingForLastChar;
                    if (size == 1) {
                        // if there is only one character to read, set the flag to send the STOP ater we get it
                        twi->TWIHS_CR = TWIHS_CR_STOP;
                    }
                    enableOnRXReadyInterrupt();
                }
                // twi->TWIHS_CR = TWIHS_CR_START;
            }
            else {
                // tell the peripheral that we're writing
                // twi->TWIHS_MMR &= ~TWIHS_MMR_MREAD;

                // TX
                if (size > 1) {
                    const bool handle_interrupts = true;
                    const bool include_next = false;

                    local_buffer_ptr = buffer + (size-1);
                    state_ = InternalState::TXDMAStarted;

                    enableOnTXDoneInterrupt();
                    enableOnNACKInterrupt();

                    // Note we set size to size-1 since we have to handle the last character "manually"
                    // This happens in prehandleInterrupt()
                    dma_is_setup = dma.startTXTransfer(buffer, size-1, handle_interrupts, include_next);
                    if (!dma_is_setup) { return false; } // fail early
                }
                else {
                    local_buffer_ptr = buffer;
                    state_ = InternalState::TXWaitingForTXReady1;
                    enableOnTXReadyInterrupt();
                }
                // twi->TWIHS_CR = TWIHS_CR_START;
            }

            // enable();

            return true;
        }


        void prehandleInterrupt(TWIInterruptCause &cause) {
            if (cause.isRxError()) {
                state_ = InternalState::RXError;
            }

            if (cause.isTxError()) {
                state_ = InternalState::TXError;
            }

            // NACK cases
            if (cause.isNACK() & (state_ == InternalState::RXError || state_ == InternalState::TXError)) {
                disableOnRXReadyInterrupt();
                disableOnTXReadyInterrupt();

                disableOnNACKInterrupt();

                // Problem: How to tell how much, if anything, was transmitted?
                state_ = InternalState::Idle;

                dma.disable();
                dma.stopRxDoneInterrupts();
                dma.stopTxDoneInterrupts();

                return;
            }

            // RX cases
            if (InternalState::RXDMAStarted == state_ && cause.isRxTransferDone()) {
                cause.clearRxTransferDone(); // stop this from being propagated
                // At this point we've recieved all but the last two characters
                // Now we wait for the next-to-last character to come in, read it into the buffer, and set the STOP bit
                state_ = InternalState::RXWaitingForRXReady;

                // If we don't already have an RXReady, set the interrupt for it
                if (!cause.isRxReady()) { enableOnRXReadyInterrupt(); }
            } // no else here!

            if (InternalState::RXWaitingForRXReady == state_ && cause.isRxReady()) {
                cause.clearRxReady(); // stop this from being propagated
                // At this point we've recieved the next-to-last character, but haven't read it yet
                // Now we set the STOP bit, ...
                twi->TWIHS_CR = TWIHS_CR_STOP;

                // ... prepare to wait for the last character
                state_ = InternalState::RXWaitingForLastChar;
                enableOnRXReadyInterrupt();

                /// ... then finally read the next-to-last char.
                *local_buffer_ptr = (uint8_t)twi->TWIHS_RHR;
                ++local_buffer_ptr;

                return; // there's nothing more we can do here, save time
            }

            if (InternalState::RXWaitingForLastChar == state_ && cause.isRxReady()) {
                // we want isRxReady to be propagated
                cause.setRxTransferDone(); // And now we push that the rx transfer is done

                // At this point we've recieved last character, but haven't read it yet
                *local_buffer_ptr = (uint8_t)twi->TWIHS_RHR;
                local_buffer_ptr = nullptr;

                // Finsih the state machine, and stop the interrupts
                state_ = InternalState::Idle;
                disableOnRXReadyInterrupt();
                disableOnNACKInterrupt();

                return; // there's nothing more we can do here, save time
            }


            // TX cases
            if (InternalState::TXDMAStarted == state_ && cause.isTxDone()) {
                cause.clearTxDone(); // stop this from being propagated
            }
            if (InternalState::TXDMAStarted == state_ && cause.isTxTransferDone()) {
                cause.clearTxTransferDone(); // stop this from being propagated
                // At this point DMA has sent all but the last character to the TWI
                // Now we wait for the TWI hardware to indicate it's ready for another character
                // which may have already happened
                state_ = InternalState::TXWaitingForTXReady1;
                dma.stopTxDoneInterrupts();
                dma.disable();

                // If we don't already have an TXReady, set the interrupt for it
                if (!cause.isTxReady()) { enableOnTXReadyInterrupt(); }
            } // no else here!

            if (InternalState::TXWaitingForTXReady1 == state_ && cause.isTxReady()) {
                cause.clearTxReady(); // stop this from being propagated

                // At this point we've sent the next-to-last character,
                // now put the last char in the hold register, ...
                twi->TWIHS_THR = *local_buffer_ptr;
                local_buffer_ptr = nullptr;

                // ... and set the STOP bit, ...
                twi->TWIHS_CR = TWIHS_CR_STOP;

                // ... and move the state machine along.
                state_ = InternalState::TXWaitingForTXReady2;
                enableOnTXReadyInterrupt();

                return; // there's nothing more we can do here, save time
            }

            if (InternalState::TXWaitingForTXReady2 == state_ && cause.isTxReady()) {
                // we want isTxReady to be propagated
                cause.setTxTransferDone(); // And now we push that the tx transfer is done

                // Finish up the state machine.
                state_ = InternalState::Idle;
                disableOnTXReadyInterrupt();
                disableOnNACKInterrupt();
            }
        }

        // abort transfer of message
        // TODO

        // get transfer status
        // TODO
    };

    // TWIGetHardware is just a pass-through for now
    template <pin_number twiSCKPinNumber, pin_number twiSDAPinNumber>
    using TWIGetHardware = _TWIHardware<TWISCKPin<twiSCKPinNumber>::twiNum>;

} // namespace Motate

#endif /* end of include guard: SAMTWI_H_ONCE */
