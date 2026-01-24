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

    // Digital pins (Arduino header) - Metro M4 AirLift specific!
    // Note: AirLift uses different pins than standard Metro M4 due to ESP32 NINA
    _MAKE_MOTATE_PIN( 0, 'A', 23);   // D0/RX - PA23 SERCOM3/PAD[1]
    _MAKE_MOTATE_PIN( 1, 'A', 22);   // D1/TX - PA22 SERCOM3/PAD[0]
    _MAKE_MOTATE_PIN( 2, 'B', 17);   // D2 - PB17 (TCC3/WO[1], TCC0/WO[5])
    _MAKE_MOTATE_PIN( 3, 'B', 16);   // D3 - PB16 (TCC3/WO[0], TCC0/WO[4])
    _MAKE_MOTATE_PIN( 4, 'B', 13);   // D4 - PB13 (TCC3/WO[1], TCC0/WO[1])
    _MAKE_MOTATE_PIN( 5, 'B', 14);   // D5 - PB14 (TCC4/WO[0], TCC0/WO[2])
    _MAKE_MOTATE_PIN( 6, 'B', 15);   // D6 - PB15 (TCC4/WO[1], TCC0/WO[3])
    _MAKE_MOTATE_PIN( 7, 'B', 12);   // D7 - PB12 (TCC3/WO[0], TCC0/WO[0])
    _MAKE_MOTATE_PIN( 8, 'A', 21);   // D8 - PA21 (TCC0/WO[1], TCC1/WO[5])
    _MAKE_MOTATE_PIN( 9, 'A', 20);   // D9 - PA20 (TCC0/WO[0], TCC1/WO[4])
    _MAKE_MOTATE_PIN(10, 'A', 18);   // D10 - PA18 (TCC1/WO[2], SERCOM1 TX)
    _MAKE_MOTATE_PIN(11, 'A', 19);   // D11 - PA19 (TCC1/WO[3], SERCOM1 RX)
    _MAKE_MOTATE_PIN(12, 'A', 17);   // D12 - PA17 (TCC1/WO[1])
    _MAKE_MOTATE_PIN(13, 'A', 16);   // D13/LED - PA16 (TCC1/WO[0])

    // Analog pins (Arduino header) - Metro M4 AirLift specific!
    _MAKE_MOTATE_PIN(14, 'A',  2);   // A0 - PA02 (AIN0, DAC0)
    _MAKE_MOTATE_PIN(15, 'A',  5);   // A1 - PA05 (AIN2)
    _MAKE_MOTATE_PIN(16, 'B',  6);   // A2 - PB06 (AIN3)
    _MAKE_MOTATE_PIN(17, 'B',  0);   // A3 - PB00 (AIN4)
    _MAKE_MOTATE_PIN(18, 'B',  8);   // A4 - PB08 (AIN5)
    _MAKE_MOTATE_PIN(19, 'B',  9);   // A5 - PB09 (AIN10)

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

    // ESP32 NINA UART pins (for G-code WiFi streaming)
    // NINA_TX/SAMD_RX is PA04 (pin 18/A4) - already defined above
    // NINA_RX/SAMD_TX is PA07 (pin 9/D9) - already defined above
    // These can use SERCOM0: PA04=PAD0(RX), PA07=PAD3(TX)

    // ESP32 NINA flow control and reset pins
    _MAKE_MOTATE_PIN(50, 'B',  1);   // NINA_GPIO0 (RTS to ESP32) - PB01
    _MAKE_MOTATE_PIN(51, 'B',  4);   // NINA_ACK (CTS from ESP32) - PB04
    _MAKE_MOTATE_PIN(52, 'B',  5);   // NINA_RESET - PB05

    /*************************************************************
     *
     * PWM Pin Definitions - TCC peripheral mappings
     *
     * SAMD51 TCC Pin Multiplexing:
     *   Function F (index 5) - TCC0, TCC1
     *   Function G (index 6) - TCC0, TCC1, TCC2
     *
     * Format: _MAKE_MOTATE_PWM_PIN(port, pin, TimerType, peripheral, inverted)
     *
     * Note: We use typedefs to avoid preprocessor issues with template commas
     *
     *************************************************************/

    // Type aliases to protect template commas from preprocessor
    using TCC0_CH0 = TCCChannel<0, 0>;
    using TCC0_CH1 = TCCChannel<0, 1>;
    using TCC0_CH2 = TCCChannel<0, 2>;
    using TCC0_CH3 = TCCChannel<0, 3>;
    using TCC0_CH4 = TCCChannel<0, 4>;
    using TCC0_CH5 = TCCChannel<0, 5>;
    using TCC1_CH0 = TCCChannel<1, 0>;
    using TCC1_CH1 = TCCChannel<1, 1>;
    using TCC1_CH2 = TCCChannel<1, 2>;
    using TCC1_CH3 = TCCChannel<1, 3>;
    using TCC3_CH0 = TCCChannel<3, 0>;
    using TCC3_CH1 = TCCChannel<3, 1>;
    using TCC4_CH0 = TCCChannel<4, 0>;
    using TCC4_CH1 = TCCChannel<4, 1>;

    // PWM Pin Definitions for Metro M4 AirLift
    // Updated to match the correct AirLift pin mapping

    // D2/PB17 - TCC3/WO[1] or TCC0/WO[5] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 17, TCC3_CH1, F, false);

    // D3/PB16 - TCC3/WO[0] or TCC0/WO[4] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 16, TCC3_CH0, F, false);

    // D4/PB13 - TCC3/WO[1] or TCC0/WO[1] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 13, TCC0_CH1, F, false);

    // D5/PB14 - TCC4/WO[0] or TCC0/WO[2] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 14, TCC4_CH0, F, false);

    // D6/PB15 - TCC4/WO[1] or TCC0/WO[3] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 15, TCC4_CH1, F, false);

    // D7/PB12 - TCC3/WO[0] or TCC0/WO[0] via Function F
    _MAKE_MOTATE_PWM_PIN('B', 12, TCC0_CH0, F, false);

    // D8/PA21 - TCC0/WO[1] or TCC1/WO[5] via Function G
    _MAKE_MOTATE_PWM_PIN('A', 21, TCC0_CH1, G, false);

    // D9/PA20 - TCC0/WO[0] or TCC1/WO[4] via Function G
    _MAKE_MOTATE_PWM_PIN('A', 20, TCC0_CH0, G, false);

    // D10/PA18 - TCC1/WO[2] via Function F
    _MAKE_MOTATE_PWM_PIN('A', 18, TCC1_CH2, F, false);

    // D11/PA19 - TCC1/WO[3] via Function F
    _MAKE_MOTATE_PWM_PIN('A', 19, TCC1_CH3, F, false);

    // D12/PA17 - TCC1/WO[1] via Function F
    _MAKE_MOTATE_PWM_PIN('A', 17, TCC1_CH1, F, false);

    // D13/PA16 - TCC1/WO[0] via Function F (Built-in LED)
    _MAKE_MOTATE_PWM_PIN('A', 16, TCC1_CH0, F, false);

    // PB22/Pin40 - TCC1/WO[2] via Function F (NeoPixel)
    _MAKE_MOTATE_PWM_PIN('B', 22, TCC1_CH2, F, false);

} // namespace Motate

#endif // motate_pin_assignments_h
