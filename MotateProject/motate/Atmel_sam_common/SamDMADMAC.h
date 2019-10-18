/*
 utility/SamDMADMAC.h - Library for the Motate system
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

#ifndef SAMDMADMAC_H_ONCE
#define SAMDMADMAC_H_ONCE

// Only if we have an DMAC
#if defined(DMAC)

namespace Motate {

// DMA_DMAC_hardware template - - MUST be specialized
template <typename periph_t, uint8_t periph_num>
struct DMA_DMAC_hardware {
    DMA_DMAC_hardware() = delete;
};
template <typename periph_t, uint8_t periph_num>
struct DMA_DMAC_RX_hardware {
    DMA_DMAC_RX_hardware() = delete;
};
template <typename periph_t, uint8_t periph_num>
struct DMA_DMAC_TX_hardware {
    DMA_DMAC_TX_hardware() = delete;
};

struct DMA_DMAC_common {
    static constexpr uint32_t  peripheralId{ID_DMAC};
    static Dmac* const         dmac() { return DMAC; };
    static constexpr IRQn_Type dmacIRQ() { return DMAC_IRQn; };

    void setInterrupts(const Interrupt::Type interrupts) const {
        // Once it's known that interrupts are required, always have them on
        NVIC_EnableIRQ(dmacIRQ());

        if (interrupts != Interrupt::Off) {
            // Subobjects must handle Interrupt::OnTxTransferDone and Interrupt::OnRxTransferDone

            /* Set interrupt priority */
            if (interrupts & Interrupt::PriorityHighest) {
                NVIC_SetPriority(dmacIRQ(), 0);
            } else if (interrupts & Interrupt::PriorityHigh) {
                NVIC_SetPriority(dmacIRQ(), 1);
            } else if (interrupts & Interrupt::PriorityMedium) {
                NVIC_SetPriority(dmacIRQ(), 2);
            } else if (interrupts & Interrupt::PriorityLow) {
                NVIC_SetPriority(dmacIRQ(), 3);
            } else if (interrupts & Interrupt::PriorityLowest) {
                NVIC_SetPriority(dmacIRQ(), 4);
            }
        }
    };
};

struct _DMACInterrupt {
    const std::function<void(void)> interrupt_handler;
    uint8_t                         channel_num;
    uint32_t                        channel_mask;
    _DMACInterrupt*                 next;

    _DMACInterrupt(const _DMACInterrupt&) = delete;             // delete the copy constructor, we only allow moves
    _DMACInterrupt& operator=(const _DMACInterrupt&) = delete;  // delete the assigment operator, we only allow moves

    // Note we MOVE construct this interrupt function...
    _DMACInterrupt(const std::function<void(void)>&& _interrupt, _DMACInterrupt*& _first)
        : interrupt_handler{std::move(_interrupt)}, next{nullptr} {
        if (interrupt_handler) {  // std::function returns false if the function isn't valid
            if (_first == nullptr) {
                _first       = this;
                channel_num  = 0;
                channel_mask = (uint32_t)((DMAC_EBCISR_BTC0 | DMAC_EBCISR_CBTC0 | DMAC_EBCISR_ERR0) << channel_num);
                return;
            }

            _DMACInterrupt* i = _first;
            while (i->next != nullptr) {
                i           = i->next;
                channel_num = std::max(channel_num, i->channel_num);
            }
            ++channel_num;
            i->next      = this;
            channel_mask = (uint32_t)(1 << channel_num);
        }
    };

    uint8_t getChannel() const { return channel_num; }
};

extern _DMACInterrupt* _first_dmac_interrupt;

// NOTE, we have 23 channels, and less than 23 peripheral types using this,
// so we'll assign channels uniquely, but otherwise arbitrarily from lowest
// to highest. If using DMAC directly, beware and use the highest channels
// first.

template <typename periph_t, uint8_t periph_num>
struct DMA_DMAC_TX : virtual DMA_DMAC_TX_hardware<periph_t, periph_num> {
    typedef DMA_DMAC_TX_hardware<periph_t, periph_num> _hw;

    using _hw::dmac;
    using _hw::dmacTxPeripheralId;
    // using _hw::dmacTxChannelNumber;
    // using _hw::dmacTxChannel;
    using _hw::dmacIRQ;
    using _hw::dmacPeripheralTxAddress;
    using _hw::peripheralId;

    typedef typename _hw::buffer_t buffer_t;
    static constexpr uint32_t buffer_width = std::alignment_of<typename std::remove_pointer<buffer_t>::type>::value;


