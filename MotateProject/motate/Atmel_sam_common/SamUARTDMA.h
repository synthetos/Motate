/*
 SamUARTDMA.h - Library for the Motate system
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

#if !(defined(SAMUART_H_ONCE)) && 0
#error This file should ONLY be included from SamUART.h, and never included directly
#endif

namespace Motate {
    // NOTE: If we have an XDMAC peripheral, we don't have PDC, and it's the
    // only want to DMC all of these peripherals.
    // So we don't need the XDMAC deduction -- it's already done.

    // We're deducing if there's a USART and it has a PDC
    // Notice that this relies on defines set up in SamCommon.h
#if !defined(HAS_PDC_USART0) && defined(HAS_USART0)

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
#if !defined(HAS_PDC_UART0) && defined(HAS_UART0)

#pragma mark DMA_XDMAC Usart implementation

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_hardware<Uart*, uartPeripheralNumber> : virtual Motate::UART_internal::UARTInfo<uartPeripheralNumber>
    {
        static constexpr auto peripheralId { Motate::UART_internal::UARTInfo<uartPeripheralNumber>::peripheralId };
        static constexpr Uart * const uart() {
            return Motate::UART_internal::UARTInfo<uartPeripheralNumber>::uart;
        };

        typedef char* buffer_t;
    };

    template<uint8_t uartPeripheralNumber>
    struct DMA_XDMAC_TX_hardware<Uart*, uartPeripheralNumber> : virtual DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>::uart;
        using DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>::peripheralId;

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
        using DMA_XDMAC_hardware<Uart*, uartPeripheralNumber>::peripheralId;

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
} // namespace Motate
