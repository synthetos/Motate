/*
 * samd51_gpio_demo.cpp - SAMD51 GPIO Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 1: Basic Boot & GPIO
 *
 * Purpose:
 *   - Verify basic GPIO output functionality
 *   - Measure toggle rate for ISR-speed GPIO operations
 *   - Confirm board boots and runs correctly
 *
 * Test Setup:
 *   - Connect Saleae Logic Analyzer to D12 (PA17)
 *   - LED on D13 should blink at ~1Hz
 *
 * Pass Criteria:
 *   - LED toggles at 1Hz (visible)
 *   - Debug pin toggle rate > 1 MHz (proves ISR-speed GPIO)
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D13/PA16 - Built-in LED (slow blink indicator)
 *   D12/PA17 - Debug output (fast toggle for timing measurement)
 *   D11/PA19 - Additional debug output (optional)
 *   D10/PA18 - Additional debug output (optional)
 */

#include "MotatePins.h"
#include "MotateTimers.h"

// Use Motate namespace for cleaner code
using namespace Motate;

/****** Pin Definitions ******/

// Pin numbers from motate_pin_assignments.h
constexpr pin_number kLED_PinNumber = 13;     // D13/PA16 - Built-in LED
constexpr pin_number kDebug1_PinNumber = 12;  // D12/PA17 - Primary debug output
constexpr pin_number kDebug2_PinNumber = 11;  // D11/PA19 - Secondary debug output
constexpr pin_number kDebug3_PinNumber = 10;  // D10/PA18 - Tertiary debug output

/****** Global Pin Objects ******/

OutputPin<kLED_PinNumber> led_pin;
OutputPin<kDebug1_PinNumber> debug1_pin;
OutputPin<kDebug2_PinNumber> debug2_pin;
OutputPin<kDebug3_PinNumber> debug3_pin;

/****** Test Configuration ******/

// Number of fast toggles per LED blink cycle
constexpr uint32_t kFastToggleCount = 10000;

// Delay between LED blinks (ms)
constexpr uint32_t kBlinkDelayMs = 500;

/****** Optional setup() function ******/

void setup() {
    // Initialize all pins as outputs, initially low
    led_pin = 0;
    debug1_pin = 0;
    debug2_pin = 0;
    debug3_pin = 0;

    // Brief startup indication - rapid blinks
    for (int i = 0; i < 5; i++) {
        led_pin = 1;
        delay(50);
        led_pin = 0;
        delay(50);
    }
}

/****** Main run loop() ******/

void loop() {
    // Toggle LED for visibility
    led_pin.toggle();

    // Fast toggle test on debug1_pin
    // This measures raw GPIO toggle speed
    // At 120 MHz, expect >10 MHz toggle rate with direct register access
    // Through Motate abstraction, expect >1 MHz

    // Marker pulse on debug2 to indicate start of fast toggle burst
    debug2_pin.set();

    for (uint32_t i = 0; i < kFastToggleCount; i++) {
        debug1_pin.set();
        debug1_pin.clear();
    }

    // Marker pulse end
    debug2_pin.clear();

    // Test: Toggle all debug pins in sequence
    // This pattern helps verify all pins are correctly mapped
    debug3_pin.set();
    debug1_pin.set();
    debug2_pin.set();
    debug3_pin.clear();
    debug1_pin.clear();
    debug2_pin.clear();

    delay(kBlinkDelayMs);
}

/*
 * Expected Saleae Measurements:
 *
 * debug1_pin (D12):
 *   - Should see 10000 high/low cycles in a burst
 *   - Measure frequency of toggle: target > 1 MHz
 *   - High pulse width: target < 1us
 *
 * debug2_pin (D11):
 *   - Goes high at start of fast toggle burst
 *   - Goes low at end of fast toggle burst
 *   - Duration indicates total time for 10000 toggles
 *
 * LED (D13):
 *   - Should toggle every 500ms (1 Hz blink rate)
 *
 * Calculation:
 *   If 10000 toggles complete in X microseconds:
 *   Toggle frequency = 10000 / (X * 1e-6) Hz
 *
 *   For ISR-compatible GPIO, we need:
 *   - Step pulse generation at 200 kHz (5 us period)
 *   - Need GPIO toggle + ISR overhead < 2 us
 *   - So GPIO toggle alone should be < 100 ns (>10 MHz)
 */
