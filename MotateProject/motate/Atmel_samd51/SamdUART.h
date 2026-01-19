/*
 * SamdUART.h - UART Library for SAMD51 using SERCOM
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
 *
 * This file is part of the Motate Library.
 *
 * SAMD51 SERCOM USART Implementation
 * Follows the same interface pattern as Atmel_sam_common/SamUART.h
 */

#ifndef SAMD_UART_H_ONCE
#define SAMD_UART_H_ONCE

#include "SamdCommon.h"

#include <functional>
#include <type_traits>
#include <cstdint>

namespace Motate {

    // Debug counters
    extern volatile uint32_t g_uart_rx_count;
    extern volatile uint32_t g_uart_tx_count;
    extern volatile uint32_t g_uart_rx_overrun;

    // Forward declarations for interrupt types from MotateCommon.h
    // These are defined in MotateCommon.h which is included before us

    // SERCOM info structure - provides hardware details for each SERCOM
    template<uint8_t sercomNum> struct SERCOMInfo {
        // Default - invalid
        static Sercom* sercom() { return nullptr; }
        static constexpr uint32_t peripheralId = 0;
        static constexpr IRQn_Type IRQ_0 = (IRQn_Type)0;  // DRE interrupt
        static constexpr IRQn_Type IRQ_1 = (IRQn_Type)0;  // TXC interrupt
        static constexpr IRQn_Type IRQ_2 = (IRQn_Type)0;  // RXC interrupt
        static constexpr IRQn_Type IRQ_3 = (IRQn_Type)0;  // Error interrupt
        static constexpr uint8_t gclkId = 0;
    };

    // SERCOM0 specialization
    template<> struct SERCOMInfo<0> {
        static Sercom* sercom() { return SERCOM0; }
        static constexpr uint32_t peripheralId = ID_SERCOM0;
        static constexpr IRQn_Type IRQ_0 = SERCOM0_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM0_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM0_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM0_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM0_GCLK_ID_CORE;
    };

    // SERCOM1 specialization
    template<> struct SERCOMInfo<1> {
        static Sercom* sercom() { return SERCOM1; }
        static constexpr uint32_t peripheralId = ID_SERCOM1;
        static constexpr IRQn_Type IRQ_0 = SERCOM1_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM1_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM1_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM1_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM1_GCLK_ID_CORE;
    };

    // SERCOM2 specialization
    template<> struct SERCOMInfo<2> {
        static Sercom* sercom() { return SERCOM2; }
        static constexpr uint32_t peripheralId = ID_SERCOM2;
        static constexpr IRQn_Type IRQ_0 = SERCOM2_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM2_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM2_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM2_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM2_GCLK_ID_CORE;
    };

    // SERCOM3 specialization
    template<> struct SERCOMInfo<3> {
        static Sercom* sercom() { return SERCOM3; }
        static constexpr uint32_t peripheralId = ID_SERCOM3;
        static constexpr IRQn_Type IRQ_0 = SERCOM3_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM3_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM3_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM3_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM3_GCLK_ID_CORE;
    };

    // SERCOM4 specialization
    template<> struct SERCOMInfo<4> {
        static Sercom* sercom() { return SERCOM4; }
        static constexpr uint32_t peripheralId = ID_SERCOM4;
        static constexpr IRQn_Type IRQ_0 = SERCOM4_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM4_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM4_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM4_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM4_GCLK_ID_CORE;
    };

    // SERCOM5 specialization
    template<> struct SERCOMInfo<5> {
        static Sercom* sercom() { return SERCOM5; }
        static constexpr uint32_t peripheralId = ID_SERCOM5;
        static constexpr IRQn_Type IRQ_0 = SERCOM5_0_IRQn;
        static constexpr IRQn_Type IRQ_1 = SERCOM5_1_IRQn;
        static constexpr IRQn_Type IRQ_2 = SERCOM5_2_IRQn;
        static constexpr IRQn_Type IRQ_3 = SERCOM5_3_IRQn;
        static constexpr uint8_t gclkId = SERCOM5_GCLK_ID_CORE;
    };

    /**
     * SERCOM Hardware wrapper for USART mode
     * Implements the interface expected by MotateUART.h UART<> template
     */
    template<uint8_t sercomNum>
    struct _SERCOMHardware {
        using Info = SERCOMInfo<sercomNum>;

        static SercomUsart* usart() {
            return &(Info::sercom()->USART);
        }

