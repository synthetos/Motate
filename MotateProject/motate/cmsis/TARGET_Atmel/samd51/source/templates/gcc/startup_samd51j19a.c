/**
 * \file
 *
 * \brief Startup code for SAMD51J19A
 *
 * Copyright (c) 2019 Microchip Technology Inc.
 * Copyright (c) 2024 Synthetos LLC (modifications for g2core)
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * SAMD51J19A Vector Table and Reset Handler
 * - 137 peripheral IRQs + 16 Cortex-M4 exceptions
 * - Cortex-M4F with FPU support
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "samd51.h"

/* Initialize segments */
extern uint32_t _sfixed;
extern uint32_t _efixed;
extern uint32_t _etext;
extern uint32_t _srelocate;
extern uint32_t _erelocate;
extern uint32_t _szero;
extern uint32_t _ezero;
extern uint32_t _sstack;
extern uint32_t _estack;

/** \cond DOXYGEN_SHOULD_SKIP_THIS */
int main(void);
/** \endcond */

void __libc_init_array(void);
void SystemInit(void);

/* Default empty handler */
void Dummy_Handler(void);

/* Cortex-M4 core handlers */
void NMI_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void HardFault_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void MemManage_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void BusFault_Handler   ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void UsageFault_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SVC_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DebugMon_Handler   ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PendSV_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SysTick_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));

/* Peripheral handlers - SAMD51J19A specific */
void PM_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void MCLK_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSCCTRL_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSCCTRL_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSCCTRL_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSCCTRL_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSCCTRL_4_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void OSC32KCTRL_Handler ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SUPC_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SUPC_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void WDT_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RTC_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_0_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_1_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_2_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_3_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_4_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_5_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_6_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_7_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_8_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_9_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_10_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_11_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_12_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_13_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_14_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EIC_15_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void FREQM_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void NVMCTRL_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void NVMCTRL_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_3_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DMAC_4_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EVSYS_0_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EVSYS_1_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EVSYS_2_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EVSYS_3_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void EVSYS_4_Handler    ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PAC_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void RAMECC_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM0_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM0_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM0_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM0_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM1_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM1_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM1_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM1_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM2_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM2_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM2_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM2_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM3_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM3_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM3_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM3_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM4_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM4_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM4_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM4_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM5_0_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM5_1_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM5_2_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SERCOM5_3_Handler  ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void USB_0_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void USB_1_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void USB_2_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void USB_3_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_3_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_4_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_5_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC0_6_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_3_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC1_4_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC2_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC2_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC2_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC2_3_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC3_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC3_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC3_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC4_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC4_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TCC4_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC0_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC1_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC2_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC3_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC4_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TC5_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PDEC_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PDEC_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PDEC_2_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ADC0_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ADC0_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ADC1_0_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ADC1_1_Handler     ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void AC_Handler         ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DAC_0_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DAC_1_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DAC_2_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DAC_3_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void DAC_4_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void I2S_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PCC_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void AES_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void TRNG_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void ICM_Handler        ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void PUKCC_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void QSPI_Handler       ( void ) __attribute__ ((weak, alias("Dummy_Handler")));
void SDHC0_Handler      ( void ) __attribute__ ((weak, alias("Dummy_Handler")));

