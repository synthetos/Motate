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
    // We're deducing if there's a SPI0 and it has a PDC
    #if defined(CAN_SPI_PDC_DMA)

    #else
    #pragma mark DMA_XDMAC SPI implementation

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_hardware<Spi*, spiPeripheralNumber> : Motate::SPI_internal::SPIInfo<spiPeripheralNumber>
    {
        using SPI_internal::SPIInfo<spiPeripheralNumber>::spi;
        static constexpr auto peripheralId = SPI_internal::SPIInfo<spiPeripheralNumber>::peripheralId;
        typedef char* buffer_t;
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<Spi*, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::spi;
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::peripheralId;

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
            return &spi->SPI_TDR;
        };
    };

    template<uint8_t spiPeripheralNumber>
    struct DMA_XDMAC_RX_hardware<Spi*, spiPeripheralNumber> : virtual DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::spi;
        using DMA_XDMAC_hardware<Spi*, spiPeripheralNumber>::peripheralId;

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
            return &spi->SPI_RDR;
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Spi*, periph_num> : DMA_XDMAC_RX<Spi*, periph_num>, DMA_XDMAC_TX<Spi*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(uint16_t)> &handler) : DMA_XDMAC_RX<Spi*, periph_num>{handler}, DMA_XDMAC_TX<Spi*, periph_num>{handler} {};

        using DMA_XDMAC_common::xdma;
        using DMA_XDMAC_RX<Spi*, periph_num>::xdmaRxChannelNumber;
        using DMA_XDMAC_TX<Spi*, periph_num>::xdmaTxChannelNumber;

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
            } else {
                DMA_XDMAC_TX<Spi*, periph_num>::stopTxDoneInterrupts();
                DMA_XDMAC_RX<Spi*, periph_num>::stopRxDoneInterrupts();

            }
        };

        void reset() const {
            DMA_XDMAC_TX<Spi*, periph_num>::resetTX();
            DMA_XDMAC_RX<Spi*, periph_num>::resetRX();
        };

        void enable() const {
            xdma()->XDMAC_GE = (XDMAC_GIE_IE0 << xdmaRxChannelNumber()) | (XDMAC_GIE_IE0 << xdmaTxChannelNumber());
        }

        void disable() const
        {
            xdma()->XDMAC_GD = (XDMAC_GID_ID0 << xdmaRxChannelNumber()) | (XDMAC_GID_ID0 << xdmaRxChannelNumber());
        };
    };
    #endif // SPI + XDMAC
}; //namespace Motate
