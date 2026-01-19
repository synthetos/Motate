/*
 * SamdUART.cpp - UART implementation for SAMD51
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
 */

#include "SamdUART.h"

namespace Motate {

    // Debug counters
    volatile uint32_t g_uart_rx_count = 0;
    volatile uint32_t g_uart_tx_count = 0;
    volatile uint32_t g_uart_rx_overrun = 0;

    // Hardware instance pointers for interrupt handlers
    _SERCOMHardware<0>* _sercom0_hw = nullptr;
    _SERCOMHardware<1>* _sercom1_hw = nullptr;
    _SERCOMHardware<2>* _sercom2_hw = nullptr;
    _SERCOMHardware<3>* _sercom3_hw = nullptr;
    _SERCOMHardware<4>* _sercom4_hw = nullptr;
    _SERCOMHardware<5>* _sercom5_hw = nullptr;

} // namespace Motate

// Note: SERCOM interrupt handlers are shared with SPI/I2C
// The handlers check which mode is active and dispatch accordingly.
// For UART-only SERCOM instances, we can add handlers here.
// However, on SAMD51 each SERCOM has 4 interrupt vectors (0-3).
// These are typically handled in a common handler file.
//
// For now, we rely on polling in board code or the existing
// interrupt infrastructure. If dedicated handlers are needed,
// they can be added in cortex_handlers.cpp with weak symbols.