/* Exception Table */
__attribute__ ((section(".vectors")))
const DeviceVectors exception_table = {

    /* Configure Initial Stack Pointer, using linker-generated symbols */
    .pvStack = (void*) (&_estack),

    .pfnReset_Handler      = (void*) Reset_Handler,
    .pfnNonMaskableInt_Handler = (void*) NMI_Handler,
    .pfnHardFault_Handler  = (void*) HardFault_Handler,
    .pfnMemManagement_Handler = (void*) MemManage_Handler,
    .pfnBusFault_Handler   = (void*) BusFault_Handler,
    .pfnUsageFault_Handler = (void*) UsageFault_Handler,
    .pvReservedM9          = (void*) (0UL),
    .pvReservedM8          = (void*) (0UL),
    .pvReservedM7          = (void*) (0UL),
    .pvReservedM6          = (void*) (0UL),
    .pfnSVCall_Handler     = (void*) SVC_Handler,
    .pfnDebugMonitor_Handler = (void*) DebugMon_Handler,
    .pvReservedM3          = (void*) (0UL),
    .pfnPendSV_Handler     = (void*) PendSV_Handler,
    .pfnSysTick_Handler    = (void*) SysTick_Handler,

    /* Configurable interrupts - SAMD51J19A */
    .pfnPM_Handler         = (void*) PM_Handler,          /*  0 Power Manager */
    .pfnMCLK_Handler       = (void*) MCLK_Handler,        /*  1 Main Clock */
    .pfnOSCCTRL_0_Handler  = (void*) OSCCTRL_0_Handler,   /*  2 Oscillators Control IRQ 0 */
    .pfnOSCCTRL_1_Handler  = (void*) OSCCTRL_1_Handler,   /*  3 Oscillators Control IRQ 1 */
    .pfnOSCCTRL_2_Handler  = (void*) OSCCTRL_2_Handler,   /*  4 Oscillators Control IRQ 2 */
    .pfnOSCCTRL_3_Handler  = (void*) OSCCTRL_3_Handler,   /*  5 Oscillators Control IRQ 3 */
    .pfnOSCCTRL_4_Handler  = (void*) OSCCTRL_4_Handler,   /*  6 Oscillators Control IRQ 4 */
    .pfnOSC32KCTRL_Handler = (void*) OSC32KCTRL_Handler,  /*  7 32kHz Oscillators Control */
    .pfnSUPC_0_Handler     = (void*) SUPC_0_Handler,      /*  8 Supply Controller IRQ 0 */
    .pfnSUPC_1_Handler     = (void*) SUPC_1_Handler,      /*  9 Supply Controller IRQ 1 */
    .pfnWDT_Handler        = (void*) WDT_Handler,         /* 10 Watchdog Timer */
    .pfnRTC_Handler        = (void*) RTC_Handler,         /* 11 Real-Time Counter */
    .pfnEIC_0_Handler      = (void*) EIC_0_Handler,       /* 12 External Interrupt Controller IRQ 0 */
    .pfnEIC_1_Handler      = (void*) EIC_1_Handler,       /* 13 External Interrupt Controller IRQ 1 */
    .pfnEIC_2_Handler      = (void*) EIC_2_Handler,       /* 14 External Interrupt Controller IRQ 2 */
    .pfnEIC_3_Handler      = (void*) EIC_3_Handler,       /* 15 External Interrupt Controller IRQ 3 */
    .pfnEIC_4_Handler      = (void*) EIC_4_Handler,       /* 16 External Interrupt Controller IRQ 4 */
    .pfnEIC_5_Handler      = (void*) EIC_5_Handler,       /* 17 External Interrupt Controller IRQ 5 */
    .pfnEIC_6_Handler      = (void*) EIC_6_Handler,       /* 18 External Interrupt Controller IRQ 6 */
    .pfnEIC_7_Handler      = (void*) EIC_7_Handler,       /* 19 External Interrupt Controller IRQ 7 */
    .pfnEIC_8_Handler      = (void*) EIC_8_Handler,       /* 20 External Interrupt Controller IRQ 8 */
    .pfnEIC_9_Handler      = (void*) EIC_9_Handler,       /* 21 External Interrupt Controller IRQ 9 */
    .pfnEIC_10_Handler     = (void*) EIC_10_Handler,      /* 22 External Interrupt Controller IRQ 10 */
    .pfnEIC_11_Handler     = (void*) EIC_11_Handler,      /* 23 External Interrupt Controller IRQ 11 */
    .pfnEIC_12_Handler     = (void*) EIC_12_Handler,      /* 24 External Interrupt Controller IRQ 12 */
    .pfnEIC_13_Handler     = (void*) EIC_13_Handler,      /* 25 External Interrupt Controller IRQ 13 */
    .pfnEIC_14_Handler     = (void*) EIC_14_Handler,      /* 26 External Interrupt Controller IRQ 14 */
    .pfnEIC_15_Handler     = (void*) EIC_15_Handler,      /* 27 External Interrupt Controller IRQ 15 */
    .pfnFREQM_Handler      = (void*) FREQM_Handler,       /* 28 Frequency Meter */
    .pfnNVMCTRL_0_Handler  = (void*) NVMCTRL_0_Handler,   /* 29 Non-Volatile Memory Controller IRQ 0 */
    .pfnNVMCTRL_1_Handler  = (void*) NVMCTRL_1_Handler,   /* 30 Non-Volatile Memory Controller IRQ 1 */
    .pfnDMAC_0_Handler     = (void*) DMAC_0_Handler,      /* 31 Direct Memory Access Controller IRQ 0 */
    .pfnDMAC_1_Handler     = (void*) DMAC_1_Handler,      /* 32 Direct Memory Access Controller IRQ 1 */
    .pfnDMAC_2_Handler     = (void*) DMAC_2_Handler,      /* 33 Direct Memory Access Controller IRQ 2 */
    .pfnDMAC_3_Handler     = (void*) DMAC_3_Handler,      /* 34 Direct Memory Access Controller IRQ 3 */
    .pfnDMAC_4_Handler     = (void*) DMAC_4_Handler,      /* 35 Direct Memory Access Controller IRQ 4 */
    .pfnEVSYS_0_Handler    = (void*) EVSYS_0_Handler,     /* 36 Event System Interface IRQ 0 */
    .pfnEVSYS_1_Handler    = (void*) EVSYS_1_Handler,     /* 37 Event System Interface IRQ 1 */
    .pfnEVSYS_2_Handler    = (void*) EVSYS_2_Handler,     /* 38 Event System Interface IRQ 2 */
    .pfnEVSYS_3_Handler    = (void*) EVSYS_3_Handler,     /* 39 Event System Interface IRQ 3 */
    .pfnEVSYS_4_Handler    = (void*) EVSYS_4_Handler,     /* 40 Event System Interface IRQ 4 */
    .pfnPAC_Handler        = (void*) PAC_Handler,         /* 41 Peripheral Access Controller */
    .pvReserved42          = (void*) (0UL),               /* 42 Reserved */
    .pvReserved43          = (void*) (0UL),               /* 43 Reserved */
    .pvReserved44          = (void*) (0UL),               /* 44 Reserved */
    .pfnRAMECC_Handler     = (void*) RAMECC_Handler,      /* 45 RAM ECC */
    .pfnSERCOM0_0_Handler  = (void*) SERCOM0_0_Handler,   /* 46 Serial Communication Interface 0 IRQ 0 */
    .pfnSERCOM0_1_Handler  = (void*) SERCOM0_1_Handler,   /* 47 Serial Communication Interface 0 IRQ 1 */
    .pfnSERCOM0_2_Handler  = (void*) SERCOM0_2_Handler,   /* 48 Serial Communication Interface 0 IRQ 2 */
    .pfnSERCOM0_3_Handler  = (void*) SERCOM0_3_Handler,   /* 49 Serial Communication Interface 0 IRQ 3 */
    .pfnSERCOM1_0_Handler  = (void*) SERCOM1_0_Handler,   /* 50 Serial Communication Interface 1 IRQ 0 */
    .pfnSERCOM1_1_Handler  = (void*) SERCOM1_1_Handler,   /* 51 Serial Communication Interface 1 IRQ 1 */
    .pfnSERCOM1_2_Handler  = (void*) SERCOM1_2_Handler,   /* 52 Serial Communication Interface 1 IRQ 2 */
    .pfnSERCOM1_3_Handler  = (void*) SERCOM1_3_Handler,   /* 53 Serial Communication Interface 1 IRQ 3 */
    .pfnSERCOM2_0_Handler  = (void*) SERCOM2_0_Handler,   /* 54 Serial Communication Interface 2 IRQ 0 */
    .pfnSERCOM2_1_Handler  = (void*) SERCOM2_1_Handler,   /* 55 Serial Communication Interface 2 IRQ 1 */
    .pfnSERCOM2_2_Handler  = (void*) SERCOM2_2_Handler,   /* 56 Serial Communication Interface 2 IRQ 2 */
    .pfnSERCOM2_3_Handler  = (void*) SERCOM2_3_Handler,   /* 57 Serial Communication Interface 2 IRQ 3 */
    .pfnSERCOM3_0_Handler  = (void*) SERCOM3_0_Handler,   /* 58 Serial Communication Interface 3 IRQ 0 */
    .pfnSERCOM3_1_Handler  = (void*) SERCOM3_1_Handler,   /* 59 Serial Communication Interface 3 IRQ 1 */
    .pfnSERCOM3_2_Handler  = (void*) SERCOM3_2_Handler,   /* 60 Serial Communication Interface 3 IRQ 2 */
    .pfnSERCOM3_3_Handler  = (void*) SERCOM3_3_Handler,   /* 61 Serial Communication Interface 3 IRQ 3 */
    .pfnSERCOM4_0_Handler  = (void*) SERCOM4_0_Handler,   /* 62 Serial Communication Interface 4 IRQ 0 */
    .pfnSERCOM4_1_Handler  = (void*) SERCOM4_1_Handler,   /* 63 Serial Communication Interface 4 IRQ 1 */
    .pfnSERCOM4_2_Handler  = (void*) SERCOM4_2_Handler,   /* 64 Serial Communication Interface 4 IRQ 2 */
    .pfnSERCOM4_3_Handler  = (void*) SERCOM4_3_Handler,   /* 65 Serial Communication Interface 4 IRQ 3 */
    .pfnSERCOM5_0_Handler  = (void*) SERCOM5_0_Handler,   /* 66 Serial Communication Interface 5 IRQ 0 */
    .pfnSERCOM5_1_Handler  = (void*) SERCOM5_1_Handler,   /* 67 Serial Communication Interface 5 IRQ 1 */
    .pfnSERCOM5_2_Handler  = (void*) SERCOM5_2_Handler,   /* 68 Serial Communication Interface 5 IRQ 2 */
    .pfnSERCOM5_3_Handler  = (void*) SERCOM5_3_Handler,   /* 69 Serial Communication Interface 5 IRQ 3 */
    .pvReserved70          = (void*) (0UL),               /* 70 Reserved */
    .pvReserved71          = (void*) (0UL),               /* 71 Reserved */
    .pvReserved72          = (void*) (0UL),               /* 72 Reserved */
    .pvReserved73          = (void*) (0UL),               /* 73 Reserved */
    .pvReserved74          = (void*) (0UL),               /* 74 Reserved */
    .pvReserved75          = (void*) (0UL),               /* 75 Reserved */
    .pvReserved76          = (void*) (0UL),               /* 76 Reserved */
    .pvReserved77          = (void*) (0UL),               /* 77 Reserved */
    .pvReserved78          = (void*) (0UL),               /* 78 Reserved */
    .pvReserved79          = (void*) (0UL),               /* 79 Reserved */
    .pfnUSB_0_Handler      = (void*) USB_0_Handler,       /* 80 Universal Serial Bus IRQ 0 */
    .pfnUSB_1_Handler      = (void*) USB_1_Handler,       /* 81 Universal Serial Bus IRQ 1 */
    .pfnUSB_2_Handler      = (void*) USB_2_Handler,       /* 82 Universal Serial Bus IRQ 2 */
    .pfnUSB_3_Handler      = (void*) USB_3_Handler,       /* 83 Universal Serial Bus IRQ 3 */
    .pvReserved84          = (void*) (0UL),               /* 84 Reserved */
    .pfnTCC0_0_Handler     = (void*) TCC0_0_Handler,      /* 85 Timer Counter Control 0 IRQ 0 */
    .pfnTCC0_1_Handler     = (void*) TCC0_1_Handler,      /* 86 Timer Counter Control 0 IRQ 1 */
    .pfnTCC0_2_Handler     = (void*) TCC0_2_Handler,      /* 87 Timer Counter Control 0 IRQ 2 */
    .pfnTCC0_3_Handler     = (void*) TCC0_3_Handler,      /* 88 Timer Counter Control 0 IRQ 3 */
    .pfnTCC0_4_Handler     = (void*) TCC0_4_Handler,      /* 89 Timer Counter Control 0 IRQ 4 */
    .pfnTCC0_5_Handler     = (void*) TCC0_5_Handler,      /* 90 Timer Counter Control 0 IRQ 5 */
    .pfnTCC0_6_Handler     = (void*) TCC0_6_Handler,      /* 91 Timer Counter Control 0 IRQ 6 */
    .pfnTCC1_0_Handler     = (void*) TCC1_0_Handler,      /* 92 Timer Counter Control 1 IRQ 0 */
    .pfnTCC1_1_Handler     = (void*) TCC1_1_Handler,      /* 93 Timer Counter Control 1 IRQ 1 */
    .pfnTCC1_2_Handler     = (void*) TCC1_2_Handler,      /* 94 Timer Counter Control 1 IRQ 2 */
    .pfnTCC1_3_Handler     = (void*) TCC1_3_Handler,      /* 95 Timer Counter Control 1 IRQ 3 */
    .pfnTCC1_4_Handler     = (void*) TCC1_4_Handler,      /* 96 Timer Counter Control 1 IRQ 4 */
    .pfnTCC2_0_Handler     = (void*) TCC2_0_Handler,      /* 97 Timer Counter Control 2 IRQ 0 */
    .pfnTCC2_1_Handler     = (void*) TCC2_1_Handler,      /* 98 Timer Counter Control 2 IRQ 1 */
    .pfnTCC2_2_Handler     = (void*) TCC2_2_Handler,      /* 99 Timer Counter Control 2 IRQ 2 */
    .pfnTCC2_3_Handler     = (void*) TCC2_3_Handler,      /* 100 Timer Counter Control 2 IRQ 3 */
    .pfnTCC3_0_Handler     = (void*) TCC3_0_Handler,      /* 101 Timer Counter Control 3 IRQ 0 */
    .pfnTCC3_1_Handler     = (void*) TCC3_1_Handler,      /* 102 Timer Counter Control 3 IRQ 1 */
    .pfnTCC3_2_Handler     = (void*) TCC3_2_Handler,      /* 103 Timer Counter Control 3 IRQ 2 */
    .pfnTCC4_0_Handler     = (void*) TCC4_0_Handler,      /* 104 Timer Counter Control 4 IRQ 0 */
    .pfnTCC4_1_Handler     = (void*) TCC4_1_Handler,      /* 105 Timer Counter Control 4 IRQ 1 */
    .pfnTCC4_2_Handler     = (void*) TCC4_2_Handler,      /* 106 Timer Counter Control 4 IRQ 2 */
    .pfnTC0_Handler        = (void*) TC0_Handler,         /* 107 Basic Timer Counter 0 */
    .pfnTC1_Handler        = (void*) TC1_Handler,         /* 108 Basic Timer Counter 1 */
    .pfnTC2_Handler        = (void*) TC2_Handler,         /* 109 Basic Timer Counter 2 */
    .pfnTC3_Handler        = (void*) TC3_Handler,         /* 110 Basic Timer Counter 3 */
    .pfnTC4_Handler        = (void*) TC4_Handler,         /* 111 Basic Timer Counter 4 */
    .pfnTC5_Handler        = (void*) TC5_Handler,         /* 112 Basic Timer Counter 5 */
    .pvReserved113         = (void*) (0UL),               /* 113 Reserved */
    .pvReserved114         = (void*) (0UL),               /* 114 Reserved */
    .pfnPDEC_0_Handler     = (void*) PDEC_0_Handler,      /* 115 Quadrature Decoder IRQ 0 */
    .pfnPDEC_1_Handler     = (void*) PDEC_1_Handler,      /* 116 Quadrature Decoder IRQ 1 */
    .pfnPDEC_2_Handler     = (void*) PDEC_2_Handler,      /* 117 Quadrature Decoder IRQ 2 */
    .pfnADC0_0_Handler     = (void*) ADC0_0_Handler,      /* 118 Analog Digital Converter 0 IRQ 0 */
    .pfnADC0_1_Handler     = (void*) ADC0_1_Handler,      /* 119 Analog Digital Converter 0 IRQ 1 */
    .pfnADC1_0_Handler     = (void*) ADC1_0_Handler,      /* 120 Analog Digital Converter 1 IRQ 0 */
    .pfnADC1_1_Handler     = (void*) ADC1_1_Handler,      /* 121 Analog Digital Converter 1 IRQ 1 */
    .pfnAC_Handler         = (void*) AC_Handler,          /* 122 Analog Comparators */
    .pfnDAC_0_Handler      = (void*) DAC_0_Handler,       /* 123 Digital-to-Analog Converter IRQ 0 */
    .pfnDAC_1_Handler      = (void*) DAC_1_Handler,       /* 124 Digital-to-Analog Converter IRQ 1 */
    .pfnDAC_2_Handler      = (void*) DAC_2_Handler,       /* 125 Digital-to-Analog Converter IRQ 2 */
    .pfnDAC_3_Handler      = (void*) DAC_3_Handler,       /* 126 Digital-to-Analog Converter IRQ 3 */
    .pfnDAC_4_Handler      = (void*) DAC_4_Handler,       /* 127 Digital-to-Analog Converter IRQ 4 */
    .pfnI2S_Handler        = (void*) I2S_Handler,         /* 128 Inter-IC Sound Interface */
    .pfnPCC_Handler        = (void*) PCC_Handler,         /* 129 Parallel Capture Controller */
    .pfnAES_Handler        = (void*) AES_Handler,         /* 130 Advanced Encryption Standard */
    .pfnTRNG_Handler       = (void*) TRNG_Handler,        /* 131 True Random Generator */
    .pfnICM_Handler        = (void*) ICM_Handler,         /* 132 Integrity Check Monitor */
    .pfnPUKCC_Handler      = (void*) PUKCC_Handler,       /* 133 PUblic-Key Cryptography Controller */
    .pfnQSPI_Handler       = (void*) QSPI_Handler,        /* 134 Quad SPI interface */
    .pfnSDHC0_Handler      = (void*) SDHC0_Handler        /* 135 SD/MMC Host Controller 0 */
};