        static constexpr uint8_t sercomNumber = sercomNum;

        // Interrupt handler callback
        std::function<void(uint16_t)> _uartInterruptHandler;

        // Static interrupt handler jumper for C-linkage interrupt handlers
        static std::function<void()> _uartInterruptHandlerJumper;

        bool _inited = false;
        uint32_t _baudRate = 115200;
        uint8_t _rxPad = 0;
        uint8_t _txPad = 0;

        _SERCOMHardware() {}

        /**
         * Enable SERCOM clock via MCLK
         */
        void _enableClock() {
            if constexpr (sercomNum == 0) {
                MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM0;
            } else if constexpr (sercomNum == 1) {
                MCLK->APBAMASK.reg |= MCLK_APBAMASK_SERCOM1;
            } else if constexpr (sercomNum == 2) {
                MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM2;
            } else if constexpr (sercomNum == 3) {
                MCLK->APBBMASK.reg |= MCLK_APBBMASK_SERCOM3;
            } else if constexpr (sercomNum == 4) {
                MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM4;
            } else if constexpr (sercomNum == 5) {
                MCLK->APBDMASK.reg |= MCLK_APBDMASK_SERCOM5;
            }

            // Configure GCLK for this SERCOM (use GCLK0 = 120MHz)
            GCLK->PCHCTRL[Info::gclkId].reg = GCLK_PCHCTRL_GEN_GCLK0 | GCLK_PCHCTRL_CHEN;
            while (GCLK->PCHCTRL[Info::gclkId].bit.CHEN == 0);
        }

        /**
         * Initialize SERCOM for USART operation
         * Called from UART<> constructor - does basic setup
         */
        void init() {
            _enableClock();

            // Reset SERCOM
            usart()->CTRLA.bit.SWRST = 1;
            while (usart()->SYNCBUSY.bit.SWRST);

            // Disable all interrupts
            usart()->INTENCLR.reg = 0xFF;

            // Set up interrupt handler jumper
            _uartInterruptHandlerJumper = [this]() {
                if (_uartInterruptHandler) {
                    uint16_t interruptCause = 0;
                    if (usart()->INTFLAG.bit.RXC && usart()->INTENSET.bit.RXC) {
                        interruptCause |= (1 << 0);  // OnRxReady
                    }
                    if (usart()->INTFLAG.bit.DRE && usart()->INTENSET.bit.DRE) {
                        interruptCause |= (1 << 1);  // OnTxReady
                    }
                    if (usart()->INTFLAG.bit.TXC && usart()->INTENSET.bit.TXC) {
                        interruptCause |= (1 << 2);  // OnTxDone
                    }
                    _uartInterruptHandler(interruptCause);
                }
            };

            _inited = true;
        }

        /**
         * Enable TX and RX
         */
        void enable() {
            usart()->CTRLB.reg |= SERCOM_USART_CTRLB_RXEN | SERCOM_USART_CTRLB_TXEN;
            while (usart()->SYNCBUSY.bit.CTRLB);
            usart()->CTRLA.bit.ENABLE = 1;
            while (usart()->SYNCBUSY.bit.ENABLE);
        }

        /**
         * Disable TX and RX
         */
        void disable() {
            usart()->CTRLA.bit.ENABLE = 0;
            while (usart()->SYNCBUSY.bit.ENABLE);
        }

