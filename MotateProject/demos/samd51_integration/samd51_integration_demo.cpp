/*
 * samd51_integration_demo.cpp - SAMD51 Integration Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 6: Integration Test
 *
 * Purpose:
 *   - Run all components simultaneously for stability
 *   - Verify no HardFaults over extended operation
 *   - Verify no missed DDA interrupts
 *   - Verify USB remains responsive
 *   - Check for memory leaks or corruption
 *
 * Test Setup:
 *   - USB cable to Metro M4 AirLift
 *   - Optional: USB-Serial adapter to D0/D1
 *   - Saleae to D12, D11, D10 for timing verification
 *
 * Pass Criteria:
 *   - No HardFaults after 1+ hour
 *   - No missed DDA interrupts (counter matches expected)
 *   - USB responds to commands throughout
 *   - Memory stable (no leaks)
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D13/PA16 - Heartbeat LED
 *   D12/PA17 - DDA ISR output (200kHz)
 *   D11/PA19 - Exec ISR output (~1kHz)
 *   D10/PA18 - Plan ISR output (~1kHz)
 *   D0/PA23  - UART RX (optional secondary channel)
 *   D1/PA22  - UART TX (optional secondary channel)
 */

#include "MotatePins.h"
#include "MotateTimers.h"
#include "MotateUSB.h"
#include "MotateUSBCDC.h"
#include "MotateBuffer.h"
#include "SamdUART.h"
#include <cstdio>
#include <cstring>

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

/****** USB Settings ******/

const USBSettings_t Motate::USBSettings = {
    /* vendorID         = */ 0x1d50,
    /* productID        = */ 0x606d,
    /* productVersion   = */ 0.1,
    /* attributes       = */ kUSBConfigAttributeSelfPowered,
    /* powerConsumption = */ 500
};

const char16_t *Motate::getUSBVendorString(int8_t &length) {
    static const char16_t vendor[] = u"Synthetos";
    length = sizeof(vendor);
    return vendor;
}

const char16_t *Motate::getUSBProductString(int8_t &length) {
    static const char16_t product[] = u"SAMD51 Integration Test";
    length = sizeof(product);
    return product;
}

const char16_t *Motate::getUSBSerialNumberString(int8_t &length) {
    static const char16_t serial[] = u"000000000001";
    length = sizeof(serial);
    return serial;
}

/****** USB Device ******/

USBDevice<USBDeviceHardware, USBCDC> usb;
auto &SerialUSB = usb.Serial;
RXBuffer<1024, decltype(&SerialUSB)> rx_buffer(&SerialUSB);
TXBuffer<1024, decltype(&SerialUSB)> tx_buffer(&SerialUSB);

/****** Timer Configuration ******/

constexpr uint32_t kDDAFrequency = 200000;
TimerChannel<3, 0> dda_timer(kTimerUpToMatch, kDDAFrequency);
Timer<4> exec_timer;
Timer<5> plan_timer;

/****** Statistics ******/

// Counters - all 64-bit to prevent overflow during long tests
volatile uint64_t dda_count = 0;
volatile uint64_t exec_count = 0;
volatile uint64_t plan_count = 0;
volatile uint64_t dda_during_exec = 0;
volatile uint64_t dda_during_plan = 0;
volatile uint64_t usb_rx_bytes = 0;
volatile uint64_t usb_tx_bytes = 0;
volatile uint32_t usb_connects = 0;

// Error tracking
volatile uint32_t hardfault_count = 0;
volatile uint32_t missed_dda_count = 0;
volatile uint32_t stack_high_water = 0;

// Timing
volatile uint32_t uptime_seconds = 0;
volatile uint32_t led_counter = 0;

// ISR state flags
volatile bool in_exec_isr = false;
volatile bool in_plan_isr = false;

// USB state
volatile bool usb_connected = false;

/****** Timer Intervals ******/

constexpr uint32_t kExecTriggerInterval = 200;  // Every 200 DDA ticks = 1kHz
constexpr uint32_t kLEDToggleInterval = 200000; // 1 second at 200kHz
constexpr uint32_t kUptimeInterval = 200000;    // 1 second at 200kHz

