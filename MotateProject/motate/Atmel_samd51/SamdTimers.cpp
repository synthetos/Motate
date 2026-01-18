/*
 utility/SamdTimers.cpp - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2026 Riley Porter
 Copyright (c) 2026 Synthetos LLC

 This file is part of the Motate Library.
 */

#include "SamdTimers.h"

namespace Motate {

    // SysTickTimer instance and static members
    Timer<SysTickTimerNum> SysTickTimer;
    volatile uint32_t Timer<SysTickTimerNum>::_motateTickCount = 0;

    // WatchDogTimer instance
    WatchDogTimerHardware WatchDogTimer;

    void delay(uint32_t ms) {
        uint32_t start = SysTickTimer.getValue();
        while ((SysTickTimer.getValue() - start) < ms) {
            // Wait
        }
    }

    // Weak interrupt handler - can be overridden by user
    void Timer<SysTickTimerNum>::interrupt() {
        // Default implementation does nothing
    }

} // namespace Motate

// SysTick Handler - called by CMSIS
extern "C" void SysTick_Handler(void) {
    Motate::SysTickTimer._increment();
    Motate::SysTickTimer.interrupt();
    Motate::SysTickTimer._handleEvents();
}
