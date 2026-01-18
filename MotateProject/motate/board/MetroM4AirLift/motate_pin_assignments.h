/*
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
 *
 * This file is part of the Motate Library.
 *
 * Pin assignments for Adafruit Metro M4 AirLift Lite (ATSAMD51J19A)
 *
 * Pin mapping based on Metro M4 AirLift Lite schematic:
 * https://learn.adafruit.com/adafruit-metro-m4-express-airlift-wifi
 *
 * Arduino Pin -> SAMD51 Port Pin
 */

#ifndef motate_pin_assignments_h
#define motate_pin_assignments_h

#include <MotateTimers.h>

namespace Motate {

    // Metro M4 AirLift Lite Pin Assignments
    // Format: _MAKE_MOTATE_PIN(Arduino_Pin, Port_Letter, Port_Number)

    // Digital pins (Arduino header)
    _MAKE_MOTATE_PIN( 0, 'A', 23);   // D0/RX - PA23 SERCOM3/PAD[1]
    _MAKE_MOTATE_PIN( 1, 'A', 22);   // D1/TX - PA22 SERCOM3/PAD[0]
    _MAKE_MOTATE_PIN( 2, 'A', 14);   // D2 - PA14 (TCC2/WO[0])
    _MAKE_MOTATE_PIN( 3, 'A',  9);   // D3 - PA09 (TCC0/WO[1] or TCC1/WO[3])
    _MAKE_MOTATE_PIN( 4, 'A',  8);   // D4 - PA08 (TCC0/WO[0] or TCC1/WO[2])
    _MAKE_MOTATE_PIN( 5, 'A', 15);   // D5 - PA15 (TCC2/WO[1])
    _MAKE_MOTATE_PIN( 6, 'A', 20);   // D6 - PA20 (TCC0/WO[0] or TCC1/WO[2])
    _MAKE_MOTATE_PIN( 7, 'A', 21);   // D7 - PA21 (TCC0/WO[1] or TCC1/WO[3])
    _MAKE_MOTATE_PIN( 8, 'A',  6);   // D8 - PA06
    _MAKE_MOTATE_PIN( 9, 'A',  7);   // D9 - PA07 (TCC1/WO[1])
    _MAKE_MOTATE_PIN(10, 'A', 18);   // D10 - PA18 (TCC0/WO[2])
    _MAKE_MOTATE_PIN(11, 'A', 16);   // D11 - PA16 (TCC0/WO[4])
    _MAKE_MOTATE_PIN(12, 'A', 19);   // D12 - PA19 (TCC0/WO[3])
    _MAKE_MOTATE_PIN(13, 'A', 17);   // D13/LED - PA17 (TCC0/WO[5])

    // Analog pins (Arduino header)
    _MAKE_MOTATE_PIN(14, 'A',  2);   // A0 - PA02 (AIN0, DAC0)
    _MAKE_MOTATE_PIN(15, 'A',  5);   // A1 - PA05 (AIN5)
    _MAKE_MOTATE_PIN(16, 'B',  8);   // A2 - PB08 (AIN2)
    _MAKE_MOTATE_PIN(17, 'B',  9);   // A3 - PB09 (AIN3)
    _MAKE_MOTATE_PIN(18, 'A',  4);   // A4 - PA04 (AIN4)
    // A5 (PA06) is shared with D8, use pin 8

    // I2C pins
    _MAKE_MOTATE_PIN(20, 'B',  2);   // SDA - PB02 SERCOM5/PAD[0]
    _MAKE_MOTATE_PIN(21, 'B',  3);   // SCL - PB03 SERCOM5/PAD[1]

    // SPI pins
    _MAKE_MOTATE_PIN(22, 'A', 12);   // MOSI - PA12 SERCOM2/PAD[0]
    _MAKE_MOTATE_PIN(23, 'A', 13);   // SCK - PA13 SERCOM2/PAD[1]
    // MISO (PA14) is shared with D2, use pin 2

    // Additional pins
    _MAKE_MOTATE_PIN(25, 'A', 27);   // AREF
    _MAKE_MOTATE_PIN(26, 'A',  3);   // DAC1/A6 - PA03

    // ESP32 WiFi co-processor pins (directly connected to SAMD51)
    // These are directly wired, no need to add as separate pins

    // NeoPixel (internal status LED)
    _MAKE_MOTATE_PIN(40, 'B', 22);   // NEOPIXEL - PB22

} // namespace Motate

#endif // motate_pin_assignments_h