        /**
         * Set UART options (baud rate, parity, stop bits, etc.)
         * Called from UART<> constructor and setOptions()
         */
        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor = false) {
            _baudRate = baud;

            // Disable while configuring
            disable();

            // Configure USART mode
            uint32_t ctrla = SERCOM_USART_CTRLA_MODE(1) |  // Internal clock USART
                             SERCOM_USART_CTRLA_DORD |      // LSB first
                             SERCOM_USART_CTRLA_SAMPR(0);   // 16x oversampling

            // Set RX and TX pads - these should be set by the pin configuration
            // For now use defaults that can be overridden
            ctrla |= SERCOM_USART_CTRLA_RXPO(_rxPad);
            ctrla |= SERCOM_USART_CTRLA_TXPO(_txPad);

            usart()->CTRLA.reg = ctrla;

            // Configure CTRLB
            uint32_t ctrlb = 0;

            // Character size (default 8-bit)
            // UARTMode::As9Bit = 1 << 3
            if (options & (1 << 3)) {
                ctrlb |= SERCOM_USART_CTRLB_CHSIZE(1);  // 9-bit
            } else {
                ctrlb |= SERCOM_USART_CTRLB_CHSIZE(0);  // 8-bit
            }

            // Stop bits
            // UARTMode::TwoStopBits = 1 << 2
            if (options & (1 << 2)) {
                ctrlb |= SERCOM_USART_CTRLB_SBMODE;  // 2 stop bits
            }

            // Parity
            // UARTMode::EvenParity = 1 << 0, OddParity = 1 << 1
            if (options & (1 << 0)) {
                // Even parity
                usart()->CTRLA.reg |= SERCOM_USART_CTRLA_FORM(1);  // USART with parity
                ctrlb &= ~SERCOM_USART_CTRLB_PMODE;  // Even
            } else if (options & (1 << 1)) {
                // Odd parity
                usart()->CTRLA.reg |= SERCOM_USART_CTRLA_FORM(1);  // USART with parity
                ctrlb |= SERCOM_USART_CTRLB_PMODE;  // Odd
            }
            // else no parity (FORM = 0, which is default)

            usart()->CTRLB.reg = ctrlb;
            while (usart()->SYNCBUSY.bit.CTRLB);

            // Calculate baud rate (16x oversampling, arithmetic mode)
            // BAUD = 65536 * (1 - 16 * (f_baud / f_ref))
            // f_ref = 120MHz (GCLK0)
            uint64_t br = (uint64_t)65536 * (120000000 - 16 * baud) / 120000000;
            usart()->BAUD.reg = (uint16_t)br;

            // Re-enable
            enable();
        }

        /**
         * Set the interrupt handler callback
         */
        void setInterruptHandler(std::function<void(uint16_t)> &&handler) {
            _uartInterruptHandler = std::move(handler);
        }

