/* ---------------------------------------------------------------------------- */
/*                  Atmel Microcontroller Software Support                      */
/*                       SAM Software Package License                           */
/* ---------------------------------------------------------------------------- */
/* Copyright (c) 2014, Atmel Corporation                                        */
/*                                                                              */
/* All rights reserved.                                                         */
/*                                                                              */
/* Redistribution and use in source and binary forms, with or without           */
/* modification, are permitted provided that the following condition is met:    */
/*                                                                              */
/* - Redistributions of source code must retain the above copyright notice,     */
/* this list of conditions and the disclaimer below.                            */
/*                                                                              */
/* Atmel's name may not be used to endorse or promote products derived from     */
/* this software without specific prior written permission.                     */
/*                                                                              */
/* DISCLAIMER:  THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR   */
/* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE   */
/* DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,      */
/* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,  */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    */
/* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING         */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, */
/* EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                           */
/* ---------------------------------------------------------------------------- */

#ifndef _SAM4SP_PPLC_COMPONENT_
#define _SAM4SP_PPLC_COMPONENT_

/* ============================================================================= */
/**  SOFTWARE API DEFINITION FOR PRIME Power Line Communication (PPLC) Transceiver */
/* ============================================================================= */
/** \addtogroup SAM4SP_PPLC PRIME Power Line Communication (PPLC) Transceiver */
/*@{*/