    const std::function<void(Interrupt::Type)>& _dmaCInterruptHandler;

    _DMACInterrupt _tx_interrupt{
        [&]() {
            Interrupt::Type cause;
            if (_dmaCInterruptHandler) {
                auto status = dmac()->DMAC_EBCISR;
                if (status & ((DMAC_EBCISR_BTC0 | DMAC_EBCISR_CBTC0) << _tx_interrupt.getChannel())) {
                    cause = Interrupt::OnTxTransferDone;
                }
                if (status & (DMAC_EBCISR_ERR0 << _tx_interrupt.getChannel())) {
                    cause |= Interrupt::OnTxError;
                }
                _dmaCInterruptHandler(cause);
            }
        },
        _first_dmac_interrupt};

    const uint8_t     dmacTxChannelNumber() const { return _tx_interrupt.getChannel(); }
    DmacCh_num* const dmacTxChannel() const { return dmac()->DMAC_CH_NUM + dmacTxChannelNumber(); };

    // we'll hold a reference to the handler, the peripheral owns the one it's passing
    constexpr DMA_DMAC_TX(const std::function<void(Interrupt::Type)>& handler) : _dmaCInterruptHandler{handler} {};

    void resetTX() const {
        // init is called once after reset, so clean up after a reset
        SamCommon::enablePeripheralClock(peripheralId);

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
        dmacTxChannel()->DMAC_DSCR  = 0UL;
        dmacTxChannel()->DMAC_SADDR = 0UL;
        dmacTxChannel()->DMAC_DADDR = (uint32_t)dmacPeripheralTxAddress();
        dmacTxChannel()->DMAC_CTRLA = DMAC_CTRLA_BTSIZE(0);                // set later in setTx(...)
        dmacTxChannel()->DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR_FETCH_DISABLE |  //
                                      DMAC_CTRLB_DST_DSCR_FETCH_DISABLE |  //
                                      DMAC_CTRLB_FC_MEM2PER_DMA_FC |       // memory->peripheral
                                      DMAC_CTRLB_SRC_INCR_INCREMENTING |   // increment source address after each fetch
                                      DMAC_CTRLB_DST_INCR_FIXED |          // destination address is fixed
                                      // leave DMAC_CTRLB_IEN as 0 - we want an interrupt
                                      0;  // for layout purposes

        dmacTxChannel()->DMAC_CFG = DMAC_CFG_DST_PER(dmacTxPeripheralId()) | DMAC_CFG_DST_H2SEL_HW;

        // enable interrupts for this channel
        dmac()->DMAC_EBCIER |= ((DMAC_EBCIER_BTC0 | DMAC_EBCIER_CBTC0 | DMAC_EBCIER_ERR0) << dmacTxChannelNumber());
    };


    void disableTx() const { dmac()->DMAC_CHDR = DMAC_CHDR_DIS0 << dmacTxChannelNumber(); };
    void enableTx() const { dmac()->DMAC_CHER = DMAC_CHER_ENA0 << dmacTxChannelNumber(); };
    void setTx(void* const buffer, const uint32_t length) const {
        dmacTxChannel()->DMAC_SADDR = (uint32_t)buffer;
        dmacTxChannel()->DMAC_CTRLA = DMAC_CTRLA_BTSIZE(length) | DMAC_CTRLA_SCSIZE_CHK_1 | DMAC_CTRLA_DCSIZE_CHK_1 |
                                      DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE |
                                      // DON'T set DMAC_CTRLA_DONE
                                      0;
    };
    void setNextTx(void* const buffer, const uint32_t length) const {
        // ...
    };
    uint32_t leftToWrite(bool include_next = false) const {
        //            if (include_next) {
        //            }
        SamCommon::sync();
        return (dmacTxChannel()->DMAC_CTRLA & DMAC_CTRLA_BTSIZE_Msk) << DMAC_CTRLA_BTSIZE_Pos;
    };
    uint32_t leftToWriteNext() const {
        return 0;
        //            return pdc->PERIPH_TNCR;
    };
    bool     doneWriting(bool include_next = false) const { return leftToWrite(include_next) == 0; };
    bool     doneWritingNext() const { return leftToWriteNext() == 0; };
    buffer_t getTXTransferPosition() const { return (buffer_t)dmacTxChannel()->DMAC_SADDR; };


