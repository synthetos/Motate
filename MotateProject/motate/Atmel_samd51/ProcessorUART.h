/*
 * ProcessorUART.h - UART definitions for SAMD51 (SERCOM USART)
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
 *
 * This file is part of the Motate Library.
 *
 * SAMD51 SERCOM Pin Assignments:
 *
 * SERCOM0:
 *   PA04 = PAD0 (mux D), PA05 = PAD1 (mux D), PA06 = PAD2 (mux D), PA07 = PAD3 (mux D)
 *   PA08 = PAD0 (mux C), PA09 = PAD1 (mux C), PA10 = PAD2 (mux C), PA11 = PAD3 (mux C)
 *
 * SERCOM1:
 *   PA00 = PAD0 (mux D), PA01 = PAD1 (mux D), PA30 = PAD2 (mux D), PA31 = PAD3 (mux D)
 *   PA16 = PAD0 (mux C), PA17 = PAD1 (mux C), PA18 = PAD2 (mux C), PA19 = PAD3 (mux C)
 *
 * For USART mode:
 *   RXPO: 0-3 selects which pad is RX
 *   TXPO: 0=PAD0 is TX, 1=PAD2 is TX, 2=PAD0/RTS/CTS, 3=PAD0/XCK
 *
 * SERCOM0 configuration that works:
 *   RXPO = 3 (RX on PAD3 = PA07)
 *   TXPO = 0 (TX on PAD0 = PA04)
 */

#ifndef SAMD51_PROCESSORUART_H_ONCE
#define SAMD51_PROCESSORUART_H_ONCE

#include "SamdUART.h"

namespace Motate {

    /***************************************************************************
     * UART Pin Definitions for Metro M4 AirLift (NINA Communication)
     *
     * For ESP32 NINA communication, we use:
     * - Pin 9 (PA07) = SERCOM0 PAD3 - SAMD51 RX (from ESP32 TX)
     * - Pin 18 (PA04) = SERCOM0 PAD0 - SAMD51 TX (to ESP32 RX)
     *
     * SERCOM0 configuration:
     *   RXPO = 3 (RX on PAD3 = PA07)
     *   TXPO = 0 (TX on PAD0 = PA04)
     ***************************************************************************/

    // Pin 9 (PA07) as UART RX - SERCOM0 PAD3
    // Use mux D for SERCOM0 alternate
    template<> struct UARTRxPin<9> : Pin<9> {
        static constexpr bool is_real = true;
        static constexpr int8_t uartNum = 0;
        static constexpr uint8_t rxPad = 3;  // PAD3
        UARTRxPin() : Pin<9>(kPeripheralD) {}  // Mux D for SERCOM0
    };

    // Pin 18 (PA04) as UART TX - SERCOM0 PAD0
    // Use mux D for SERCOM0 alternate
    template<> struct UARTTxPin<18> : Pin<18> {
        static constexpr bool is_real = true;
        static constexpr int8_t uartNum = 0;
        static constexpr uint8_t txPad = 0;  // TXPO=0 means TX on PAD0
        UARTTxPin() : Pin<18>(kPeripheralD) {}  // Mux D for SERCOM0
    };

    /***************************************************************************
     * Helper templates required by MotateUART.h
     ***************************************************************************/

    /**
     * Check if RTS pin is valid for hardware flow control
     * SAMD51 SERCOM USART doesn't have dedicated RTS/CTS hardware
     */
    template<pin_number rtsPinNumber, pin_number rxPinNumber>
    constexpr const bool isRealAndCorrectRTSPin() {
        return false;  // No hardware RTS on SAMD51 SERCOM USART
    }

    /**
     * Check if CTS pin is valid for hardware flow control
     * SAMD51 SERCOM USART doesn't have dedicated RTS/CTS hardware
     */
    template<pin_number ctsPinNumber, pin_number rxPinNumber>
    constexpr const bool isRealAndCorrectCTSPin() {
        return false;  // No hardware CTS on SAMD51 SERCOM USART
    }

    /**
     * SERCOM UART Hardware wrapper that configures pads from pin info
     * This wraps _SERCOMHardware and sets up the correct RXPO/TXPO
     */
    template<uint8_t sercomNum, uint8_t rxPad, uint8_t txPad>
    struct _SERCOMHardwareWithPads : _SERCOMHardware<sercomNum> {
        using Base = _SERCOMHardware<sercomNum>;

        _SERCOMHardwareWithPads() {
            // Set the pads before init() is called
            Base::_rxPad = rxPad;
            Base::_txPad = txPad;
        }
    };

    /**
     * Get UART hardware type from pin numbers
     * Uses the pad info from UARTRxPin and UARTTxPin to configure SERCOM
     */
    template<pin_number rxPinNumber, pin_number txPinNumber>
    using UARTGetHardware = _SERCOMHardwareWithPads<
        UARTRxPin<rxPinNumber>::uartNum,
        UARTRxPin<rxPinNumber>::rxPad,
        UARTTxPin<txPinNumber>::txPad
    >;

} // namespace Motate

#endif // SAMD51_PROCESSORUART_H_ONCE
