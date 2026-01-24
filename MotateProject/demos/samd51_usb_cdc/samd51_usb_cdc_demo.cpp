/*
 * samd51_usb_cdc_demo.cpp - SAMD51 USB CDC Test
 *
 * Part of SAMD51 Motate Bring-Up Test Suite
 *
 * Phase 4: USB CDC with DMA Buffers
 *
 * Purpose:
 *   - Verify USB device enumeration on Windows/Mac/Linux
 *   - Test DTR callback for connection detection
 *   - Verify DMA circular buffer receive/transmit
 *   - Test line-based echo (g2core pattern)
 *
 * Test Setup:
 *   - Connect USB cable to Metro M4 AirLift
 *   - Open serial terminal at any baud rate (virtual serial)
 *   - Type lines of text and press Enter
 *
 * Pass Criteria:
 *   - Enumerates as CDC device on Windows/Mac/Linux
 *   - DTR callback fires on terminal connect
 *   - Echo works for lines up to 511 chars
 *   - No data loss at sustained throughput
 *
 * Pin Assignments (Metro M4 AirLift):
 *   D13/PA16 - Built-in LED (connection indicator)
 *   D12/PA17 - Debug output (optional RX activity)
 */

#include "MotatePins.h"
#include "MotateTimers.h"
#include "MotateUSB.h"
#include "MotateUSBCDC.h"
#include "MotateBuffer.h"
#include <cstdio>
#include <cstring>

using namespace Motate;

/****** Pin Definitions ******/

constexpr pin_number kLED_PinNumber = 13;     // D13/PA16 - Connection LED
constexpr pin_number kDebug_PinNumber = 12;   // D12/PA17 - Debug output

/****** Global Pin Objects ******/

OutputPin<kLED_PinNumber> led_pin;
OutputPin<kDebug_PinNumber> debug_pin;

/****** USB Settings ******/

// USB vendor and product IDs
// Using Synthetos g2core IDs for testing
const USBSettings_t Motate::USBSettings = {
    /* vendorID         = */ 0x1d50,  // OpenMoko (shared VID for open hardware)
    /* productID        = */ 0x606d,  // g2core
    /* productVersion   = */ 0.1,
    /* attributes       = */ kUSBConfigAttributeSelfPowered,
    /* powerConsumption = */ 500      // 500mA max
};

/****** USB String Descriptors ******/

// Vendor string - company name
const char16_t *Motate::getUSBVendorString(int8_t &length) {
    static const char16_t vendor[] = u"Synthetos";
    length = sizeof(vendor);
    return vendor;
}

// Product string - device name
const char16_t *Motate::getUSBProductString(int8_t &length) {
    static const char16_t product[] = u"SAMD51 USB CDC Demo";
    length = sizeof(product);
    return product;
}

// Serial number string
const char16_t *Motate::getUSBSerialNumberString(int8_t &length) {
    static const char16_t serial[] = u"000000000001";
    length = sizeof(serial);
    return serial;
}

/****** USB Device and CDC Serial ******/

// USB device with CDC interface
USBDevice<USBDeviceHardware, USBCDC> usb;

// Shortcut reference to the CDC serial interface
// The CDC mixin provides a .Serial member
auto &SerialUSB = usb.Serial;

/****** DMA Circular Buffers ******/

// RX buffer (host -> device)
// 1024 bytes matching g2core's configuration
RXBuffer<1024, decltype(&SerialUSB)> rx_buffer(&SerialUSB);

// TX buffer (device -> host)
TXBuffer<1024, decltype(&SerialUSB)> tx_buffer(&SerialUSB);

/****** Line Buffer for Echo ******/

char line_buffer[512];
uint16_t line_pos = 0;

/****** Connection State ******/

volatile bool usb_connected = false;
volatile uint32_t connection_count = 0;

/****** Statistics ******/

volatile uint32_t bytes_received = 0;
volatile uint32_t bytes_sent = 0;
volatile uint32_t lines_received = 0;

/****** Connection Callback ******/

void onConnectionChange(bool connected) {
    usb_connected = connected;
    led_pin = connected ? 1 : 0;
    connection_count++;

    if (connected) {
        // Send welcome message on connect
        const char *welcome = "SAMD51 USB CDC Demo\r\n"
                              "Type lines and press Enter for echo.\r\n"
                              ">\r\n";
        tx_buffer.write(welcome, ::strlen(welcome));
    }
}

