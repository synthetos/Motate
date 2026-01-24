/*
 * samd51_uart_demo.cpp - SAMD51 UART Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 5: UART Communication
 *
 * Purpose:
 *   - Verify SERCOM USART peripheral operation
 *   - Test 115200 baud serial communication
 *   - Verify bit timing with Saleae
 *
 * Test Setup:
 *   - Connect USB-Serial adapter to D0 (RX) and D1 (TX)
 *   - Connect Saleae to D0 and D1 for timing verification
 *   - Open terminal at 115200 8N1
 *
 * Pass Criteria:
 *   - Bit timing: 8.68us +/-2% (115200 baud)
 *   - Clean start/stop bits
 *   - Echo works at sustained rate
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D0/PA23 - UART RX (SERCOM3/PAD[1])
 *   D1/PA22 - UART TX (SERCOM3/PAD[0])
 *   D13/PA16 - Built-in LED (activity indicator)
 *   D12/PA17 - Debug output (RX activity pulse)
 */

#include "MotatePins.h"
#include "MotateTimers.h"
#include "SamdUART.h"
#include <cstdio>

using namespace Motate;

// Define UARTMode locally since the full MotateUART.h has compatibility issues
struct UARTMode {
    static constexpr uint16_t NoParity    = 0;
    static constexpr uint16_t EvenParity  = 1 << 0;
    static constexpr uint16_t OddParity   = 1 << 1;
    static constexpr uint16_t OneStopBit  = 0;
    static constexpr uint16_t TwoStopBits = 1 << 2;
    static constexpr uint16_t As8Bit      = 0;
    static constexpr uint16_t As9Bit      = 1 << 3;
    static constexpr uint16_t As8N1       = As8Bit | NoParity | OneStopBit;
};

/****** Pin Definitions ******/

constexpr pin_number kLED_PinNumber = 13;     // D13/PA16 - Activity LED
constexpr pin_number kDebug_PinNumber = 12;   // D12/PA17 - Debug output
constexpr pin_number kRX_PinNumber = 0;       // D0/PA23 - UART RX
constexpr pin_number kTX_PinNumber = 1;       // D1/PA22 - UART TX

/****** Global Pin Objects ******/

OutputPin<kLED_PinNumber> led_pin;
OutputPin<kDebug_PinNumber> debug_pin;

// UART TX and RX pins need to be configured for SERCOM alternate function
// PA22 = SERCOM3/PAD[0] = TX (function C)
// PA23 = SERCOM3/PAD[1] = RX (function C)

/****** UART Hardware ******/

// Use SERCOM3 for standard serial header (D0/D1)
_SERCOMHardware<3> uart_hw;

/****** Configuration ******/

constexpr uint32_t kBaudRate = 115200;

/****** Statistics ******/

volatile uint32_t rx_count = 0;
volatile uint32_t tx_count = 0;
volatile uint32_t echo_count = 0;

/****** Helper: Configure pin for SERCOM alternate function ******/

void configureSercomPins() {
    // PA22 (D1/TX) = SERCOM3 PAD[0], function C (peripheral index 2)
    // PA23 (D0/RX) = SERCOM3 PAD[1], function C (peripheral index 2)

    // Get PORT module
    PORT->Group[0].DIRSET.reg = (1 << 22);  // PA22 as output (TX)
    PORT->Group[0].DIRCLR.reg = (1 << 23);  // PA23 as input (RX)

    // Configure PA22 for SERCOM3 - PMUX function C (index 2)
    PORT->Group[0].PINCFG[22].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[22/2].bit.PMUXE = 2;  // Function C for even pin (PA22)

    // Configure PA23 for SERCOM3 - PMUX function C (index 2)
    PORT->Group[0].PINCFG[23].bit.PMUXEN = 1;
    PORT->Group[0].PMUX[23/2].bit.PMUXO = 2;  // Function C for odd pin (PA23)

    // Enable input (for RX)
    PORT->Group[0].PINCFG[23].bit.INEN = 1;
}

/****** Optional setup() function ******/

