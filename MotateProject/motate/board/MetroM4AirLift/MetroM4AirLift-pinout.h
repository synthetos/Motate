/*
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
 *
 * This file is part of the Motate Library.
 *
 * Pinout definitions for Adafruit Metro M4 AirLift Lite
 */

#ifndef MetroM4AirLift_pinout_h
#define MetroM4AirLift_pinout_h

#include <MotatePins.h>

namespace Motate {

    // LED Pin
    pin_number kLED_Pin = 13;  // D13 / PA17

    // USB Pins (directly connected to USB peripheral)
    // PA24 = USB D-
    // PA25 = USB D+

    // Debug LED assignments
    pin_number kDebug1_Pin = 13;
    pin_number kDebug2_Pin = -1;
    pin_number kDebug3_Pin = -1;
    pin_number kDebug4_Pin = -1;

} // namespace Motate

#endif // MetroM4AirLift_pinout_h