    // Bundle it all up
    bool startTXTransfer(void* const    buffer,
                         const uint32_t length,
                         bool           handle_interrupts = true,
                         bool           include_next      = false) const {
        if (doneWriting()) {
            disableTx();
            if (handle_interrupts) {
                stopTxDoneInterrupts();
            }
            setTx(buffer, length);
            if (length != 0) {
                if (handle_interrupts) {
                    startTxDoneInterrupts();
                }
                enableTx();
                return true;
            }
            return false;
        }
        return false;
    };


    void startTxDoneInterrupts() const {
        dmac()->DMAC_EBCIER |= ((DMAC_EBCIER_BTC0 | DMAC_EBCIER_CBTC0 | DMAC_EBCIER_ERR0) << dmacTxChannelNumber());
    };
    void stopTxDoneInterrupts() const {
        dmac()->DMAC_EBCIDR |= ((DMAC_EBCIDR_BTC0 | DMAC_EBCIDR_CBTC0 | DMAC_EBCIDR_ERR0) << dmacTxChannelNumber());
    };

    // DMAC_Handler is handled with _tx_interrupt and _rx_interupt. They use
    // the std::function _dmaCInterruptHandler, which get's set by the peripheral
    // in the constexpr constructor.

    // These two get called from the peripheral interrupt handler,
    // and must always return false
    constexpr bool inTxBufferEmptyInterrupt() const { return false; };
};

// DMA_DMAC is split into two halves, TX and RX. Each or both can be inherited from.
template <typename periph_t, uint8_t periph_num>
struct DMA_DMAC_RX : virtual DMA_DMAC_RX_hardware<periph_t, periph_num> {
    typedef DMA_DMAC_RX_hardware<periph_t, periph_num> _hw;

    using _hw::dmac;
    using _hw::dmacIRQ;
    using _hw::dmacPeripheralRxAddress;
    using _hw::dmacRxPeripheralId;
    using _hw::peripheralId;

    typedef typename _hw::buffer_t buffer_t;
    static constexpr uint32_t buffer_width = std::alignment_of<typename std::remove_pointer<buffer_t>::type>::value;

    const std::function<void(Interrupt::Type)>& _dmaCInterruptHandler;

    _DMACInterrupt _rx_interrupt{[&]() {
                                     if (_dmaCInterruptHandler) {
                                         _dmaCInterruptHandler(Interrupt::OnRxTransferDone);
                                     }
                                 },
                                 _first_dmac_interrupt};

    const uint8_t     dmacRxChannelNumber() const { return _rx_interrupt.getChannel(); }
    DmacCh_num* const dmacRxChannel() const { return dmac()->DMAC_CH_NUM + dmacRxChannelNumber(); };

    constexpr DMA_DMAC_RX(const std::function<void(Interrupt::Type)>& handler) : _dmaCInterruptHandler{handler} {};

    void resetRX() const {
        // init is called once after reset, so clean up after a reset
        SamCommon::enablePeripheralClock(peripheralId);

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
        dmacRxChannel()->DMAC_DSCR  = 0UL;
        dmacRxChannel()->DMAC_SADDR = (uint32_t)dmacPeripheralRxAddress();
        dmacRxChannel()->DMAC_DADDR = 0;
        dmacRxChannel()->DMAC_CTRLA = DMAC_CTRLA_BTSIZE(0);                // set later in setRx(...)
        dmacRxChannel()->DMAC_CTRLB = DMAC_CTRLB_SRC_DSCR_FETCH_DISABLE |  //
                                      DMAC_CTRLB_DST_DSCR_FETCH_DISABLE |  //
                                      DMAC_CTRLB_FC_PER2MEM_DMA_FC |       // memory->peripheral
                                      DMAC_CTRLB_SRC_INCR_FIXED |          // increment source address after each fetch
                                      DMAC_CTRLB_DST_INCR_INCREMENTING |   // destination address is fixed
                                      // leave DMAC_CTRLB_IEN as 0 - we want an interrupt
                                      0;  // for layout purposes

        dmacRxChannel()->DMAC_CFG = DMAC_CFG_DST_PER(dmacRxPeripheralId()) | DMAC_CFG_DST_H2SEL_HW;

        // enable interrupts for this channel
        dmac()->DMAC_EBCIER |= ((DMAC_EBCIER_BTC0 | DMAC_EBCIER_CBTC0 | DMAC_EBCIER_ERR0) << dmacRxChannelNumber());

    };

