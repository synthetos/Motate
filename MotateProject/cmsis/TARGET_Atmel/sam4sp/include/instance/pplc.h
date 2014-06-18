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

#ifndef _SAM4SP_PPLC_INSTANCE_
#define _SAM4SP_PPLC_INSTANCE_

/* ========== Register definition for PPLC peripheral ========== */
#if (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__))
  #define REG_PPLC_CR                    (0x40008000U) /**< \brief (PPLC) Control Register */
  #define REG_PPLC_MR                    (0x40008004U) /**< \brief (PPLC) Mode Register */
  #define REG_PPLC_RDR                   (0x40008008U) /**< \brief (PPLC) Receive Data Register */
  #define REG_PPLC_TDR                   (0x4000800CU) /**< \brief (PPLC) Transmit Data Register */
  #define REG_PPLC_SR                    (0x40008010U) /**< \brief (PPLC) Status Register */
  #define REG_PPLC_IER                   (0x40008014U) /**< \brief (PPLC) Interrupt Enable Register */
  #define REG_PPLC_IDR                   (0x40008018U) /**< \brief (PPLC) Interrupt Disable Register */
  #define REG_PPLC_IMR                   (0x4000801CU) /**< \brief (PPLC) Interrupt Mask Register */
  #define REG_PPLC_CSR                   (0x40008030U) /**< \brief (PPLC) Chip Select Register */
  #define REG_PPLC_WPMR                  (0x400080E4U) /**< \brief (PPLC) Write Protection Control Register */
  #define REG_PPLC_WPSR                  (0x400080E8U) /**< \brief (PPLC) Write Protection Status Register */
  #define REG_PPLC_RPR                   (0x40008100U) /**< \brief (PPLC) Receive Pointer Register */
  #define REG_PPLC_RCR                   (0x40008104U) /**< \brief (PPLC) Receive Counter Register */
  #define REG_PPLC_TPR                   (0x40008108U) /**< \brief (PPLC) Transmit Pointer Register */
  #define REG_PPLC_TCR                   (0x4000810CU) /**< \brief (PPLC) Transmit Counter Register */
  #define REG_PPLC_RNPR                  (0x40008110U) /**< \brief (PPLC) Receive Next Pointer Register */
  #define REG_PPLC_RNCR                  (0x40008114U) /**< \brief (PPLC) Receive Next Counter Register */
  #define REG_PPLC_TNPR                  (0x40008118U) /**< \brief (PPLC) Transmit Next Pointer Register */
  #define REG_PPLC_TNCR                  (0x4000811CU) /**< \brief (PPLC) Transmit Next Counter Register */
  #define REG_PPLC_PTCR                  (0x40008120U) /**< \brief (PPLC) Transfer Control Register */
  #define REG_PPLC_PTSR                  (0x40008124U) /**< \brief (PPLC) Transfer Status Register */
#else
  #define REG_PPLC_CR   (*(__O  uint32_t*)0x40008000U) /**< \brief (PPLC) Control Register */
  #define REG_PPLC_MR   (*(__IO uint32_t*)0x40008004U) /**< \brief (PPLC) Mode Register */
  #define REG_PPLC_RDR  (*(__I  uint32_t*)0x40008008U) /**< \brief (PPLC) Receive Data Register */
  #define REG_PPLC_TDR  (*(__O  uint32_t*)0x4000800CU) /**< \brief (PPLC) Transmit Data Register */
  #define REG_PPLC_SR   (*(__I  uint32_t*)0x40008010U) /**< \brief (PPLC) Status Register */
  #define REG_PPLC_IER  (*(__O  uint32_t*)0x40008014U) /**< \brief (PPLC) Interrupt Enable Register */
  #define REG_PPLC_IDR  (*(__O  uint32_t*)0x40008018U) /**< \brief (PPLC) Interrupt Disable Register */
  #define REG_PPLC_IMR  (*(__I  uint32_t*)0x4000801CU) /**< \brief (PPLC) Interrupt Mask Register */
  #define REG_PPLC_CSR  (*(__IO uint32_t*)0x40008030U) /**< \brief (PPLC) Chip Select Register */
  #define REG_PPLC_WPMR (*(__IO uint32_t*)0x400080E4U) /**< \brief (PPLC) Write Protection Control Register */
  #define REG_PPLC_WPSR (*(__I  uint32_t*)0x400080E8U) /**< \brief (PPLC) Write Protection Status Register */
  #define REG_PPLC_RPR  (*(__IO uint32_t*)0x40008100U) /**< \brief (PPLC) Receive Pointer Register */
  #define REG_PPLC_RCR  (*(__IO uint32_t*)0x40008104U) /**< \brief (PPLC) Receive Counter Register */
  #define REG_PPLC_TPR  (*(__IO uint32_t*)0x40008108U) /**< \brief (PPLC) Transmit Pointer Register */
  #define REG_PPLC_TCR  (*(__IO uint32_t*)0x4000810CU) /**< \brief (PPLC) Transmit Counter Register */
  #define REG_PPLC_RNPR (*(__IO uint32_t*)0x40008110U) /**< \brief (PPLC) Receive Next Pointer Register */
  #define REG_PPLC_RNCR (*(__IO uint32_t*)0x40008114U) /**< \brief (PPLC) Receive Next Counter Register */
  #define REG_PPLC_TNPR (*(__IO uint32_t*)0x40008118U) /**< \brief (PPLC) Transmit Next Pointer Register */
  #define REG_PPLC_TNCR (*(__IO uint32_t*)0x4000811CU) /**< \brief (PPLC) Transmit Next Counter Register */
  #define REG_PPLC_PTCR (*(__O  uint32_t*)0x40008120U) /**< \brief (PPLC) Transfer Control Register */
  #define REG_PPLC_PTSR (*(__I  uint32_t*)0x40008124U) /**< \brief (PPLC) Transfer Status Register */
#endif /* (defined(__ASSEMBLY__) || defined(__IAR_SYSTEMS_ASM__)) */

#endif /* _SAM4SP_PPLC_INSTANCE_ */
