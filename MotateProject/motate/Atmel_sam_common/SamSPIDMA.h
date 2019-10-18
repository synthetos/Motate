/*
 SamSPIDMA.h - Library for the Motate system
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

#ifndef SAMSPI_H_ONCE
#error This file should ONLY be included from SamSPI.h, and never included directly
#endif

namespace Motate {
    // Empty class to use as a tag for template specialization
    struct SPI_tag {};

    // We're deducing if there's a SPI0 and it has a PDC
    #if defined(CAN_SPI_PDC_DMA)
    template<uint8_t spiPeripheralNumber>
    struct DMA_PDC_hardware<SPI_tag, spiPeripheralNumber> : virtual Motate::SPI_internal::SPIInfo<spiPeripheralNumber> {
        using Motate::SPI_internal::SPIInfo<spiPeripheralNumber>::spi;
        static constexpr auto pdc() {
            return Motate::SPI_internal::SPIInfo<spiPeripheralNumber>::pdc;
        };

        typedef char* buffer_t ;

        void startRxDoneInterrupts(const bool include_next = false) const {
            spi->SPI_IER = SPI_IER_RXBUFF; : SPI_IER_ENDRX;
        };
        void stopRxDoneInterrupts(const bool include_next = false) const {
            spi->SPI_IDR = SPI_IDR_RXBUFF; : SPI_IDR_ENDRX;
        };
        void startTxDoneInterrupts(const bool include_next = true) const {
            spi->SPI_IER = SPI_IER_TXBUFE : SPI_IER_ENDTX;
        };
        void stopTxDoneInterrupts(const bool include_next = true) const {
            spi->SPI_IDR = SPI_IDR_TXBUFE : SPI_IDR_ENDTX;
        };

        int16_t readByte() const {
            if (!(uart()->UART_SR & UART_SR_RXRDY)) { return -1; }
            return (uart()->UART_RHR & UART_RHR_RXCHR_Msk);
        }

        bool inRxBufferFullInterrupt() const
        {
            auto SPI_SR_hold = spi->SPI_SR;
            auto SPI_IMR_hold = spi->SPI_IMR;

            // we check if the interupt is enabled, then read it if it is
            if ((SPI_IMR_hold & SPI_IMR_RXBUFF) && (SPI_SR_hold & SPI_SR_RXBUFF))
            {
                return true;
            }

            return false;
        }

        bool inTxBufferEmptyInterrupt() const
        {
            auto SPI_SR_hold = spi->SPI_SR;
            auto SPI_IMR_hold = spi->SPI_IMR;

            // we check if the interupt is enabled, then read it if it is
            if ((SPI_IMR_hold & SPI_IMR_TXBUFE) && (SPI_SR_hold & SPI_SR_TXBUFE))
            {
                return true;
            }
            return false;
        }
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<SPI_tag, periph_num> : DMA_PDC<SPI_tag, periph_num> {
        // nothing to do here, except for a constxpr constructor
        // we take the handler, but we don't actually handle interrupts for the peripheral
        // so we ignore it
        constexpr DMA(const std::function<void(Interrupt::Type)> &handler) : DMA_PDC<SPI_tag, periph_num>{} {};
    };

    #elif defined (XDMAC)

    #pragma mark DMA_XDMAC SPI implementation

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber> : Motate::SPI_internal::SPIInfo<spiPeripheralNumber>
    {
        using SPI_internal::SPIInfo<spiPeripheralNumber>::spi;
        static constexpr auto peripheralId = SPI_internal::SPIInfo<spiPeripheralNumber>::peripheralId;
        typedef char* buffer_t;
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<SPI_tag, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>::spi;
        using DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 1;
                case (1): return 3;
            };
            return 0;
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &spi->SPI_TDR;
        };
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<SPI_tag, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>::spi;
        using DMA_XDMAC_hardware<SPI_tag, spiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const xdmaRxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 2;
                case (1): return 4;
            };
            return 0;
        };
        static constexpr volatile void * const xdmaPeripheralRxAddress()
        {
            return &spi->SPI_RDR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<SPI_tag, periph_num> : DMA_XDMAC_RX<SPI_tag, periph_num>, DMA_XDMAC_TX<SPI_tag, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(Interrupt::Type)> &handler) : DMA_XDMAC_RX<SPI_tag, periph_num>{handler}, DMA_XDMAC_TX<SPI_tag, periph_num>{handler} {};

        using DMA_XDMAC_common::xdma;
        using rx = DMA_XDMAC_RX<SPI_tag, periph_num>;
        using tx = DMA_XDMAC_TX<SPI_tag, periph_num>;
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

    #elif defined (DMAC)

    #pragma mark DMA_DMAC SPI implementation

    template<uint8_t spiPeripheralNumber>
    struct DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber> : Motate::SPI_internal::SPIInfo<spiPeripheralNumber>
    {
        using SPI_internal::SPIInfo<spiPeripheralNumber>::spi;
        static constexpr auto peripheralId = SPI_internal::SPIInfo<spiPeripheralNumber>::peripheralId;
        typedef char* buffer_t;
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_DMAC_TX_hardware<SPI_tag, spiPeripheralNumber> : virtual DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>, virtual DMA_DMAC_common {
        using DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>::spi;
        using DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const dmacTxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 1;
                case (1): return 5;
            };
            return 0;
        };
        static constexpr volatile void * const dmacPeripheralTxAddress()
        {
            return &spi->SPI_TDR;
        };
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_DMAC_RX_hardware<SPI_tag, spiPeripheralNumber> : virtual DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>, virtual DMA_DMAC_common {
        using DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>::spi;
        using DMA_DMAC_hardware<SPI_tag, spiPeripheralNumber>::peripheralId;

        static constexpr uint8_t const dmacRxPeripheralId()
        {
            switch (spiPeripheralNumber) {
                case (0): return 2;
                case (1): return 6;
            };
            return 0;
        };
        static constexpr volatile void * const dmacPeripheralRxAddress()
        {
            return &spi->SPI_RDR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<SPI_tag, periph_num> : DMA_DMAC_RX<SPI_tag, periph_num>, DMA_DMAC_TX<SPI_tag, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(Interrupt::Type)> &handler) : DMA_DMAC_RX<SPI_tag, periph_num>{handler}, DMA_DMAC_TX<SPI_tag, periph_num>{handler} {};

        using DMA_DMAC_common::dmac;
        using rx = DMA_DMAC_RX<SPI_tag, periph_num>;
        using tx = DMA_DMAC_TX<SPI_tag, periph_num>;
        using rx::dmacRxChannelNumber;
        using tx::dmacTxChannelNumber;

        void setInterrupts(const Interrupt::Type interrupts) const
        {
            DMA_DMAC_common::setInterrupts(interrupts);

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
            tx::enableTX();
            rx::enableRX();
        }

        void disable() const
        {
            tx::disableTX();
            rx::disableRX();
        };
    };
    #endif // SPI + DMAC
} //namespace Motate