#if !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
/** \brief Pplc hardware registers */
typedef struct {
  __O  uint32_t PPLC_CR;       /**< \brief (Pplc Offset: 0x00) Control Register */
  __IO uint32_t PPLC_MR;       /**< \brief (Pplc Offset: 0x04) Mode Register */
  __I  uint32_t PPLC_RDR;      /**< \brief (Pplc Offset: 0x08) Receive Data Register */
  __O  uint32_t PPLC_TDR;      /**< \brief (Pplc Offset: 0x0C) Transmit Data Register */
  __I  uint32_t PPLC_SR;       /**< \brief (Pplc Offset: 0x10) Status Register */
  __O  uint32_t PPLC_IER;      /**< \brief (Pplc Offset: 0x14) Interrupt Enable Register */
  __O  uint32_t PPLC_IDR;      /**< \brief (Pplc Offset: 0x18) Interrupt Disable Register */
  __I  uint32_t PPLC_IMR;      /**< \brief (Pplc Offset: 0x1C) Interrupt Mask Register */
  __I  uint32_t Reserved1[4];
  __IO uint32_t PPLC_CSR;      /**< \brief (Pplc Offset: 0x30) Chip Select Register */
  __I  uint32_t Reserved2[44];
  __IO uint32_t PPLC_WPMR;     /**< \brief (Pplc Offset: 0xE4) Write Protection Control Register */
  __I  uint32_t PPLC_WPSR;     /**< \brief (Pplc Offset: 0xE8) Write Protection Status Register */
  __I  uint32_t Reserved3[5];
  __IO uint32_t PPLC_RPR;      /**< \brief (Pplc Offset: 0x100) Receive Pointer Register */
  __IO uint32_t PPLC_RCR;      /**< \brief (Pplc Offset: 0x104) Receive Counter Register */
  __IO uint32_t PPLC_TPR;      /**< \brief (Pplc Offset: 0x108) Transmit Pointer Register */
  __IO uint32_t PPLC_TCR;      /**< \brief (Pplc Offset: 0x10C) Transmit Counter Register */
  __IO uint32_t PPLC_RNPR;     /**< \brief (Pplc Offset: 0x110) Receive Next Pointer Register */
  __IO uint32_t PPLC_RNCR;     /**< \brief (Pplc Offset: 0x114) Receive Next Counter Register */
  __IO uint32_t PPLC_TNPR;     /**< \brief (Pplc Offset: 0x118) Transmit Next Pointer Register */
  __IO uint32_t PPLC_TNCR;     /**< \brief (Pplc Offset: 0x11C) Transmit Next Counter Register */
  __O  uint32_t PPLC_PTCR;     /**< \brief (Pplc Offset: 0x120) Transfer Control Register */
  __I  uint32_t PPLC_PTSR;     /**< \brief (Pplc Offset: 0x124) Transfer Status Register */
} Pplc;
#endif /* !(defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */
/* -------- PPLC_CR : (PPLC Offset: 0x00) Control Register -------- */
#define PPLC_CR_PPLCEN (0x1u << 0) /**< \brief (PPLC_CR) PPLC Enable */
#define PPLC_CR_PPLCDIS (0x1u << 1) /**< \brief (PPLC_CR) PPLC Disable */
#define PPLC_CR_SWRST (0x1u << 7) /**< \brief (PPLC_CR) PPLC Software Reset */
#define PPLC_CR_LASTXFER (0x1u << 24) /**< \brief (PPLC_CR) Last Transfer */
/* -------- PPLC_MR : (PPLC Offset: 0x04) Mode Register -------- */
#define PPLC_MR_MSTR (0x1u << 0) /**< \brief (PPLC_MR) Active Mode */
#define PPLC_MR_PS (0x1u << 1) /**< \brief (PPLC_MR) Peripheral Select */
#define PPLC_MR_PCSDEC (0x1u << 2) /**< \brief (PPLC_MR) Chip Select Decode */
#define PPLC_MR_MODFDIS (0x1u << 4) /**< \brief (PPLC_MR) Mode Fault Detection */
#define PPLC_MR_WDRBT (0x1u << 5) /**< \brief (PPLC_MR) Wait Data Read Before Transfer */
#define PPLC_MR_LLB (0x1u << 7) /**< \brief (PPLC_MR) Local Loopback Enable */
#define PPLC_MR_PCS_Pos 16
#define PPLC_MR_PCS_Msk (0xfu << PPLC_MR_PCS_Pos) /**< \brief (PPLC_MR) Peripheral Chip Select */
#define PPLC_MR_PCS(value) ((PPLC_MR_PCS_Msk & ((value) << PPLC_MR_PCS_Pos)))
#define PPLC_MR_DLYBCS_Pos 24
#define PPLC_MR_DLYBCS_Msk (0xffu << PPLC_MR_DLYBCS_Pos) /**< \brief (PPLC_MR) Delay Between Chip Selects */
#define PPLC_MR_DLYBCS(value) ((PPLC_MR_DLYBCS_Msk & ((value) << PPLC_MR_DLYBCS_Pos)))
/* -------- PPLC_RDR : (PPLC Offset: 0x08) Receive Data Register -------- */
#define PPLC_RDR_RD_Pos 0
#define PPLC_RDR_RD_Msk (0xffffu << PPLC_RDR_RD_Pos) /**< \brief (PPLC_RDR) Receive Data */
#define PPLC_RDR_PCS_Pos 16
#define PPLC_RDR_PCS_Msk (0xfu << PPLC_RDR_PCS_Pos) /**< \brief (PPLC_RDR) Peripheral Chip Select */
/* -------- PPLC_TDR : (PPLC Offset: 0x0C) Transmit Data Register -------- */
#define PPLC_TDR_TD_Pos 0
#define PPLC_TDR_TD_Msk (0xffffu << PPLC_TDR_TD_Pos) /**< \brief (PPLC_TDR) Transmit Data */
#define PPLC_TDR_TD(value) ((PPLC_TDR_TD_Msk & ((value) << PPLC_TDR_TD_Pos)))
#define PPLC_TDR_PCS_Pos 16
#define PPLC_TDR_PCS_Msk (0xfu << PPLC_TDR_PCS_Pos) /**< \brief (PPLC_TDR) Peripheral Chip Select */
#define PPLC_TDR_PCS(value) ((PPLC_TDR_PCS_Msk & ((value) << PPLC_TDR_PCS_Pos)))
#define PPLC_TDR_LASTXFER (0x1u << 24) /**< \brief (PPLC_TDR) Last Transfer */
/* -------- PPLC_SR : (PPLC Offset: 0x10) Status Register -------- */
#define PPLC_SR_RDRF (0x1u << 0) /**< \brief (PPLC_SR) Receive Data Register Full */
#define PPLC_SR_TDRE (0x1u << 1) /**< \brief (PPLC_SR) Transmit Data Register Empty */
#define PPLC_SR_MODF (0x1u << 2) /**< \brief (PPLC_SR) Mode Fault Error */
#define PPLC_SR_OVRES (0x1u << 3) /**< \brief (PPLC_SR) Overrun Error Status */
#define PPLC_SR_ENDRX (0x1u << 4) /**< \brief (PPLC_SR) End of RX buffer */
#define PPLC_SR_ENDTX (0x1u << 5) /**< \brief (PPLC_SR) End of TX buffer */
#define PPLC_SR_RXBUFF (0x1u << 6) /**< \brief (PPLC_SR) RX Buffer Full */
#define PPLC_SR_TXBUFE (0x1u << 7) /**< \brief (PPLC_SR) TX Buffer Empty */
#define PPLC_SR_NSSR (0x1u << 8) /**< \brief (PPLC_SR) NSS Rising */
#define PPLC_SR_TXEMPTY (0x1u << 9) /**< \brief (PPLC_SR) Transmission Registers Empty */
#define PPLC_SR_UNDES (0x1u << 10) /**< \brief (PPLC_SR) Underrun Error Status (Slave Mode Only) */
#define PPLC_SR_PPLCENS (0x1u << 16) /**< \brief (PPLC_SR) PPLC Enable Status */
/* -------- PPLC_IER : (PPLC Offset: 0x14) Interrupt Enable Register -------- */
#define PPLC_IER_RDRF (0x1u << 0) /**< \brief (PPLC_IER) Receive Data Register Full Interrupt Enable */
#define PPLC_IER_TDRE (0x1u << 1) /**< \brief (PPLC_IER) PPLC Transmit Data Register Empty Interrupt Enable */
#define PPLC_IER_MODF (0x1u << 2) /**< \brief (PPLC_IER) Mode Fault Error Interrupt Enable */
#define PPLC_IER_OVRES (0x1u << 3) /**< \brief (PPLC_IER) Overrun Error Interrupt Enable */
#define PPLC_IER_ENDRX (0x1u << 4) /**< \brief (PPLC_IER) End of Receive Buffer Interrupt Enable */
#define PPLC_IER_ENDTX (0x1u << 5) /**< \brief (PPLC_IER) End of Transmit Buffer Interrupt Enable */
#define PPLC_IER_RXBUFF (0x1u << 6) /**< \brief (PPLC_IER) Receive Buffer Full Interrupt Enable */
#define PPLC_IER_TXBUFE (0x1u << 7) /**< \brief (PPLC_IER) Transmit Buffer Empty Interrupt Enable */
#define PPLC_IER_NSSR (0x1u << 8) /**< \brief (PPLC_IER) NSS Rising Interrupt Enable */
#define PPLC_IER_TXEMPTY (0x1u << 9) /**< \brief (PPLC_IER) Transmission Registers Empty Enable */
#define PPLC_IER_UNDES (0x1u << 10) /**< \brief (PPLC_IER) Underrun Error Interrupt Enable */
/* -------- PPLC_IDR : (PPLC Offset: 0x18) Interrupt Disable Register -------- */
#define PPLC_IDR_RDRF (0x1u << 0) /**< \brief (PPLC_IDR) Receive Data Register Full Interrupt Disable */
#define PPLC_IDR_TDRE (0x1u << 1) /**< \brief (PPLC_IDR) PPLC Transmit Data Register Empty Interrupt Disable */
#define PPLC_IDR_MODF (0x1u << 2) /**< \brief (PPLC_IDR) Mode Fault Error Interrupt Disable */
#define PPLC_IDR_OVRES (0x1u << 3) /**< \brief (PPLC_IDR) Overrun Error Interrupt Disable */
#define PPLC_IDR_ENDRX (0x1u << 4) /**< \brief (PPLC_IDR) End of Receive Buffer Interrupt Disable */
#define PPLC_IDR_ENDTX (0x1u << 5) /**< \brief (PPLC_IDR) End of Transmit Buffer Interrupt Disable */
#define PPLC_IDR_RXBUFF (0x1u << 6) /**< \brief (PPLC_IDR) Receive Buffer Full Interrupt Disable */
#define PPLC_IDR_TXBUFE (0x1u << 7) /**< \brief (PPLC_IDR) Transmit Buffer Empty Interrupt Disable */
#define PPLC_IDR_NSSR (0x1u << 8) /**< \brief (PPLC_IDR) NSS Rising Interrupt Disable */
#define PPLC_IDR_TXEMPTY (0x1u << 9) /**< \brief (PPLC_IDR) Transmission Registers Empty Disable */
#define PPLC_IDR_UNDES (0x1u << 10) /**< \brief (PPLC_IDR) Underrun Error Interrupt Disable */
/* -------- PPLC_IMR : (PPLC Offset: 0x1C) Interrupt Mask Register -------- */
#define PPLC_IMR_RDRF (0x1u << 0) /**< \brief (PPLC_IMR) Receive Data Register Full Interrupt Mask */
#define PPLC_IMR_TDRE (0x1u << 1) /**< \brief (PPLC_IMR) PPLC Transmit Data Register Empty Interrupt Mask */
#define PPLC_IMR_MODF (0x1u << 2) /**< \brief (PPLC_IMR) Mode Fault Error Interrupt Mask */
#define PPLC_IMR_OVRES (0x1u << 3) /**< \brief (PPLC_IMR) Overrun Error Interrupt Mask */
#define PPLC_IMR_ENDRX (0x1u << 4) /**< \brief (PPLC_IMR) End of Receive Buffer Interrupt Mask */
#define PPLC_IMR_ENDTX (0x1u << 5) /**< \brief (PPLC_IMR) End of Transmit Buffer Interrupt Mask */
#define PPLC_IMR_RXBUFF (0x1u << 6) /**< \brief (PPLC_IMR) Receive Buffer Full Interrupt Mask */
#define PPLC_IMR_TXBUFE (0x1u << 7) /**< \brief (PPLC_IMR) Transmit Buffer Empty Interrupt Mask */
#define PPLC_IMR_NSSR (0x1u << 8) /**< \brief (PPLC_IMR) NSS Rising Interrupt Mask */
#define PPLC_IMR_TXEMPTY (0x1u << 9) /**< \brief (PPLC_IMR) Transmission Registers Empty Mask */
#define PPLC_IMR_UNDES (0x1u << 10) /**< \brief (PPLC_IMR) Underrun Error Interrupt Mask */
/* -------- PPLC_CSR : (PPLC Offset: 0x30) Chip Select Register -------- */
#define PPLC_CSR_CPOL (0x1u << 0) /**< \brief (PPLC_CSR) Clock Polarity */
#define PPLC_CSR_NCPHA (0x1u << 1) /**< \brief (PPLC_CSR) Clock Phase */
#define PPLC_CSR_CSNAAT (0x1u << 2) /**< \brief (PPLC_CSR) Chip Select Not Active After Transfer (Ignored if CSAAT = 1) */
#define PPLC_CSR_CSAAT (0x1u << 3) /**< \brief (PPLC_CSR) Chip Select Active After Transfer */
#define PPLC_CSR_BITS_Pos 4
#define PPLC_CSR_BITS_Msk (0xfu << PPLC_CSR_BITS_Pos) /**< \brief (PPLC_CSR) Bits Per Transfer */
#define PPLC_CSR_BITS(value) ((PPLC_CSR_BITS_Msk & ((value) << PPLC_CSR_BITS_Pos)))
#define PPLC_CSR_SCBR_Pos 8
#define PPLC_CSR_SCBR_Msk (0xffu << PPLC_CSR_SCBR_Pos) /**< \brief (PPLC_CSR) Serial Clock Baud Rate */
#define PPLC_CSR_SCBR(value) ((PPLC_CSR_SCBR_Msk & ((value) << PPLC_CSR_SCBR_Pos)))
#define PPLC_CSR_DLYBS_Pos 16
#define PPLC_CSR_DLYBS_Msk (0xffu << PPLC_CSR_DLYBS_Pos) /**< \brief (PPLC_CSR) Delay Before SPCK */
#define PPLC_CSR_DLYBS(value) ((PPLC_CSR_DLYBS_Msk & ((value) << PPLC_CSR_DLYBS_Pos)))
#define PPLC_CSR_DLYBCT_Pos 24
#define PPLC_CSR_DLYBCT_Msk (0xffu << PPLC_CSR_DLYBCT_Pos) /**< \brief (PPLC_CSR) Delay Between Consecutive Transfers */
#define PPLC_CSR_DLYBCT(value) ((PPLC_CSR_DLYBCT_Msk & ((value) << PPLC_CSR_DLYBCT_Pos)))
/* -------- PPLC_WPMR : (PPLC Offset: 0xE4) Write Protection Control Register -------- */
#define PPLC_WPMR_WPEN (0x1u << 0) /**< \brief (PPLC_WPMR) Write Protection Enable */
#define PPLC_WPMR_WPKEY_Pos 8
#define PPLC_WPMR_WPKEY_Msk (0xffffffu << PPLC_WPMR_WPKEY_Pos) /**< \brief (PPLC_WPMR) Write Protection Key Password */
#define PPLC_WPMR_WPKEY(value) ((PPLC_WPMR_WPKEY_Msk & ((value) << PPLC_WPMR_WPKEY_Pos)))
/* -------- PPLC_WPSR : (PPLC Offset: 0xE8) Write Protection Status Register -------- */
#define PPLC_WPSR_WPVS (0x1u << 0) /**< \brief (PPLC_WPSR) Write Protection Violation Status */
#define PPLC_WPSR_WPVSRC_Pos 8
#define PPLC_WPSR_WPVSRC_Msk (0xffu << PPLC_WPSR_WPVSRC_Pos) /**< \brief (PPLC_WPSR) Write Protection Violation Source */
/* -------- PPLC_RPR : (PPLC Offset: 0x100) Receive Pointer Register -------- */
#define PPLC_RPR_RXPTR_Pos 0
#define PPLC_RPR_RXPTR_Msk (0xffffffffu << PPLC_RPR_RXPTR_Pos) /**< \brief (PPLC_RPR) Receive Pointer Register */
#define PPLC_RPR_RXPTR(value) ((PPLC_RPR_RXPTR_Msk & ((value) << PPLC_RPR_RXPTR_Pos)))
/* -------- PPLC_RCR : (PPLC Offset: 0x104) Receive Counter Register -------- */
#define PPLC_RCR_RXCTR_Pos 0
#define PPLC_RCR_RXCTR_Msk (0xffffu << PPLC_RCR_RXCTR_Pos) /**< \brief (PPLC_RCR) Receive Counter Register */
#define PPLC_RCR_RXCTR(value) ((PPLC_RCR_RXCTR_Msk & ((value) << PPLC_RCR_RXCTR_Pos)))
/* -------- PPLC_TPR : (PPLC Offset: 0x108) Transmit Pointer Register -------- */
#define PPLC_TPR_TXPTR_Pos 0
#define PPLC_TPR_TXPTR_Msk (0xffffffffu << PPLC_TPR_TXPTR_Pos) /**< \brief (PPLC_TPR) Transmit Counter Register */
#define PPLC_TPR_TXPTR(value) ((PPLC_TPR_TXPTR_Msk & ((value) << PPLC_TPR_TXPTR_Pos)))
/* -------- PPLC_TCR : (PPLC Offset: 0x10C) Transmit Counter Register -------- */
#define PPLC_TCR_TXCTR_Pos 0
#define PPLC_TCR_TXCTR_Msk (0xffffu << PPLC_TCR_TXCTR_Pos) /**< \brief (PPLC_TCR) Transmit Counter Register */
#define PPLC_TCR_TXCTR(value) ((PPLC_TCR_TXCTR_Msk & ((value) << PPLC_TCR_TXCTR_Pos)))
/* -------- PPLC_RNPR : (PPLC Offset: 0x110) Receive Next Pointer Register -------- */
#define PPLC_RNPR_RXNPTR_Pos 0
#define PPLC_RNPR_RXNPTR_Msk (0xffffffffu << PPLC_RNPR_RXNPTR_Pos) /**< \brief (PPLC_RNPR) Receive Next Pointer */
#define PPLC_RNPR_RXNPTR(value) ((PPLC_RNPR_RXNPTR_Msk & ((value) << PPLC_RNPR_RXNPTR_Pos)))
/* -------- PPLC_RNCR : (PPLC Offset: 0x114) Receive Next Counter Register -------- */
#define PPLC_RNCR_RXNCTR_Pos 0
#define PPLC_RNCR_RXNCTR_Msk (0xffffu << PPLC_RNCR_RXNCTR_Pos) /**< \brief (PPLC_RNCR) Receive Next Counter */
#define PPLC_RNCR_RXNCTR(value) ((PPLC_RNCR_RXNCTR_Msk & ((value) << PPLC_RNCR_RXNCTR_Pos)))
/* -------- PPLC_TNPR : (PPLC Offset: 0x118) Transmit Next Pointer Register -------- */
#define PPLC_TNPR_TXNPTR_Pos 0
#define PPLC_TNPR_TXNPTR_Msk (0xffffffffu << PPLC_TNPR_TXNPTR_Pos) /**< \brief (PPLC_TNPR) Transmit Next Pointer */
#define PPLC_TNPR_TXNPTR(value) ((PPLC_TNPR_TXNPTR_Msk & ((value) << PPLC_TNPR_TXNPTR_Pos)))
/* -------- PPLC_TNCR : (PPLC Offset: 0x11C) Transmit Next Counter Register -------- */
#define PPLC_TNCR_TXNCTR_Pos 0
#define PPLC_TNCR_TXNCTR_Msk (0xffffu << PPLC_TNCR_TXNCTR_Pos) /**< \brief (PPLC_TNCR) Transmit Counter Next */
#define PPLC_TNCR_TXNCTR(value) ((PPLC_TNCR_TXNCTR_Msk & ((value) << PPLC_TNCR_TXNCTR_Pos)))
/* -------- PPLC_PTCR : (PPLC Offset: 0x120) Transfer Control Register -------- */
#define PPLC_PTCR_RXTEN (0x1u << 0) /**< \brief (PPLC_PTCR) Receiver Transfer Enable */
#define PPLC_PTCR_RXTDIS (0x1u << 1) /**< \brief (PPLC_PTCR) Receiver Transfer Disable */
#define PPLC_PTCR_TXTEN (0x1u << 8) /**< \brief (PPLC_PTCR) Transmitter Transfer Enable */
#define PPLC_PTCR_TXTDIS (0x1u << 9) /**< \brief (PPLC_PTCR) Transmitter Transfer Disable */
/* -------- PPLC_PTSR : (PPLC Offset: 0x124) Transfer Status Register -------- */
#define PPLC_PTSR_RXTEN (0x1u << 0) /**< \brief (PPLC_PTSR) Receiver Transfer Enable */
#define PPLC_PTSR_TXTEN (0x1u << 8) /**< \brief (PPLC_PTSR) Transmitter Transfer Enable */

/*@}*/


#endif /* _SAM4SP_PPLC_COMPONENT_ */
