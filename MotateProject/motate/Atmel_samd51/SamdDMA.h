/*
 utility/SamdDMA.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2026 Riley Porter
 Copyright (c) 2026 Synthetos LLC

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

#ifndef SAMDDMA_H_ONCE
#define SAMDDMA_H_ONCE

#include "sam.h"
#include "SamdCommon.h"

#include <functional>
#include <type_traits>
#include <cstring>

namespace Motate {

    /**************************************************
     *
     * SAMD51 DMAC - Direct Memory Access Controller
     *
     * The SAMD51 DMAC has 32 channels and uses descriptor-based transfers.
     * Descriptors must be placed in SRAM and properly aligned.
     *
     * Key features:
     * - 32 DMA channels with 4 priority levels
     * - Descriptor-based transfers (linked list support)
     * - Configurable beat size (8/16/32-bit)
     * - Peripheral trigger sources
     *
     **************************************************/

    // DMA descriptor structure - must match hardware format
    // SAMD51 descriptors are 16 bytes and must be aligned to 16 bytes
    struct alignas(16) DMADescriptor {
        uint16_t btctrl;      // Block Transfer Control
        uint16_t btcnt;       // Block Transfer Count
        uint32_t srcaddr;     // Source Address (end address for incrementing)
        uint32_t dstaddr;     // Destination Address (end address for incrementing)
        uint32_t descaddr;    // Next Descriptor Address (0 = end of list)
    };

    // Block Transfer Control register bits
    namespace BTCTRL {
        constexpr uint16_t VALID       = (1 << 0);   // Descriptor valid
        constexpr uint16_t EVOSEL_DISABLE = (0 << 1); // Event output disabled
        constexpr uint16_t EVOSEL_BLOCK   = (1 << 1); // Event on block transfer complete
        constexpr uint16_t EVOSEL_BEAT    = (3 << 1); // Event on beat transfer
        constexpr uint16_t BLOCKACT_NOACT = (0 << 3); // No action after block
        constexpr uint16_t BLOCKACT_INT   = (1 << 3); // Interrupt after block
        constexpr uint16_t BLOCKACT_SUSPEND = (2 << 3); // Suspend channel after block
        constexpr uint16_t BLOCKACT_BOTH  = (3 << 3); // Interrupt and suspend
        constexpr uint16_t BEATSIZE_BYTE  = (0 << 8); // 8-bit beat
        constexpr uint16_t BEATSIZE_HWORD = (1 << 8); // 16-bit beat
        constexpr uint16_t BEATSIZE_WORD  = (2 << 8); // 32-bit beat
        constexpr uint16_t SRCINC         = (1 << 10); // Source address increment
        constexpr uint16_t DSTINC         = (1 << 11); // Destination address increment
        constexpr uint16_t STEPSEL_DST    = (0 << 12); // Step size applied to destination
        constexpr uint16_t STEPSEL_SRC    = (1 << 12); // Step size applied to source
        constexpr uint16_t STEPSIZE_X1    = (0 << 13); // Step size = beat size * 1
        constexpr uint16_t STEPSIZE_X2    = (1 << 13); // Step size = beat size * 2
        constexpr uint16_t STEPSIZE_X4    = (2 << 13); // Step size = beat size * 4
        constexpr uint16_t STEPSIZE_X8    = (3 << 13); // Step size = beat size * 8
    }

    // DMA trigger sources for SAMD51
    // These are from the SAMD51 datasheet Table 22-3
    namespace DMAC_TRIGSRC {
        // Software trigger
        constexpr uint8_t DISABLE   = 0x00;  // Software or event trigger only

        // SERCOM DMA triggers (0x04-0x13)
        // Each SERCOM has RX and TX triggers
        constexpr uint8_t SERCOM0_RX = 0x04;
        constexpr uint8_t SERCOM0_TX = 0x05;
        constexpr uint8_t SERCOM1_RX = 0x06;
        constexpr uint8_t SERCOM1_TX = 0x07;
        constexpr uint8_t SERCOM2_RX = 0x08;
        constexpr uint8_t SERCOM2_TX = 0x09;
        constexpr uint8_t SERCOM3_RX = 0x0A;
        constexpr uint8_t SERCOM3_TX = 0x0B;
        constexpr uint8_t SERCOM4_RX = 0x0C;
        constexpr uint8_t SERCOM4_TX = 0x0D;
        constexpr uint8_t SERCOM5_RX = 0x0E;
        constexpr uint8_t SERCOM5_TX = 0x0F;
        constexpr uint8_t SERCOM6_RX = 0x10;
        constexpr uint8_t SERCOM6_TX = 0x11;
        constexpr uint8_t SERCOM7_RX = 0x12;
        constexpr uint8_t SERCOM7_TX = 0x13;

        // TC DMA triggers (0x14-0x15 for TC0/TC1 OVF)
        constexpr uint8_t TC0_OVF   = 0x14;
        constexpr uint8_t TC1_OVF   = 0x15;

        // TCC0: 6 compare channels + overflow
        constexpr uint8_t TCC0_OVF  = 0x16;  // TCC0 Overflow
        constexpr uint8_t TCC0_MC0  = 0x17;  // TCC0 Match/Compare 0
        constexpr uint8_t TCC0_MC1  = 0x18;  // TCC0 Match/Compare 1
        constexpr uint8_t TCC0_MC2  = 0x19;  // TCC0 Match/Compare 2
        constexpr uint8_t TCC0_MC3  = 0x1A;  // TCC0 Match/Compare 3
        constexpr uint8_t TCC0_MC4  = 0x1B;  // TCC0 Match/Compare 4
        constexpr uint8_t TCC0_MC5  = 0x1C;  // TCC0 Match/Compare 5

        // TCC1: 4 compare channels + overflow
        constexpr uint8_t TCC1_OVF  = 0x1D;  // TCC1 Overflow
        constexpr uint8_t TCC1_MC0  = 0x1E;  // TCC1 Match/Compare 0
        constexpr uint8_t TCC1_MC1  = 0x1F;  // TCC1 Match/Compare 1
        constexpr uint8_t TCC1_MC2  = 0x20;  // TCC1 Match/Compare 2
        constexpr uint8_t TCC1_MC3  = 0x21;  // TCC1 Match/Compare 3

        // TCC2: 3 compare channels + overflow
        constexpr uint8_t TCC2_OVF  = 0x22;  // TCC2 Overflow
        constexpr uint8_t TCC2_MC0  = 0x23;  // TCC2 Match/Compare 0
        constexpr uint8_t TCC2_MC1  = 0x24;  // TCC2 Match/Compare 1
        constexpr uint8_t TCC2_MC2  = 0x25;  // TCC2 Match/Compare 2

        // TCC3: 2 compare channels + overflow
        constexpr uint8_t TCC3_OVF  = 0x26;  // TCC3 Overflow
        constexpr uint8_t TCC3_MC0  = 0x27;  // TCC3 Match/Compare 0
        constexpr uint8_t TCC3_MC1  = 0x28;  // TCC3 Match/Compare 1

        // TCC4: 2 compare channels + overflow
        constexpr uint8_t TCC4_OVF  = 0x29;  // TCC4 Overflow
        constexpr uint8_t TCC4_MC0  = 0x2A;  // TCC4 Match/Compare 0
        constexpr uint8_t TCC4_MC1  = 0x2B;  // TCC4 Match/Compare 1
    }

    /**************************************************
     *
     * DMA Channel Manager
     *
     * Static class to manage DMAC initialization and
     * channel allocation.
     *
     **************************************************/

    // Maximum number of DMA channels we will support
    // Using fewer saves memory (16 bytes per channel for descriptors)
    constexpr uint8_t DMAC_MAX_CHANNELS = 8;

    // Descriptor memory sections - must be contiguous and properly aligned
    // These are defined in SamdDMA.cpp
    extern DMADescriptor dmac_descriptors[DMAC_MAX_CHANNELS];
    extern DMADescriptor dmac_writeback[DMAC_MAX_CHANNELS];

    struct DMAController {
        static bool _initialized;
        static uint32_t _channels_in_use;  // Bitmask of allocated channels

        /**
         * Initialize the DMAC peripheral
         * Must be called before any DMA operations
         */
        static void init() {
            if (_initialized) return;

            // Enable DMAC clock (on AHB bus, always enabled but be explicit)
            MCLK->AHBMASK.bit.DMAC_ = 1;

            // Disable DMAC before configuration
            DMAC->CTRL.bit.DMAENABLE = 0;
            DMAC->CTRL.bit.SWRST = 1;
            while (DMAC->CTRL.bit.SWRST);

            // Set descriptor base addresses
            DMAC->BASEADDR.reg = (uint32_t)dmac_descriptors;
            DMAC->WRBADDR.reg = (uint32_t)dmac_writeback;

            // Enable all priority levels
            DMAC->CTRL.reg = DMAC_CTRL_LVLEN0 | DMAC_CTRL_LVLEN1 |
                             DMAC_CTRL_LVLEN2 | DMAC_CTRL_LVLEN3;

            // Enable DMAC
            DMAC->CTRL.bit.DMAENABLE = 1;

            _initialized = true;
            _channels_in_use = 0;
        }

        /**
         * Allocate a DMA channel
         * Returns channel number (0-DMAC_MAX_CHANNELS-1) or -1 if none available
         */
        static int8_t allocateChannel() {
            if (!_initialized) init();

            for (uint8_t ch = 0; ch < DMAC_MAX_CHANNELS; ch++) {
                if (!(_channels_in_use & (1 << ch))) {
                    _channels_in_use |= (1 << ch);
                    return ch;
                }
            }
            return -1;  // No channels available
        }

        /**
         * Free a DMA channel
         */
        static void freeChannel(uint8_t channel) {
            if (channel < DMAC_MAX_CHANNELS) {
                // Disable the channel first
                DMAC->Channel[channel].CHCTRLA.bit.ENABLE = 0;
                _channels_in_use &= ~(1 << channel);
            }
        }

        /**
         * Get pointer to a channel's descriptor
         */
        static DMADescriptor* getDescriptor(uint8_t channel) {
            if (channel < DMAC_MAX_CHANNELS) {
                return &dmac_descriptors[channel];
            }
            return nullptr;
        }

        /**
         * Get pointer to a channel's writeback descriptor
         */
        static DMADescriptor* getWriteback(uint8_t channel) {
            if (channel < DMAC_MAX_CHANNELS) {
                return &dmac_writeback[channel];
            }
            return nullptr;
        }
    };

    /**************************************************
     *
     * DMA Channel Template
     *
     * Provides a high-level interface for a single DMA channel.
     * Used by peripherals that need DMA support.
     *
     **************************************************/

    template<uint8_t channelNum>
    struct DMAChannel {
        static_assert(channelNum < DMAC_MAX_CHANNELS, "DMA channel number out of range");

        // Descriptor for this channel
        static DMADescriptor* descriptor() {
            return DMAController::getDescriptor(channelNum);
        }

        // Writeback descriptor for this channel
        static DMADescriptor* writeback() {
            return DMAController::getWriteback(channelNum);
        }

        /**
         * Initialize and configure this DMA channel
         * @param triggerSource - Peripheral trigger source (from DMAC_TRIGSRC)
         * @param triggerAction - Trigger action (beat, block, or transaction)
         * @param priority - Priority level (0-3)
         */
        static void init(uint8_t triggerSource,
                         uint8_t triggerAction = DMAC_CHCTRLA_TRIGACT_BURST_Val,
                         uint8_t priority = 0)
        {
            // Ensure DMAC is initialized
            DMAController::init();

            // Disable channel before configuration
            DMAC->Channel[channelNum].CHCTRLA.bit.ENABLE = 0;
            while (DMAC->Channel[channelNum].CHCTRLA.bit.ENABLE);

            // Reset channel
            DMAC->Channel[channelNum].CHCTRLA.bit.SWRST = 1;
            while (DMAC->Channel[channelNum].CHCTRLA.bit.SWRST);

            // Configure channel
            DMAC->Channel[channelNum].CHCTRLA.reg =
                DMAC_CHCTRLA_TRIGSRC(triggerSource) |
                DMAC_CHCTRLA_TRIGACT(triggerAction) |
                DMAC_CHCTRLA_BURSTLEN(0);  // Single beat burst

            // Set priority level
            DMAC->Channel[channelNum].CHPRILVL.bit.PRILVL = priority & 0x3;

            // Clear all interrupt flags
            DMAC->Channel[channelNum].CHINTFLAG.reg = DMAC_CHINTFLAG_MASK;
        }

        /**
         * Configure a memory-to-peripheral transfer
         * @param srcBuffer - Source buffer in memory
         * @param dstAddr - Destination peripheral register address
         * @param count - Number of beats to transfer
         * @param beatSize - Size of each beat (0=byte, 1=halfword, 2=word)
         */
        static void configureMemToPeripheral(const void* srcBuffer,
                                              volatile void* dstAddr,
                                              uint16_t count,
                                              uint8_t beatSize = 1)
        {
            DMADescriptor* desc = descriptor();

            // Calculate beat size in bytes
            uint8_t bytesPerBeat = 1 << beatSize;

            // Configure block transfer control
            desc->btctrl = BTCTRL::VALID |
                           BTCTRL::BLOCKACT_NOACT |
                           (beatSize << 8) |      // BEATSIZE
                           BTCTRL::SRCINC |       // Source address increments
                           BTCTRL::STEPSEL_SRC;   // Step size applies to source

            // Set transfer count
            desc->btcnt = count;

            // Source address: end of buffer (incremented mode uses end address)
            desc->srcaddr = (uint32_t)srcBuffer + (count * bytesPerBeat);

            // Destination address: fixed peripheral register
            desc->dstaddr = (uint32_t)dstAddr;

            // No next descriptor (single block transfer)
            desc->descaddr = 0;
        }

        /**
         * Configure a peripheral-to-memory transfer
         */
        static void configurePeripheralToMem(volatile void* srcAddr,
                                              void* dstBuffer,
                                              uint16_t count,
                                              uint8_t beatSize = 1)
        {
            DMADescriptor* desc = descriptor();

            // Calculate beat size in bytes
            uint8_t bytesPerBeat = 1 << beatSize;

            // Configure block transfer control
            desc->btctrl = BTCTRL::VALID |
                           BTCTRL::BLOCKACT_NOACT |
                           (beatSize << 8) |      // BEATSIZE
                           BTCTRL::DSTINC |       // Destination address increments
                           BTCTRL::STEPSEL_DST;   // Step size applies to destination

            // Set transfer count
            desc->btcnt = count;

            // Source address: fixed peripheral register
            desc->srcaddr = (uint32_t)srcAddr;

            // Destination address: end of buffer
            desc->dstaddr = (uint32_t)dstBuffer + (count * bytesPerBeat);

            // No next descriptor
            desc->descaddr = 0;
        }

        /**
         * Enable the DMA channel
         */
        static void enable() {
            DMAC->Channel[channelNum].CHCTRLA.bit.ENABLE = 1;
        }

        /**
         * Disable the DMA channel
         */
        static void disable() {
            DMAC->Channel[channelNum].CHCTRLA.bit.ENABLE = 0;
            while (DMAC->Channel[channelNum].CHCTRLA.bit.ENABLE);
        }

        /**
         * Trigger a software DMA transfer
         */
        static void trigger() {
            DMAC->SWTRIGCTRL.reg = (1 << channelNum);
        }

        /**
         * Check if the transfer is complete
         */
        static bool isTransferComplete() {
            return DMAC->Channel[channelNum].CHINTFLAG.bit.TCMPL;
        }

        /**
         * Check if the channel is busy
         */
        static bool isBusy() {
            return DMAC->Channel[channelNum].CHSTATUS.bit.BUSY;
        }

        /**
         * Check if the channel is pending
         */
        static bool isPending() {
            return DMAC->Channel[channelNum].CHSTATUS.bit.PEND;
        }

        /**
         * Clear the transfer complete flag
         */
        static void clearTransferComplete() {
            DMAC->Channel[channelNum].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
        }

        /**
         * Clear all interrupt flags
         */
        static void clearAllFlags() {
            DMAC->Channel[channelNum].CHINTFLAG.reg = DMAC_CHINTFLAG_MASK;
        }

        /**
         * Get remaining transfer count
         */
        static uint16_t getRemainingCount() {
            return writeback()->btcnt;
        }

        /**
         * Enable transfer complete interrupt
         */
        static void enableInterrupt() {
            DMAC->Channel[channelNum].CHINTENSET.reg = DMAC_CHINTENSET_TCMPL;

            // Enable NVIC interrupt for this channel
            // SAMD51 has separate IRQs for channels 0-4, then shared for 5+
            IRQn_Type irq;
            if (channelNum < 5) {
                irq = static_cast<IRQn_Type>(DMAC_0_IRQn + channelNum);
            } else {
                irq = DMAC_4_IRQn;  // Channels 4+ share this IRQ
            }
            NVIC_EnableIRQ(irq);
        }

        /**
         * Disable transfer complete interrupt
         */
        static void disableInterrupt() {
            DMAC->Channel[channelNum].CHINTENCLR.reg = DMAC_CHINTENCLR_TCMPL;
        }

        /**
         * Suspend the channel
         */
        static void suspend() {
            DMAC->Channel[channelNum].CHCTRLB.reg = DMAC_CHCTRLB_CMD_SUSPEND;
            while (!DMAC->Channel[channelNum].CHINTFLAG.bit.SUSP);
        }

        /**
         * Resume the channel
         */
        static void resume() {
            DMAC->Channel[channelNum].CHCTRLB.reg = DMAC_CHCTRLB_CMD_RESUME;
        }
    };

    /**************************************************
     *
     * Dynamic DMA Channel
     *
     * For cases where the channel number is not known
     * at compile time.
     *
     **************************************************/

    struct DynamicDMAChannel {
        int8_t _channel;
        bool _owned;

        DynamicDMAChannel() : _channel(-1), _owned(false) {}

        ~DynamicDMAChannel() {
            if (_owned && _channel >= 0) {
                release();
            }
        }

        // Allocate a channel
        bool allocate() {
            if (_channel >= 0) return true;  // Already have one
            _channel = DMAController::allocateChannel();
            _owned = (_channel >= 0);
            return _owned;
        }

        // Use a specific channel (not owned)
        void useChannel(uint8_t ch) {
            if (_owned && _channel >= 0) {
                release();
            }
            _channel = ch;
            _owned = false;
        }

        // Release the channel
        void release() {
            if (_channel >= 0 && _owned) {
                disable();
                DMAController::freeChannel(_channel);
            }
            _channel = -1;
            _owned = false;
        }

        bool isValid() const { return _channel >= 0; }
        uint8_t channel() const { return _channel; }

        // Descriptor access
        DMADescriptor* descriptor() {
            return DMAController::getDescriptor(_channel);
        }

        DMADescriptor* writeback() {
            return DMAController::getWriteback(_channel);
        }

        // Initialize the channel
        void init(uint8_t triggerSource,
                  uint8_t triggerAction = DMAC_CHCTRLA_TRIGACT_BURST_Val,
                  uint8_t priority = 0)
        {
            if (_channel < 0) return;

            DMAController::init();

            DMAC->Channel[_channel].CHCTRLA.bit.ENABLE = 0;
            while (DMAC->Channel[_channel].CHCTRLA.bit.ENABLE);

            DMAC->Channel[_channel].CHCTRLA.bit.SWRST = 1;
            while (DMAC->Channel[_channel].CHCTRLA.bit.SWRST);

            DMAC->Channel[_channel].CHCTRLA.reg =
                DMAC_CHCTRLA_TRIGSRC(triggerSource) |
                DMAC_CHCTRLA_TRIGACT(triggerAction) |
                DMAC_CHCTRLA_BURSTLEN(0);

            DMAC->Channel[_channel].CHPRILVL.bit.PRILVL = priority & 0x3;
            DMAC->Channel[_channel].CHINTFLAG.reg = DMAC_CHINTFLAG_MASK;
        }

        // Configure memory to peripheral transfer
        void configureMemToPeripheral(const void* srcBuffer,
                                       volatile void* dstAddr,
                                       uint16_t count,
                                       uint8_t beatSize = 1)
        {
            if (_channel < 0) return;

            DMADescriptor* desc = descriptor();
            uint8_t bytesPerBeat = 1 << beatSize;

            desc->btctrl = BTCTRL::VALID |
                           BTCTRL::BLOCKACT_NOACT |
                           (beatSize << 8) |
                           BTCTRL::SRCINC |
                           BTCTRL::STEPSEL_SRC;

            desc->btcnt = count;
            desc->srcaddr = (uint32_t)srcBuffer + (count * bytesPerBeat);
            desc->dstaddr = (uint32_t)dstAddr;
            desc->descaddr = 0;
        }

        void enable() {
            if (_channel >= 0) {
                DMAC->Channel[_channel].CHCTRLA.bit.ENABLE = 1;
            }
        }

        void disable() {
            if (_channel >= 0) {
                DMAC->Channel[_channel].CHCTRLA.bit.ENABLE = 0;
                while (DMAC->Channel[_channel].CHCTRLA.bit.ENABLE);
            }
        }

        void trigger() {
            if (_channel >= 0) {
                DMAC->SWTRIGCTRL.reg = (1 << _channel);
            }
        }

        bool isTransferComplete() {
            if (_channel < 0) return true;
            return DMAC->Channel[_channel].CHINTFLAG.bit.TCMPL;
        }

        bool isBusy() {
            if (_channel < 0) return false;
            return DMAC->Channel[_channel].CHSTATUS.bit.BUSY;
        }

        void clearTransferComplete() {
            if (_channel >= 0) {
                DMAC->Channel[_channel].CHINTFLAG.reg = DMAC_CHINTFLAG_TCMPL;
            }
        }

        void clearAllFlags() {
            if (_channel >= 0) {
                DMAC->Channel[_channel].CHINTFLAG.reg = DMAC_CHINTFLAG_MASK;
            }
        }

        uint16_t getRemainingCount() {
            if (_channel < 0) return 0;
            return writeback()->btcnt;
        }
    };

} // namespace Motate

#endif /* end of include guard: SAMDDMA_H_ONCE */