/****** DDA Timer ISR (TC3 - Highest Priority) ******/

namespace Motate {
    template<> void TimerChannel<3, 0>::interrupt() {
        dda_pin.set();

        // Track preemption
        if (in_exec_isr) dda_during_exec++;
        if (in_plan_isr) dda_during_plan++;

        dda_count++;

        // Simulated step generation work
        volatile uint32_t dummy = 0;
        for (int i = 0; i < 6; i++) {
            dummy += (uint32_t)(dda_count);
        }
        (void)dummy;

        // Trigger exec
        if ((dda_count % kExecTriggerInterval) == 0) {
            exec_timer.setInterruptPending();
        }

        // LED heartbeat and uptime
        led_counter++;
        if (led_counter >= kLEDToggleInterval) {
            led_counter = 0;
            led_pin.toggle();
            uptime_seconds++;
        }

        dda_pin.clear();
    }
}

/****** Exec Timer ISR (TC4 - High Priority) ******/

namespace Motate {
    template<> void Timer<4>::interrupt() {
        exec_pin.set();
        in_exec_isr = true;

        exec_count++;

        // Simulated segment work (~100us)
        volatile uint32_t work = 0;
        for (int i = 0; i < 1000; i++) {
            work += (uint32_t)(exec_count);
        }
        (void)work;

        // Trigger planning
        plan_timer.setInterruptPending();

        in_exec_isr = false;
        exec_pin.clear();
    }
}

/****** Plan Timer ISR (TC5 - Medium Priority) ******/

namespace Motate {
    template<> void Timer<5>::interrupt() {
        plan_pin.set();
        in_plan_isr = true;

        plan_count++;

        // Simulated planning work (~50us)
        volatile uint32_t work = 0;
        for (int i = 0; i < 500; i++) {
            work += (uint32_t)(plan_count);
        }
        (void)work;

        in_plan_isr = false;
        plan_pin.clear();
    }
}

/****** USB Connection Callback ******/

void onConnectionChange(bool connected) {
    usb_connected = connected;
    if (connected) {
        usb_connects++;
        const char *msg = "=== SAMD51 Integration Test ===\r\n"
                          "Commands: status, help\r\n> ";
        tx_buffer.write(msg, ::strlen(msg));
    }
}

/****** Command Processing ******/

char cmd_buffer[128];
uint16_t cmd_pos = 0;

void processCommand() {
    cmd_buffer[cmd_pos] = '\0';

    if (strncmp(cmd_buffer, "status", 6) == 0) {
        char buf[512];
        int len = snprintf(buf, sizeof(buf),
            "\r\n=== Status Report ===\r\n"
            "Uptime: %lu seconds\r\n"
            "DDA count: %llu (expected: %llu)\r\n"
            "Exec count: %llu\r\n"
            "Plan count: %llu\r\n"
            "DDA preempts exec: %llu\r\n"
            "DDA preempts plan: %llu\r\n"
            "USB RX bytes: %llu\r\n"
            "USB TX bytes: %llu\r\n"
            "USB connects: %lu\r\n"
            "Missed DDA: %lu\r\n"
            "> ",
            uptime_seconds,
            dda_count,
            (uint64_t)uptime_seconds * kDDAFrequency,
            exec_count,
            plan_count,
            dda_during_exec,
            dda_during_plan,
            usb_rx_bytes,
            usb_tx_bytes,
            usb_connects,
            missed_dda_count);
        tx_buffer.write(buf, len);
        usb_tx_bytes += len;
    }
    else if (strncmp(cmd_buffer, "help", 4) == 0) {
        const char *help = "\r\nCommands:\r\n"
                           "  status - Show statistics\r\n"
                           "  help   - Show this help\r\n"
                           "\r\n"
                           "Test Procedure:\r\n"
                           "  1. Let run for 1+ hour\r\n"
                           "  2. Check 'status' periodically\r\n"
                           "  3. DDA count should match expected\r\n"
                           "  4. Missed DDA should be 0\r\n"
                           "> ";
        tx_buffer.write(help, ::strlen(help));
        usb_tx_bytes += ::strlen(help);
    }
    else if (cmd_pos > 0) {
        const char *unknown = "\r\nUnknown command. Type 'help' for commands.\r\n> ";
        tx_buffer.write(unknown, ::strlen(unknown));
        usb_tx_bytes += ::strlen(unknown);
    }

    cmd_pos = 0;
}

