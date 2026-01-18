/**
 * \file
 *
 * \brief System initialization for SAMD51
 *
 * Copyright (c) 2019 Microchip Technology Inc.
 * Copyright (c) 2024 Synthetos LLC (modifications for g2core)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Configures the SAMD51 for 120 MHz operation using DPLL0
 * with the internal 32.768 kHz oscillator as reference.
 *
 * Clock Configuration:
 *   GCLK0 (Main)  = 120 MHz (CPU, AHB, APB)
 *   GCLK1         = 48 MHz (USB)
 *   GCLK2         = 100 MHz (Available for high-speed peripherals)
 *
 * Flash wait states: 5 WS at 120 MHz (per datasheet table 54-42)
 * NVM cache: Enabled
 */

#include "samd51.h"

#ifdef __cplusplus
extern "C" {
#endif

/* System clock frequency (updated by SystemCoreClockUpdate) */
uint32_t SystemCoreClock = 48000000UL;  /* Start with DFLL48M default */

/* External oscillator frequency (if used) - default 12 MHz */
#ifndef CONF_XOSC_FREQUENCY
#define CONF_XOSC_FREQUENCY 12000000UL
#endif

/* Target frequencies */
#define F_CPU       120000000UL
#define F_USB       48000000UL

/**
 * \brief Wait for a register to be synchronized
 */
static inline void gclk_sync(void)
{
    while (GCLK->SYNCBUSY.reg)
        ;
}

static inline void oscctrl_dpll_sync(uint8_t dpll)
{
    while (OSCCTRL->Dpll[dpll].DPLLSYNCBUSY.reg)
        ;
}

/**
 * \brief Initialize the system
 *
 * Configures:
 * - Flash wait states for 120 MHz operation
 * - NVM cache
 * - GCLK generator 0 to 120 MHz using DPLL0
 * - GCLK generator 1 to 48 MHz using DFLL48M (for USB)
 */
void SystemInit(void)
{
    /* Set flash wait states for 120 MHz at 3.3V (see datasheet table 54-42) */
    /* 5 wait states required for >104 MHz at 3.3V */
    NVMCTRL->CTRLA.bit.RWS = 5;

    /* Enable automatic NVM wait state management (optional, for power saving) */
    NVMCTRL->CTRLA.bit.AUTOWS = 1;

    /* ----- Configure GCLK generators ----- */

    /* First, we need GCLK0 running to configure anything else.
     * The SAMD51 boots with DFLL48M running open-loop at 48 MHz on GCLK0.
     * We'll configure DPLL0 and then switch GCLK0 to it.
     */

    /* Enable the XOSC32K oscillator (32.768 kHz external crystal) if available,
     * otherwise use the internal ultra-low-power 32K oscillator (OSCULP32K).
     *
     * For simplicity and portability (Metro M4 has the crystal), we use OSCULP32K
     * as the DPLL reference source. The DPLL output will be less accurate but
     * doesn't require external components.
     *
     * OSCULP32K is always running after reset.
     */

    /* Configure GCLK2 to output OSCULP32K (32.768 kHz) for DPLL reference */
    GCLK->GENCTRL[2].reg = GCLK_GENCTRL_SRC_OSCULP32K |
                           GCLK_GENCTRL_GENEN;
    gclk_sync();

    /* Configure DPLL0 for 120 MHz output
     *
     * Reference: GCLK2 (32.768 kHz)
     * Output = Reference * (LDR + 1 + LDRFRAC/32)
     *
     * For 120 MHz from 32.768 kHz:
     *   120000000 / 32768 = 3662.109375
     *   LDR = 3661, LDRFRAC = 3 (3661 + 1 + 3/32 = 3662.09375)
     *   Actual output = 32768 * 3662.09375 = 119996948 Hz (~120 MHz)
     */

    /* First, connect GCLK2 to DPLL0 as reference */
    GCLK->PCHCTRL[OSCCTRL_GCLK_ID_FDPLL0].reg = GCLK_PCHCTRL_GEN_GCLK2 |
                                                 GCLK_PCHCTRL_CHEN;
    gclk_sync();

    /* Configure DPLL0 */
    OSCCTRL->Dpll[0].DPLLRATIO.reg = OSCCTRL_DPLLRATIO_LDR(3661) |
                                      OSCCTRL_DPLLRATIO_LDRFRAC(3);
    oscctrl_dpll_sync(0);

    /* Configure DPLL0 control:
     * - Use GCLK as reference (REFCLK = 0)
     * - Lock time: default
     * - Filter: default
     */
    OSCCTRL->Dpll[0].DPLLCTRLB.reg = OSCCTRL_DPLLCTRLB_REFCLK_GCLK |
                                      OSCCTRL_DPLLCTRLB_LBYPASS;
    oscctrl_dpll_sync(0);

    /* Enable DPLL0 */
    OSCCTRL->Dpll[0].DPLLCTRLA.reg = OSCCTRL_DPLLCTRLA_ENABLE;
    oscctrl_dpll_sync(0);

    /* Wait for DPLL0 to lock */
    while (!(OSCCTRL->Dpll[0].DPLLSTATUS.bit.LOCK))
        ;

    /* Now switch GCLK0 to DPLL0 for 120 MHz operation */
    GCLK->GENCTRL[0].reg = GCLK_GENCTRL_SRC_DPLL0 |
                           GCLK_GENCTRL_IDC |
                           GCLK_GENCTRL_GENEN;
    gclk_sync();

    /* Configure GCLK1 for 48 MHz (USB) using DFLL48M
     * DFLL48M is already running from reset in open-loop mode at 48 MHz
     */
    GCLK->GENCTRL[1].reg = GCLK_GENCTRL_SRC_DFLL |
                           GCLK_GENCTRL_IDC |
                           GCLK_GENCTRL_GENEN;
    gclk_sync();

    /* Update SystemCoreClock variable */
    SystemCoreClock = F_CPU;
}

/**
 * \brief Update SystemCoreClock variable
 *
 * Updates the SystemCoreClock variable to reflect the current GCLK0 frequency.
 * Call this after any clock configuration changes.
 */
void SystemCoreClockUpdate(void)
{
    uint32_t main_clk_src;
    uint32_t dpll_ref;
    uint32_t dpll_ratio;
    uint32_t dpll_frac;

    /* Get GCLK0 source */
    main_clk_src = GCLK->GENCTRL[0].bit.SRC;

    switch (main_clk_src) {
        case GCLK_GENCTRL_SRC_XOSC0_Val:
        case GCLK_GENCTRL_SRC_XOSC1_Val:
            SystemCoreClock = CONF_XOSC_FREQUENCY;
            break;

        case GCLK_GENCTRL_SRC_OSCULP32K_Val:
        case GCLK_GENCTRL_SRC_XOSC32K_Val:
            SystemCoreClock = 32768UL;
            break;

        case GCLK_GENCTRL_SRC_DFLL_Val:
            SystemCoreClock = 48000000UL;
            break;

        case GCLK_GENCTRL_SRC_DPLL0_Val:
            /* Calculate DPLL0 output frequency */
            dpll_ratio = OSCCTRL->Dpll[0].DPLLRATIO.bit.LDR;
            dpll_frac = OSCCTRL->Dpll[0].DPLLRATIO.bit.LDRFRAC;

            /* Determine reference clock */
            switch (OSCCTRL->Dpll[0].DPLLCTRLB.bit.REFCLK) {
                case 0:  /* GCLK reference */
                    dpll_ref = 32768UL;  /* Assume GCLK2 at 32.768 kHz */
                    break;
                case 1:  /* XOSC32K */
                    dpll_ref = 32768UL;
                    break;
                case 2:  /* XOSC0 */
                case 3:  /* XOSC1 */
                    dpll_ref = CONF_XOSC_FREQUENCY;
                    break;
                default:
                    dpll_ref = 32768UL;
                    break;
            }

            /* Output = ref * (LDR + 1 + LDRFRAC/32) */
            SystemCoreClock = dpll_ref * (dpll_ratio + 1);
            SystemCoreClock += (dpll_ref * dpll_frac) / 32;
            break;

        case GCLK_GENCTRL_SRC_DPLL1_Val:
            /* Similar calculation for DPLL1 */
            dpll_ratio = OSCCTRL->Dpll[1].DPLLRATIO.bit.LDR;
            dpll_frac = OSCCTRL->Dpll[1].DPLLRATIO.bit.LDRFRAC;
            dpll_ref = 32768UL;  /* Assume 32K reference */
            SystemCoreClock = dpll_ref * (dpll_ratio + 1);
            SystemCoreClock += (dpll_ref * dpll_frac) / 32;
            break;

        default:
            /* Unknown source, assume DFLL */
            SystemCoreClock = 48000000UL;
            break;
    }

    /* Apply GCLK0 divider if set */
    if (GCLK->GENCTRL[0].bit.DIVSEL) {
        /* Divide by 2^(DIV+1) */
        SystemCoreClock >>= (GCLK->GENCTRL[0].bit.DIV + 1);
    } else if (GCLK->GENCTRL[0].bit.DIV > 0) {
        /* Divide by DIV */
        SystemCoreClock /= GCLK->GENCTRL[0].bit.DIV;
    }
}

#ifdef __cplusplus
}
#endif