void setup() {
    // Initialize debug pins
    led_pin = 0;
    debug_pin = 0;

    // Startup LED flash
    for (int i = 0; i < 3; i++) {
        led_pin = 1;
        delay(100);
        led_pin = 0;
        delay(100);
    }

    // Configure SERCOM pins
    configureSercomPins();

    // Set pad configuration for SERCOM3
    // RX on PAD[1], TX on PAD[0] (TXPO = 0)
    uart_hw.setPads(1, 0);  // rxPad=1, txPad=0

    // Initialize UART hardware
    uart_hw.init();

    // Set options: 115200 8N1
    uart_hw.setOptions(kBaudRate, UARTMode::As8N1, true);

    // Send startup message
    const char* msg = "\r\n=== SAMD51 UART Demo ===\r\n"
                      "115200 8N1 Echo Test\r\n"
                      "Type characters to echo.\r\n"
                      ">\r\n";
    for (const char* p = msg; *p; p++) {
        while (!uart_hw.txReady());  // Wait for TX buffer
        uart_hw.writeByte(*p);
        tx_count++;
    }
    uart_hw.flush();
}

/****** Main run loop() ******/

void loop() {
    // Check for received data
    if (uart_hw.rxReady()) {
        debug_pin.set();  // RX activity pulse

        // Read byte
        int16_t c = uart_hw.readByte();
        if (c >= 0) {
            rx_count++;

            // Handle special characters
            if (c == '\r') {
                // Echo CR+LF
                while (!uart_hw.txReady());
                uart_hw.writeByte('\r');
                while (!uart_hw.txReady());
                uart_hw.writeByte('\n');
                tx_count += 2;
            }
            else if (c == '?') {
                // Print stats
                char stats[80];
                int len = snprintf(stats, sizeof(stats),
                    "\r\nStats: rx=%lu tx=%lu echo=%lu\r\n> ",
                    rx_count, tx_count, echo_count);
                for (int i = 0; i < len; i++) {
                    while (!uart_hw.txReady());
                    uart_hw.writeByte(stats[i]);
                    tx_count++;
                }
            }
            else {
                // Echo character back
                while (!uart_hw.txReady());
                uart_hw.writeByte((uint8_t)c);
                tx_count++;
                echo_count++;
            }

            // Toggle LED for activity
            led_pin.toggle();
        }

        debug_pin.clear();
    }
}

/*
 * Expected Behavior:
 *
 * Hardware Setup:
 *   1. Connect USB-Serial adapter (FTDI, CP2102, etc.) to Metro M4:
 *      - Adapter TX -> Metro D0 (RX)
 *      - Adapter RX -> Metro D1 (TX)
 *      - Adapter GND -> Metro GND
 *   2. Do NOT connect adapter's VCC - Metro is powered via USB
 *
 *   3. For Saleae timing verification:
 *      - Channel 0 -> D1 (TX line)
 *      - Channel 1 -> D0 (RX line)
 *      - Channel 2 -> D12 (debug pulse)
 *
 * Software Test:
 *   1. Open terminal at 115200 8N1 (PuTTY, screen, minicom)
 *   2. Should see startup message
 *   3. Type characters - they echo back
 *   4. Press '?' to see statistics
 *
 * Saleae Measurements:
 *   - Capture TX line during transmission
 *   - Measure bit width: should be 8.68us (1/115200)
 *   - Verify start bit (low), 8 data bits, stop bit (high)
 *   - Tolerance: +/- 2% (8.50us - 8.86us)
 *
 * Baud Rate Calculation:
 *   At 115200 baud, bit period = 1/115200 = 8.68us
 *   With 120MHz clock and 16x oversampling:
 *   BAUD = 65536 * (1 - 16 * 115200 / 120000000)
 *        = 65536 * (1 - 0.01536)
 *        = 65536 * 0.98464
 *        = 64529
 *
 *   Actual baud = 120000000 / (16 * (65536 - 64529) / 65536)
 *               = 120000000 / (16 * 1007 / 65536)
 *               = 115207 (within 0.006% of target)
 *
 * Debug Counters (via J-Link):
 *   - rx_count: Total bytes received
 *   - tx_count: Total bytes transmitted
 *   - echo_count: Characters echoed
 *   - g_uart_rx_count: Hardware driver RX count
 *   - g_uart_tx_count: Hardware driver TX count
 */
