/*
 utility/SamdCommon.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2024 Synthetos LLC

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

#ifndef SAMDCOMMON_H_ONCE
#define SAMDCOMMON_H_ONCE

#include "sam.h"

namespace Motate {

/**
 * SAMD51-specific common utilities
 *
 * The SAMD51 uses different clock architecture than SAM3/4/S70:
 * - MCLK (Main Clock) controller instead of PMC
 * - GCLK (Generic Clock) for peripheral clocking
 * - APBxMASK registers for peripheral clock enable
 */

struct SamdCommon {
    static inline void sync() { __DMB(); };

    /**
     * Enable peripheral clock via MCLK APBx mask registers
     *
     * SAMD51 peripheral clock enable is done via MCLK->APBxMASK registers:
     * - APBAMASK: Low-power peripherals (PM, MCLK, RSTC, OSCCTRL, OSC32KCTRL, SUPC, GCLK, WDT, RTC, EIC, FREQM, SERCOM0-1, TCC0-1, TC0-1)
     * - APBBMASK: USB, DSU, NVMCTRL, PORT, EVSYS, SERCOM2-3, TCC2-3, TCC4, TC2-3, RAMECC
     * - APBCMASK: TCC4 (alt), TC4-5, PDEC, AC, AES, TRNG, ICM, PUKCC, QSPI, CCL, SERCOM4-5
     * - APBDMASK: SERCOM6-7, TCC6, TC6-7, ADC0-1, DAC, I2S, PCC
     *
     * For simplicity, we provide a helper that takes the APB bus and mask bit.
     */

    enum class APBBus { A, B, C, D };

    static void enablePeripheralClock(APBBus bus, uint32_t mask) {
        switch (bus) {
            case APBBus::A:
                MCLK->APBAMASK.reg |= mask;
                break;
            case APBBus::B:
                MCLK->APBBMASK.reg |= mask;
                break;
            case APBBus::C:
                MCLK->APBCMASK.reg |= mask;
                break;
            case APBBus::D:
                MCLK->APBDMASK.reg |= mask;
                break;
        }
        sync();
    };

    static void disablePeripheralClock(APBBus bus, uint32_t mask) {
        switch (bus) {
            case APBBus::A:
                MCLK->APBAMASK.reg &= ~mask;
                break;
            case APBBus::B:
                MCLK->APBBMASK.reg &= ~mask;
                break;
            case APBBus::C:
                MCLK->APBCMASK.reg &= ~mask;
                break;
            case APBBus::D:
                MCLK->APBDMASK.reg &= ~mask;
                break;
        }
        sync();
    };

    /**
     * Enable a GCLK generator output to a peripheral channel
     *
     * @param channel The GCLK channel (peripheral) to configure
     * @param generator The GCLK generator number (0-11)
     */
    static void enableGCLKChannel(uint32_t channel, uint32_t generator) {
        GCLK->PCHCTRL[channel].reg = GCLK_PCHCTRL_GEN(generator) | GCLK_PCHCTRL_CHEN;
        while (GCLK->SYNCBUSY.reg)
            ;
    };

    static void disableGCLKChannel(uint32_t channel) {
        GCLK->PCHCTRL[channel].reg &= ~GCLK_PCHCTRL_CHEN;
        while (GCLK->SYNCBUSY.reg)
            ;
    };

    /**
     * Get peripheral clock frequency
     *
     * Returns SystemCoreClock as GCLK0 is typically used for all peripherals.
     * For peripherals on different GCLK generators, calculate separately.
     */
    static uint32_t getPeripheralClockFreq() {
        return SystemCoreClock;
    };

    /**
     * RAII interrupt disabler
     */
    struct InterruptDisabler {
        volatile uint32_t flags;
        InterruptDisabler() : flags{__get_PRIMASK()} {
            __disable_irq();
            sync();
        };
        ~InterruptDisabler() {
            sync();
            if (!flags) {
                __enable_irq();
            }
        };
    };

    /**
     * Common GCLK generator IDs
     */
    static constexpr uint32_t GCLK_MAIN = 0;       // 120 MHz (GCLK0)
    static constexpr uint32_t GCLK_48MHZ = 1;      // 48 MHz (GCLK1, for USB)
    static constexpr uint32_t GCLK_32K = 2;        // 32.768 kHz (GCLK2)
};

}  // namespace Motate

#endif /* end of include guard: SAMDCOMMON_H_ONCE */
