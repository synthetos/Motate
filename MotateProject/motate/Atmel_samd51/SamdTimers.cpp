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

// TC Interrupt Handlers
// These handlers cache the interrupt flags before clearing them,
// then call the Motate template interrupt handler.
//
// The pattern uses weak symbols so that:
// 1. If user code defines TimerChannel<N,0>::interrupt(), it gets called
// 2. If user code defines Timer<N>::interrupt(), it gets called
// 3. If neither is defined, the weak symbol is null and the check fails gracefully

#define _MAKE_TCx_Handler(x, tcbase) \
    namespace Motate { \
        template<> void TimerChannel<x, 0>::interrupt() __attribute__ ((weak)); \
        template<> void Timer<x>::interrupt() __attribute__ ((weak)); \
    } \
    extern "C" \
    void TC##x##_Handler(void) { \
        Motate::Timer<x>::_interrupt_cause_cached = tcbase->COUNT16.INTFLAG.reg; \
        tcbase->COUNT16.INTFLAG.reg = TC_INTFLAG_MASK; \
        if (Motate::TimerChannel<x, 0>::interrupt) { \
            Motate::TimerChannel<x, 0>::interrupt(); \
        } \
        if (Motate::Timer<x>::interrupt) { \
            Motate::Timer<x>::interrupt(); \
        } \
    }

_MAKE_TCx_Handler(0, TC0)
_MAKE_TCx_Handler(1, TC1)
_MAKE_TCx_Handler(2, TC2)
_MAKE_TCx_Handler(3, TC3)
_MAKE_TCx_Handler(4, TC4)
_MAKE_TCx_Handler(5, TC5)

#undef _MAKE_TCx_Handler
