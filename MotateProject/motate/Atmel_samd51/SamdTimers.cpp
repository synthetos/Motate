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

// TCC Interrupt Handlers
// TCC peripherals have multiple IRQs per instance (OVF, MCx, etc.)
// We handle the main OVF/ERR interrupt here.
//
// Note: For DMA-driven PWM (NeoPixel), interrupts are typically not used
// as DMA handles the transfers automatically.

#define _MAKE_TCCx_Handler(x, tccbase) \
    namespace Motate { \
        template<> void TCCTimer<x>::interrupt() __attribute__ ((weak)); \
        template<> void TCCChannel<x, 0>::interrupt() __attribute__ ((weak)); \
        template<> void TCCChannel<x, 1>::interrupt() __attribute__ ((weak)); \
        template<> void TCCChannel<x, 2>::interrupt() __attribute__ ((weak)); \
    } \
    extern "C" \
    void TCC##x##_0_Handler(void) { \
        Motate::TCCTimer<x>::_interrupt_cause_cached = tccbase->INTFLAG.reg; \
        tccbase->INTFLAG.reg = TCC_INTFLAG_MASK; \
        if (Motate::TCCTimer<x>::interrupt) { \
            Motate::TCCTimer<x>::interrupt(); \
        } \
    } \
    extern "C" \
    void TCC##x##_1_Handler(void) { \
        TCC##x##_0_Handler(); \
    } \
    extern "C" \
    void TCC##x##_2_Handler(void) { \
        TCC##x##_0_Handler(); \
    }

_MAKE_TCCx_Handler(0, TCC0)
_MAKE_TCCx_Handler(1, TCC1)
_MAKE_TCCx_Handler(2, TCC2)

#ifdef TCC3
_MAKE_TCCx_Handler(3, TCC3)
#endif

#ifdef TCC4
_MAKE_TCCx_Handler(4, TCC4)
#endif

#undef _MAKE_TCCx_Handler