/**
 * \brief This is the code that gets called on processor reset.
 * To initialize the device, and call the main() routine.
 */
void Reset_Handler(void)
{
    uint32_t *pSrc, *pDest;

    /* Initialize the relocate segment */
    pSrc = &_etext;
    pDest = &_srelocate;

    if (pSrc != pDest) {
        for (; pDest < &_erelocate;) {
            *pDest++ = *pSrc++;
        }
    }

    /* Clear the zero segment */
    for (pDest = &_szero; pDest < &_ezero;) {
        *pDest++ = 0;
    }

    /* Set the vector table base address */
    pSrc = (uint32_t *) & _sfixed;
    __DSB();
    __ISB();
    SCB->VTOR = ((uint32_t) pSrc & SCB_VTOR_TBLOFF_Msk);
    __DSB();
    __ISB();

#if __FPU_USED
    /* Enable the FPU */
    __DSB();
    __ISB();
    SCB->CPACR |= ((3UL << 10*2) |             /* set CP10 Full Access */
                   (3UL << 11*2)  );           /* set CP11 Full Access */
    __DSB();
    __ISB();
#endif

    /* Initialize system clocks (120 MHz CPU, 48 MHz USB) */
    SystemInit();

    /* Initialize the C library */
    __libc_init_array();

    /* Branch to main function */
    main();

    /* Infinite loop */
    while (1);
}

/**
 * \brief Default interrupt handler for unused IRQs.
 */
void Dummy_Handler(void)
{
    while (1) {}
}

#ifdef __cplusplus
} // extern "C"
#endif
