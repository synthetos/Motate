/*
 * samd51_timer_basic_demo.cpp - SAMD51 Basic Timer Interrupt Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 2: Single TC Timer Interrupt
 *
 * Purpose:
 *   - Verify TC timer peripheral initialization
 *   - Test interrupt generation at 200kHz (DDA rate)
 *   - Measure ISR execution time and jitter
 *
 * Test Setup:
 *   - Connect Saleae Logic Analyzer to D12 (PA17)
 *   - Capture at >= 10 MHz sample rate
 *
 * Pass Criteria:
 *   - Period = 5.00us +/-0.5% (200kHz)
 *   - Pulse width < 2us (ISR execution time)
 *   - Jitter < 100ns
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D13/PA16 - Built-in LED (heartbeat indicator)
 *   D12/PA17 - ISR toggle output (primary timing measurement)
 */

#include "MotatePins.h"
#include "MotateTimers.h"

using namespace Motate;

/****** Pin Definitions ******/

constexpr pin_number kLED_PinNumber = 13;     // D13/PA16 - Built-in LED
constexpr pin_number kISR_PinNumber = 12;     // D12/PA17 - ISR toggle output

/****** Global Pin Objects ******/

OutputPin<kLED_PinNumber> led_pin;
OutputPin<kISR_PinNumber> isr_pin;

/****** Timer Configuration ******/

// Use TC3 for DDA timer (same as g2core stepper.cpp)
// 200kHz = 5us period
constexpr uint32_t kDDAFrequency = 200000;

// Create timer channel object
TimerChannel<3, 0> dda_timer(kTimerUpToMatch, kDDAFrequency);

/****** ISR Statistics ******/

volatile uint32_t isr_count = 0;
volatile uint32_t led_toggle_counter = 0;

// Toggle LED every N interrupts (200000 = once per second at 200kHz)
constexpr uint32_t kLEDToggleInterval = 200000;

/****** Timer Interrupt Handler ******/

/*
 * Timer interrupt for TC3.
 *
 * This must be a template specialization of TimerChannel<3, 0>::interrupt()
 * The weak symbol in SamdTimers.cpp allows this user-defined function to
 * override the default empty implementation.
 *
 * At 200kHz, this ISR fires every 5 microseconds.
 * The ISR should complete in under 2 microseconds to leave headroom.
 */
namespace Motate {
    template<> void TimerChannel<3, 0>::interrupt() {
        // Set pin HIGH at ISR entry
        isr_pin.set();

        // Simulate minimal DDA work:
        // - Increment counter
        // - Check 6 axes (simulated with volatile read)
        isr_count++;

        // Simulated axis checks (similar to g2core stepper ISR)
        volatile uint32_t dummy = 0;
        for (int axis = 0; axis < 6; axis++) {
            dummy += isr_count;  // Simulated step check
        }
        (void)dummy;  // Suppress unused warning

        // LED heartbeat toggle
        led_toggle_counter++;
        if (led_toggle_counter >= kLEDToggleInterval) {
            led_toggle_counter = 0;
            led_pin.toggle();
        }

        // Set pin LOW at ISR exit
        isr_pin.clear();
    }
}

/****** Optional setup() function ******/

void setup() {
    // Initialize pins
    led_pin = 0;
    isr_pin = 0;

    // Startup indication
    for (int i = 0; i < 3; i++) {
        led_pin = 1;
        delay(100);
        led_pin = 0;
        delay(100);
    }

    // Configure timer interrupts
    // kInterruptOnOverflow triggers when counter reaches CC[0] in MFRQ mode
    // kInterruptPriorityHighest gives NVIC priority 0
    dda_timer.setInterrupts(kInterruptOnOverflow | kInterruptPriorityHighest);

    // Start the timer
    dda_timer.start();
}

/****** Main run loop() ******/

void loop() {
    // Main loop does nothing - all work is in ISR
    // This simulates g2core's pattern where motion is ISR-driven

    // The LED will toggle once per second as a heartbeat indicator
    // from within the ISR to prove the timer is running

    delay(1000);
}

/*
 * Expected Saleae Measurements:
 *
 * isr_pin (D12):
 *   - Should see 200kHz square wave
 *   - HIGH time = ISR execution duration (target < 2us)
 *   - LOW time = time between ISR completions
 *   - Total period = 5.00us +/- 0.5% (4.975us - 5.025us)
 *
 * Measurement Procedure:
 *   1. Set sample rate to 10 MHz or higher
 *   2. Capture several milliseconds of data
 *   3. Use frequency measurement on D12 - should read ~200kHz
 *   4. Measure HIGH pulse width - should be < 2us
 *   5. Check period consistency - look for jitter
 *
 * If ISR takes too long:
 *   - HIGH pulse will be > 2us
 *   - May see missed interrupts (period > 5us)
 *   - LED will blink slower than 1Hz
 *
 * Debugging with J-Link:
 *   - Read g_tc3_handler_count for total ISR invocations
 *   - Read isr_count for successful ISR completions
 *   - Compare to expected count (200000 per second)
 */
