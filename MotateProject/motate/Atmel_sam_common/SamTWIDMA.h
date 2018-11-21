/*
 SamTWIDMA.h - Library for the Motate system
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
#error This file should ONLY be included from SamTWI.h, and never included directly
#endif

namespace Motate {
    // Empty class to use as a tag for template specialization
    struct TWI_tag {};

    // We only support XDMAC DMA for now
    #if defined (XDMAC)
    #pragma mark DMA_XDMAC TWI implementation

    template<uint8_t twiPeripheralNumber>
    struct DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber> : TWI_internal::TWIInfo<twiPeripheralNumber>
    {
        using info = TWI_internal::TWIInfo<twiPeripheralNumber>;
        using info::twi;
        static constexpr auto peripheralId = info::peripheralId;
        typedef char* buffer_t;
    };

    template<uint8_t twiPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<TWI_tag, twiPeripheralNumber> : virtual DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>::twi;
        using DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            switch (twiPeripheralNumber) {
                case (0): return 14;
                case (1): return 16;
                case (2): return 18;
            };
            return 0;
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &twi->TWIHS_THR;
        };
    };

    template<uint8_t twiPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<TWI_tag, twiPeripheralNumber> : virtual DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>::twi;
        using DMA_XDMAC_hardware<TWI_tag, twiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const xdmaRxPeripheralId()
        {
            switch (twiPeripheralNumber) {
                case (0): return 15;
                case (1): return 17;
                case (2): return 19;
            };
            return 0;
        };
        static constexpr volatile void * const xdmaPeripheralRxAddress()
        {
            return &twi->TWIHS_RHR;
        };
    };

    // Construct a DMA specialization that uses the XDMAC
    template<uint8_t periph_num>
    struct DMA<TWI_tag, periph_num> : DMA_XDMAC_RX<TWI_tag, periph_num>, DMA_XDMAC_TX<TWI_tag, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(Interrupt::Type)>& handler)
            : DMA_XDMAC_RX<TWI_tag, periph_num>{handler}, DMA_XDMAC_TX<TWI_tag, periph_num>{handler} {};

        using DMA_XDMAC_common::xdma;
        using rx = DMA_XDMAC_RX<TWI_tag, periph_num>;
        using tx = DMA_XDMAC_TX<TWI_tag, periph_num>;
        using rx::xdmaRxChannelNumber;
        using tx::xdmaTxChannelNumber;

        void setInterrupts(const Interrupt::Type interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    tx::startTxDoneInterrupts();
                } else {
                    tx::stopTxDoneInterrupts();
                }

                if (interrupts & Interrupt::OnRxTransferDone) {
                    rx::startRxDoneInterrupts();
                } else {
                    rx::stopRxDoneInterrupts();
                }
            } else {
                tx::stopTxDoneInterrupts();
                rx::stopRxDoneInterrupts();

            }
        };

        void reset() const {
            tx::resetTX();
            rx::resetRX();
        };

        void enable() const {
            xdma()->XDMAC_GE = (XDMAC_GIE_IE0 << rx::xdmaRxChannelNumber()) | (XDMAC_GIE_IE0 << tx::xdmaTxChannelNumber());
        }

        void disable() const
        {
            xdma()->XDMAC_GD = (XDMAC_GID_ID0 << rx::xdmaRxChannelNumber()) | (XDMAC_GID_ID0 << tx::xdmaTxChannelNumber());
        };
    };
    #endif // TWI + XDMAC
} //namespace Motate
