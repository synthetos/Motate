/*
 * samd51_timer_hierarchy_demo.cpp - SAMD51 Timer Hierarchy Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 3: Multi-Timer with Software Triggers (CRITICAL TEST)
 *
 * Purpose:
 *   - Verify 3-level interrupt hierarchy matching g2core
 *   - Test software-triggered interrupts via setInterruptPending()
 *   - Confirm interrupt preemption works correctly
 *   - Validate that DDA timer continues during lower-priority ISRs
 *
 * This test replicates g2core's interrupt architecture:
 *   - TC3 (200kHz): DDA timer - highest priority, hardware-triggered
 *   - TC4: Exec timer - high priority, software-triggered from DDA
 *   - TC5: Planning timer - medium priority, software-triggered from Exec
 *
 * Test Setup:
 *   - Connect Saleae Logic Analyzer to D12, D11, D10
 *   - Capture at >= 10 MHz sample rate
 *
 * Pass Criteria:
 *   - DDA (D12): 200kHz, <2us pulse, NO missed interrupts
 *   - Exec (D11): ~1kHz (every 200 DDA ticks), ~100us pulse
 *   - Plan (D10): Fires after each exec
 *   - DDA continues during Exec/Plan (preemption works)
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D13/PA16 - Built-in LED (heartbeat)
 *   D12/PA17 - DDA ISR toggle (highest priority)
 *   D11/PA19 - Exec ISR toggle (high priority)
 *   D10/PA18 - Plan ISR toggle (medium priority)
 */

#include "MotatePins.h"
#include "MotateTimers.h"

using namespace Motate;

/****** Pin Definitions ******/

constexpr pin_number kLED_PinNumber = 13;     // D13/PA16 - Heartbeat LED
constexpr pin_number kDDA_PinNumber = 12;     // D12/PA17 - DDA ISR output
constexpr pin_number kExec_PinNumber = 11;    // D11/PA19 - Exec ISR output
constexpr pin_number kPlan_PinNumber = 10;    // D10/PA18 - Plan ISR output

/****** Global Pin Objects ******/

OutputPin<kLED_PinNumber> led_pin;
OutputPin<kDDA_PinNumber> dda_pin;
OutputPin<kExec_PinNumber> exec_pin;
OutputPin<kPlan_PinNumber> plan_pin;

/****** Timer Configuration ******/

// DDA Timer: TC3 at 200kHz (hardware triggered)
// CRITICAL: ISR must complete in <2µs for reliable operation with
// lower-priority software-triggered interrupts. The original 3.2µs ISR
// caused priority starvation where TC4/TC5 never completed.
constexpr uint32_t kDDAFrequency = 200000;
TimerChannel<3, 0> dda_timer(kTimerUpToMatch, kDDAFrequency);

// Exec Timer: TC4 (software triggered only)
// Initialize with low frequency - actual triggering is via software
Timer<4> exec_timer;

// Plan Timer: TC5 (software triggered only)
Timer<5> plan_timer;

/****** ISR Statistics ******/

volatile uint32_t dda_count = 0;
volatile uint32_t exec_count = 0;
volatile uint32_t plan_count = 0;
volatile uint32_t dda_during_exec = 0;  // DDA interrupts that fired during exec
volatile uint32_t dda_during_plan = 0;  // DDA interrupts that fired during plan

// Trigger exec every N DDA ticks (200 = 1kHz at 200kHz DDA)
constexpr uint32_t kExecTriggerInterval = 200;

// Countdown timer for exec trigger (avoids expensive modulo)
volatile uint32_t exec_countdown = kExecTriggerInterval;

// LED heartbeat counter
volatile uint32_t led_counter = 0;
constexpr uint32_t kLEDToggleInterval = 200000;  // 1 second at 200kHz

// Flag to track if we're in exec or plan ISR (for preemption verification)
volatile bool in_exec_isr = false;
volatile bool in_plan_isr = false;

/****** DDA Timer Interrupt (Highest Priority) ******/

/*
 * DDA Timer ISR - TC3
 *
 * Fires at 200kHz (5us period).
 * Highest priority (NVIC priority 0).
 *
 * Every 200 ticks (1ms), triggers the Exec timer via software.
 * This ISR MUST preempt the Exec and Plan ISRs.
 */
namespace Motate {
    template<> void TimerChannel<3, 0>::interrupt() {
        dda_pin.set();  // Start timing marker

        dda_count++;

        // Track preemption (only increment if flag is set)
        dda_during_exec += in_exec_isr;
        dda_during_plan += in_plan_isr;

        // Trigger exec timer using countdown (avoids expensive modulo)
        if (--exec_countdown == 0) {
            exec_countdown = kExecTriggerInterval;
            exec_timer.setInterruptPending();
        }

        // LED heartbeat (less frequent check)
        if (++led_counter >= kLEDToggleInterval) {
            led_counter = 0;
            led_pin.toggle();
        }

        dda_pin.clear();  // End timing marker
    }
}

