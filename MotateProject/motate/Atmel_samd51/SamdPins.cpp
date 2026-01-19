/*
 SamdPins.cpp - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2026 Riley Porter
 Copyright (c) 2026 Synthetos LLC

 This file is part of the Motate Library.

 This file ("the software") is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License, version 2 as published by the
 Free Software Foundation. You should have received a copy of the GNU General Public
 License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.

 As a special exception, you may use this file as part of a software library without
 restriction. Specifically, if other files instantiate templates or use macros or
 inline functions from this file, or you compile this file and link it with  other
 files to produce an executable, this file does not by itself cause the resulting
 executable to be covered by the GNU General Public License. This exception does not
 however invalidate any other reasons why the executable file might be covered by the
 GNU General Public License.

 THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "MotatePins.h"

using Motate::_pinChangeInterrupt;
using Motate::PortHardware;

// Static member definitions for PortHardware template specializations
// SAMD51 has PORTA and PORTB (some variants also have PORTC and PORTD)

template <>
_pinChangeInterrupt* PortHardware<'A'>::_firstInterrupt = nullptr;

template <>
_pinChangeInterrupt* PortHardware<'B'>::_firstInterrupt = nullptr;

#ifdef PORTC
template <>
_pinChangeInterrupt* PortHardware<'C'>::_firstInterrupt = nullptr;
#endif

#ifdef PORTD
template <>
_pinChangeInterrupt* PortHardware<'D'>::_firstInterrupt = nullptr;
#endif

// EIC (External Interrupt Controller) Handlers
// SAMD51 uses EIC for pin change interrupts, not per-port PIO interrupts like SAM3X/SAM4S
// The EIC has 16 external interrupt lines (EXTINT[0-15])
// Each pin can be mapped to one EXTINT line based on its pin number (pin % 16)
//
// For now, we provide stub handlers. Full EIC support would require:
// 1. Enabling GCLK for EIC (GCLK_EIC)
// 2. Enabling EIC in MCLK
// 3. Configuring EIC CONFIG[n] for sense mode
// 4. Enabling interrupts in EIC->INTENSET
// 5. Enabling NVIC IRQ for EIC_x_IRQn

// EIC interrupt handlers - SAMD51 has individual IRQs for EXTINT[0-15]
// These are called when external interrupts fire

extern "C" {

// EXTINT[0]
void EIC_0_Handler(void) {
    // Clear interrupt flag
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 0);

    // Walk the interrupt chain for PORTA pins where (pin % 16) == 0
    // This is a simplified implementation - full version would track which port/pin
    // Actually, with EIC, we need a different approach since EIC lines aren't port-specific
    // For now, just clear the flag
}

// EXTINT[1]
void EIC_1_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 1);
}

// EXTINT[2]
void EIC_2_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 2);
}

// EXTINT[3]
void EIC_3_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 3);
}

// EXTINT[4]
void EIC_4_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 4);
}

// EXTINT[5]
void EIC_5_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 5);
}

// EXTINT[6]
void EIC_6_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 6);
}

// EXTINT[7]
void EIC_7_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 7);
}

// EXTINT[8]
void EIC_8_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 8);
}

// EXTINT[9]
void EIC_9_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 9);
}

// EXTINT[10]
void EIC_10_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 10);
}

// EXTINT[11]
void EIC_11_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 11);
}

// EXTINT[12]
void EIC_12_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 12);
}

// EXTINT[13]
void EIC_13_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 13);
}

// EXTINT[14]
void EIC_14_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 14);
}

// EXTINT[15]
void EIC_15_Handler(void) {
    EIC->INTFLAG.reg = EIC_INTFLAG_EXTINT(1 << 15);
}

} // extern "C"