/****** Optional setup() function ******/

void setup() {
    // Initialize pins
    led_pin = 0;
    debug_pin = 0;

    // Startup LED flash
    for (int i = 0; i < 3; i++) {
        led_pin = 1;
        delay(100);
        led_pin = 0;
        delay(100);
    }

    // Set up connection callback
    SerialUSB.setConnectionCallback(onConnectionChange);

    // Initialize DMA buffers
    rx_buffer.init();
    tx_buffer.init();

    // Attach USB device (enables USB peripheral)
    usb.attach();
}

/****** Process a complete line ******/

void processLine() {
    lines_received++;

    // Echo the line back with prefix
    const char *prefix = "echo: ";
    tx_buffer.write(prefix, ::strlen(prefix));
    tx_buffer.write(line_buffer, line_pos);
    tx_buffer.write("\r\n", 2);

    // Special commands
    if (strncmp(line_buffer, "stats", 5) == 0) {
        char stats[128];
        int len = snprintf(stats, sizeof(stats),
            "Stats: rx=%lu tx=%lu lines=%lu conn=%lu\r\n",
            bytes_received, bytes_sent, lines_received, connection_count);
        tx_buffer.write(stats, len);
    }
    else if (strncmp(line_buffer, "help", 4) == 0) {
        const char *help = "Commands:\r\n"
                           "  help  - Show this help\r\n"
                           "  stats - Show statistics\r\n"
                           "  Any other text is echoed back\r\n";
        tx_buffer.write(help, ::strlen(help));
    }

    // Prompt for next line
    tx_buffer.write("> ", 2);

    // Reset line buffer
    line_pos = 0;
}

/****** Main run loop() ******/

void loop() {
    // Poll USB for SETUP packets (required on SAMD51)
    // SAMD51 doesn't interrupt on SETUP, so we poll via SOF
    // This is handled internally by the USB driver during attach

    // Process received data
    int16_t c;
    while ((c = rx_buffer.read()) >= 0) {
        debug_pin.toggle();  // Show RX activity
        bytes_received++;

        // Handle character
        if (c == '\r' || c == '\n') {
            // End of line
            if (line_pos > 0) {
                line_buffer[line_pos] = '\0';
                processLine();
            }
        }
        else if (c == '\b' || c == 0x7F) {
            // Backspace
            if (line_pos > 0) {
                line_pos--;
                // Echo backspace (erase character on terminal)
                tx_buffer.write("\b \b", 3);
            }
        }
        else if (line_pos < sizeof(line_buffer) - 1) {
            // Normal character - add to buffer
            line_buffer[line_pos++] = (char)c;
            // Echo character back
            char echo = (char)c;
            tx_buffer.write(&echo, 1);
        }
    }

    // Small delay to prevent tight polling
    delay(1);
}

/*
 * Expected Behavior:
 *
 * 1. Connect USB cable to Metro M4 AirLift
 * 2. Device should enumerate as "SAMD51 USB CDC Demo"
 *    - Windows: Check Device Manager -> Ports (COM & LPT)
 *    - Mac: ls /dev/tty.usb*
 *    - Linux: ls /dev/ttyACM*
 *
 * 3. Open serial terminal (PuTTY, screen, minicom, etc.)
 *    - Baud rate doesn't matter (virtual serial)
 *
 * 4. LED should turn ON when terminal connects (DTR callback)
 *
 * 5. Type "hello" and press Enter:
 *    Expected output:
 *      hello
 *      echo: hello
 *      >
 *
 * 6. Type "stats" and press Enter:
 *    Expected output:
 *      Stats: rx=N tx=N lines=N conn=N
 *
 * Stress Test:
 *   - Paste large amounts of text
 *   - Should echo without data loss
 *   - Send files: cat large_file > /dev/ttyACM0
 *
 * Debug Counters (via J-Link):
 *   - bytes_received: Total bytes from host
 *   - bytes_sent: Total bytes to host
 *   - lines_received: Total lines processed
 *   - connection_count: Number of DTR connect events
 *   - g_cdc_set_control_line_count: SET_CONTROL_LINE_STATE count
 */