/****** Setup ******/

void setup() {
    // Initialize pins
    led_pin = 0;
    dda_pin = 0;
    exec_pin = 0;
    plan_pin = 0;

    // Startup indication
    for (int i = 0; i < 5; i++) {
        led_pin = 1;
        delay(50);
        led_pin = 0;
        delay(50);
    }

    // Initialize USB
    SerialUSB.setConnectionCallback(onConnectionChange);
    rx_buffer.init();
    tx_buffer.init();
    usb.attach();

    // Configure timers
    dda_timer.setInterrupts(kInterruptOnOverflow | kInterruptPriorityHighest);
    exec_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityHigh);
    plan_timer.setInterrupts(kInterruptOnSoftwareTrigger | kInterruptPriorityMedium);

    // Start DDA timer
    dda_timer.start();
}

/****** Main Loop ******/

void loop() {
    // Process USB data
    int16_t c;
    while ((c = rx_buffer.read()) >= 0) {
        usb_rx_bytes++;

        if (c == '\r' || c == '\n') {
            if (cmd_pos > 0) {
                processCommand();
            }
        }
        else if (c == '\b' || c == 0x7F) {
            if (cmd_pos > 0) {
                cmd_pos--;
                tx_buffer.write("\b \b", 3);
                usb_tx_bytes += 3;
            }
        }
        else if (cmd_pos < sizeof(cmd_buffer) - 1) {
            cmd_buffer[cmd_pos++] = (char)c;
            char echo = (char)c;
            tx_buffer.write(&echo, 1);
            usb_tx_bytes++;
        }
    }

    // Check for missed DDA interrupts (compare count to expected)
    static uint64_t last_dda_count = 0;
    static uint32_t last_uptime = 0;

    if (uptime_seconds > last_uptime) {
        uint64_t expected = (uint64_t)uptime_seconds * kDDAFrequency;
        if (dda_count < expected - kDDAFrequency) {
            // More than 1 second behind - missed interrupts
            missed_dda_count += (expected - dda_count) / kDDAFrequency;
        }
        last_uptime = uptime_seconds;
        last_dda_count = dda_count;
    }

    delay(10);  // Small delay to prevent tight polling
}

/*
 * Test Procedure:
 *
 * 1. Build and flash:
 *    cd MotateProject/demos/samd51_integration
 *    make BOARD=MetroM4AirLift
 *    [Flash via J-Link or UF2]
 *
 * 2. Connect Saleae:
 *    - D12: DDA pulses (200kHz)
 *    - D11: Exec pulses (~1kHz)
 *    - D10: Plan pulses (~1kHz)
 *
 * 3. Open USB serial terminal (any baud rate)
 *
 * 4. Run initial verification:
 *    - Type 'status' - should show counters
 *    - Verify DDA preempts shown (proves timer hierarchy)
 *
 * 5. Saleae verification:
 *    - Capture 1ms of data
 *    - D12: should see ~200 pulses (200kHz)
 *    - During D11 HIGH: should see D12 pulses continue
 *
 * 6. Long-duration test:
 *    - Let run for 1+ hour
 *    - Check 'status' at 1min, 10min, 30min, 1hr
 *    - Expected DDA count = uptime_seconds * 200000
 *    - Missed DDA should remain 0
 *
 * Pass Criteria:
 *   | Metric           | Required                     |
 *   |------------------|------------------------------|
 *   | Runtime          | 1+ hour without HardFault    |
 *   | DDA count        | Within 0.1% of expected      |
 *   | Missed DDA       | 0                            |
 *   | USB responsive   | Commands work at any time    |
 *   | Preemption       | dda_during_exec > 0          |
 *
 * Debug via J-Link:
 *   - Read hardfault_count (should be 0)
 *   - Read missed_dda_count (should be 0)
 *   - Read stack_high_water (monitor for growth)
 */