        /**
         * Configure interrupts and priority
         */
        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != 0) {  // Interrupt::Off = 0
                // Set priority based on interrupt flags
                uint8_t priority = 3;  // Default medium-low

                if (interrupts & (1 << 8)) {  // PriorityHighest
                    priority = 0;
                } else if (interrupts & (1 << 9)) {  // PriorityHigh
                    priority = 1;
                } else if (interrupts & (1 << 10)) {  // PriorityMedium
                    priority = 2;
                } else if (interrupts & (1 << 11)) {  // PriorityLow
                    priority = 3;
                }

                // SAMD51 has 4 interrupt vectors per SERCOM
                NVIC_SetPriority(Info::IRQ_0, priority);
                NVIC_SetPriority(Info::IRQ_1, priority);
                NVIC_SetPriority(Info::IRQ_2, priority);
                NVIC_SetPriority(Info::IRQ_3, priority);

                NVIC_EnableIRQ(Info::IRQ_0);
                NVIC_EnableIRQ(Info::IRQ_1);
                NVIC_EnableIRQ(Info::IRQ_2);
                NVIC_EnableIRQ(Info::IRQ_3);
            } else {
                NVIC_DisableIRQ(Info::IRQ_0);
                NVIC_DisableIRQ(Info::IRQ_1);
                NVIC_DisableIRQ(Info::IRQ_2);
                NVIC_DisableIRQ(Info::IRQ_3);
            }
        }

        /**
         * Enable/disable RX ready interrupt
         */
        void setInterruptRxReady(bool value) {
            if (value) {
                usart()->INTENSET.reg = SERCOM_USART_INTENSET_RXC;
            } else {
                usart()->INTENCLR.reg = SERCOM_USART_INTENCLR_RXC;
            }
        }

        /**
         * Enable/disable TX ready interrupt
         */
        void setInterruptTxReady(bool value) {
            if (value) {
                usart()->INTENSET.reg = SERCOM_USART_INTENSET_DRE;
            } else {
                usart()->INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
            }
        }

        /**
         * Enable/disable TX complete interrupt
         */
        void setInterruptTxTransferDone(bool value) {
            if (value) {
                usart()->INTENSET.reg = SERCOM_USART_INTENSET_TXC;
            } else {
                usart()->INTENCLR.reg = SERCOM_USART_INTENCLR_TXC;
            }
        }

        /**
         * Enable/disable RX transfer done interrupt (for DMA, stub for now)
         */
        void setInterruptRxTransferDone(bool value) {
            // DMA not implemented yet - just enable/disable RXC
            setInterruptRxReady(value);
        }

        /**
         * Enable/disable CTS change interrupt
         * SAMD51 SERCOM doesn't have hardware CTS, so this is a stub
         */
        void _setInterruptCTSChange(bool value) {
            (void)value;
            // No hardware CTS on SAMD51 SERCOM USART
            // Software flow control is handled by GPIO pins
        }

        /**
         * Pause TX (for flow control)
         */
        void pauseTX() {
            // Disable DRE interrupt to stop transmitting
            usart()->INTENCLR.reg = SERCOM_USART_INTENCLR_DRE;
        }

        /**
         * Resume TX (for flow control)
         */
        void resumeTX() {
            // Re-enable DRE interrupt to continue transmitting
            usart()->INTENSET.reg = SERCOM_USART_INTENSET_DRE;
        }

        /**
         * Check if connected (for software flow control)
         * Returns true if we're allowed to send
         */
        bool isConnected() {
            return true;  // Always connected for now (no hardware flow control)
        }

        /**
         * Read a byte from the UART
         * Returns -1 if no data available
         */
        int16_t readByte() {
            if (usart()->INTFLAG.bit.RXC) {
                g_uart_rx_count++;
                return usart()->DATA.reg;
            }
            return -1;
        }

        /**
         * Write a byte to the UART
         * Returns 1 if written, 0 if buffer full
         */
        int16_t writeByte(uint8_t data) {
            if (usart()->INTFLAG.bit.DRE) {
                g_uart_tx_count++;
                usart()->DATA.reg = data;
                return 1;
            }
            return 0;
        }

        /**
         * Wait for all TX data to be sent
         */
        void flush() {
            while (!usart()->INTFLAG.bit.TXC);
        }

        /**
         * Discard any data in RX buffer
         */
        void flushRead() {
            while (usart()->INTFLAG.bit.RXC) {
                (void)usart()->DATA.reg;  // Discard
            }
        }

        /**
         * Check if RX data is available
         */
        bool rxReady() const {
            return usart()->INTFLAG.bit.RXC;
        }

        /**
         * Check if TX buffer is ready
         */
        bool txReady() const {
            return usart()->INTFLAG.bit.DRE;
        }

        /**
         * Set RX and TX pad numbers (called before init)
         */
        void setPads(uint8_t rxPad, uint8_t txPad) {
            _rxPad = rxPad;
            _txPad = txPad;
        }

        /***************************************************************************
         * DMA Transfer Methods (stubs for now - polled operation)
         * TODO: Implement proper DMA support using SAMD51 DMAC
         ***************************************************************************/

        /**
         * Start a DMA RX transfer
         * For now, returns false to indicate DMA not available
         */
        bool startRXTransfer(char* buffer, uint16_t length) {
            (void)buffer;
            (void)length;
            // DMA not implemented - will use polled/interrupt mode
            return true;  // Return true to avoid error, but no actual DMA
        }

        /**
         * Start a DMA TX transfer
         * For now, returns false to indicate DMA not available
         */
        bool startTXTransfer(char* buffer, uint16_t length) {
            (void)buffer;
            (void)length;
            // DMA not implemented - will use polled/interrupt mode
            return true;  // Return true to avoid error, but no actual DMA
        }

        /**
         * Get current RX DMA position
         * Returns nullptr when DMA not active
         */
        char* getRXTransferPosition() {
            return nullptr;
        }

        /**
         * Get current TX DMA position
         * Returns nullptr when DMA not active
         */
        char* getTXTransferPosition() {
            return nullptr;
        }
    };

    // Static member definition
    template<uint8_t sercomNum>
    std::function<void()> _SERCOMHardware<sercomNum>::_uartInterruptHandlerJumper;

    // Hardware instance pointers for interrupt handlers
    extern _SERCOMHardware<0>* _sercom0_hw;
    extern _SERCOMHardware<1>* _sercom1_hw;
    extern _SERCOMHardware<2>* _sercom2_hw;
    extern _SERCOMHardware<3>* _sercom3_hw;
    extern _SERCOMHardware<4>* _sercom4_hw;
    extern _SERCOMHardware<5>* _sercom5_hw;

    // Note: Pin-related templates (isRealAndCorrectRTSPin, isRealAndCorrectCTSPin,
    // UARTGetHardware, UARTRxPin, UARTTxPin) are defined in ProcessorUART.h
    // which is included from MotateUART.h after MotatePins.h provides pin_number.

} // namespace Motate

#endif // SAMD_UART_H_ONCE