/****** Exec Timer Interrupt (High Priority) ******/

/*
 * Exec Timer ISR - TC4
 *
 * Software-triggered from DDA timer every 1ms.
 * High priority (NVIC priority 1).
 *
 * Simulates segment preparation work (~100us).
 * Triggers Plan timer via software.
 */
namespace Motate {
    template<> void Timer<4>::interrupt() {
        exec_pin.set();
        in_exec_isr = true;

        exec_count++;

        // Simulate segment preparation work (~100us at 120MHz)
        // 100us * 120MHz = 12000 cycles
        // Each loop iteration is ~10-20 cycles, so ~1000 iterations
        volatile uint32_t work = 0;
        for (int i = 0; i < 1000; i++) {
            work += exec_count;
        }
        (void)work;

        // Trigger planning interrupt
        plan_timer.setInterruptPending();

        in_exec_isr = false;
        exec_pin.clear();
    }
}

/****** Plan Timer Interrupt (Medium Priority) ******/

/*
 * Plan Timer ISR - TC5
 *
 * Software-triggered from Exec timer.
 * Medium priority (NVIC priority 2).
 *
 * Simulates motion planning work (~50us).
 */
namespace Motate {
    template<> void Timer<5>::interrupt() {
        plan_pin.set();
        in_plan_isr = true;

        plan_count++;

        // Simulate planning work (~50us)
        volatile uint32_t work = 0;
        for (int i = 0; i < 500; i++) {
            work += plan_count;
        }
        (void)work;

        in_plan_isr = false;
        plan_pin.clear();
    }
}

/****** Optional setup() function ******/

void setup() {
    // Initialize pins
    led_pin = 0;
    dda_pin = 0;
    exec_pin = 0;
    plan_pin = 0;

    // Startup indication
    for (int i = 0; i < 3; i++) {
        led_pin = 1;
        delay(100);
        led_pin = 0;
        delay(100);
    }

    // Configure DDA timer (TC3)
    // Hardware-triggered at 200kHz, highest priority
    dda_timer.setInterrupts(kInterruptOnOverflow | kInterruptPriorityHighest);

    // Configure Exec timer (TC4)
    // Software-triggered only, high priority
    // We still need to enable the NVIC but no hardware trigger
    exec_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityHigh);

    // Configure Plan timer (TC5)
    // Software-triggered only, medium priority
    plan_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityMedium);

    // Start the DDA timer (TC3)
    // TC4 and TC5 don't need to be started - they're software-triggered only
    dda_timer.start();
}

/****** Main run loop() ******/

void loop() {
    // Main loop does nothing - all work is interrupt-driven
    delay(1000);
}

/*
 * Expected Saleae Measurements:
 *
 * DDA Pin (D12):
 *   - Frequency: 200kHz (period = 5.00us)
 *   - HIGH pulse width: < 2us (ISR execution time)
 *   - Should see CONTINUOUS pulses with NO gaps
 *   - Jitter should be < 100ns
 *
 * Exec Pin (D11):
 *   - Frequency: ~1kHz (every 200 DDA ticks)
 *   - HIGH pulse width: ~100us (simulated work)
 *   - CRITICAL: DDA pulses should CONTINUE during this HIGH time
 *     (visible as small DDA pulses on D12 while D11 is high)
 *
 * Plan Pin (D10):
 *   - Fires immediately after each Exec ISR completes
 *   - HIGH pulse width: ~50us (simulated work)
 *   - Should see DDA pulses continue during this HIGH time
 *
 * Verifying Preemption:
 *   1. Zoom to show D11 HIGH period (~100us)
 *   2. Count DDA pulses on D12 during this time
 *   3. Should see ~20 DDA pulses (100us / 5us = 20)
 *   4. If DDA stops during Exec, preemption is broken!
 *
 * Debug Counters (via J-Link):
 *   - dda_count: Total DDA interrupts (200000/sec)
 *   - exec_count: Total Exec interrupts (1000/sec)
 *   - plan_count: Total Plan interrupts (1000/sec)
 *   - dda_during_exec: DDA interrupts that preempted Exec
 *   - dda_during_plan: DDA interrupts that preempted Plan
 *
 * If preemption is working:
 *   - dda_during_exec should be ~20x exec_count
 *   - dda_during_plan should be ~10x plan_count
 *
 * If preemption is broken:
 *   - dda_during_exec and dda_during_plan will be 0
 *   - DDA frequency will drop during Exec/Plan ISRs
 */