    void disableRx() const { dmac()->DMAC_CHDR = DMAC_CHDR_DIS0 << dmacRxChannelNumber(); };
    void enableRx() const { dmac()->DMAC_CHER = DMAC_CHER_ENA0 << dmacRxChannelNumber(); };
    void setRx(void* const buffer, const uint32_t length) const {
        dmacRxChannel()->DMAC_DADDR = (uint32_t)buffer;
        dmacRxChannel()->DMAC_CTRLA = DMAC_CTRLA_BTSIZE(length) | DMAC_CTRLA_SCSIZE_CHK_1 | DMAC_CTRLA_DCSIZE_CHK_1 |
                                      DMAC_CTRLA_SRC_WIDTH_BYTE | DMAC_CTRLA_DST_WIDTH_BYTE |
                                      // DON'T set DMAC_CTRLA_DONE
                                      0;
    };
    void setNextRx(void* const buffer, const uint32_t length) const {
        //            pdc->PERIPH_RNPR = (uint32_t)buffer;
        //            pdc->PERIPH_RNCR = length;
    };
    void flushRead() const {
        SamCommon::sync();
    };
    uint32_t leftToRead(bool include_next = false) const {
        //            if (include_next) {
        //            }
        SamCommon::sync();
        return (dmacRxChannel()->DMAC_CTRLA & DMAC_CTRLA_BTSIZE_Msk) << DMAC_CTRLA_BTSIZE_Pos;
    };
    uint32_t leftToReadNext() const {
        return 0;
        //            return pdc->PERIPH_RNCR;
    };
    bool     doneReading(bool include_next = false) const { return leftToRead(include_next) == 0; };
    bool     doneReadingNext() const { return leftToReadNext() == 0; };
    buffer_t getRXTransferPosition() const {
        return (buffer_t)dmacRxChannel()->DMAC_DADDR;
    };


    // Bundle it all up
    bool startRXTransfer(void* const    buffer,
                         const uint32_t length,
                         bool           handle_interrupts = true,
                         bool           include_next      = false) const {
        if (0 == length) {
            return false;
        }

        if (doneReading()) {
            disableRx();
            if (handle_interrupts) {
                stopRxDoneInterrupts();
            }
            setRx(buffer, length);
            enableRx();
            if (handle_interrupts) {
                startRxDoneInterrupts();
            }
        }
        // check to see if they overlap, in which case we're extending the region
        else if ((dmacRxChannel()->DMAC_DADDR >= (uint32_t)buffer) &&
                 (dmacRxChannel()->DMAC_DADDR < ((uint32_t)buffer + length))) {
            if (handle_interrupts) {
                stopRxDoneInterrupts();
            }

            // they overlap, we need to compute the new length
            decltype(dmacRxChannel()->DMAC_DADDR) pos_save;
            do {
                pos_save = dmacRxChannel()->DMAC_DADDR;

                // new_length = (start_pos + length) - current_positon
                auto new_length             = ((uint32_t)buffer + length) - pos_save;
                dmacRxChannel()->DMAC_CTRLA = DMAC_CTRLA_BTSIZE(new_length) | DMAC_CTRLA_SCSIZE_CHK_1 |
                                              DMAC_CTRLA_DCSIZE_CHK_1 | DMAC_CTRLA_SRC_WIDTH_BYTE |
                                              DMAC_CTRLA_DST_WIDTH_BYTE |
                                              // DON'T set DMAC_CTRLA_DONE
                                              0;

                // catch rare case where it advances while we were computing
            } while (dmacRxChannel()->DMAC_DADDR > pos_save);

            enableRx();
            if (handle_interrupts) {
                startRxDoneInterrupts();
            }
        }
        // otherwise, we set the next region, if requested. We DON'T attempt to extend it.
        else if (include_next && doneReadingNext()) {
            setNextRx(buffer, length);
            return true;
        }

        return (length > 0);
    };


    void startRxDoneInterrupts() const {
        dmac()->DMAC_EBCIER |= ((DMAC_EBCIER_BTC0 | DMAC_EBCIER_CBTC0 | DMAC_EBCIER_ERR0) << dmacRxChannelNumber());
    };
    void stopRxDoneInterrupts() const {
        dmac()->DMAC_EBCIDR |= ((DMAC_EBCIDR_BTC0 | DMAC_EBCIDR_CBTC0 | DMAC_EBCIDR_ERR0) << dmacRxChannelNumber());
    };

    // DMAC_Handler is handled with _tx_interrupt and _rx_interupt. They use
    // the std::function _dmaCInterruptHandler, which gets set by the peripheral
    // in the constexpr constructor.

    // These two get called from the peripheral interrupt handler,
    // and must always return false
    constexpr bool inRxBufferFullInterrupt() const { return false; };
};

}  // namespace Motate

#endif  // has DMAC

#endif /* end of include guard: SAMDMADMAC_H_ONCE */
