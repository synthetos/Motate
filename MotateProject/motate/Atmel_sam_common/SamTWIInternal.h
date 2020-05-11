/*
 SamTWIInternal.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2018 Robert Giseburt

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

#ifndef SAMTWI_H_ONCE
#error This file should ONLY be included from SamTWI.h, and never included directly
#endif

// for the IDE to pickup these defines
#include "SamCommon.h"  // pull in defines and fix them

namespace Motate::TWI_internal {

template <int8_t twiPeripheralNumber>
struct TWIInfoBase {
    static constexpr bool exists = false;
};  // TWIInfo <generic>

#if defined(ID_TWIHS0)
    // This is for the SamS70
template <>
struct TWIInfoBase<0> {
    static constexpr bool exists = true;

    static constexpr uint32_t  peripheralId = ID_TWIHS0;
    static constexpr IRQn_Type IRQ          = TWIHS0_IRQn;

   protected:
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWIHS0)> twi{};
};  // TWIInfo <0>

// #undef TWIHS0 // leave for debugging
#undef ID_TWIHS0
#undef TWIHS0_IRQn

// for the sake of the cpp file to define the handlers
#define HAS_TWIHS0
#endif

#if defined(ID_TWIHS1)
// This is for the SamS70
template <>
struct TWIInfoBase<1> {
    static constexpr bool exists = true;

    static constexpr uint32_t  peripheralId = ID_TWIHS1;
    static constexpr IRQn_Type IRQ          = TWIHS1_IRQn;

   protected:
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWIHS1)> twi{};
};  // TWIInfo <1>

// #undef TWIHS1 // leave for debugging
#undef ID_TWIHS1
#undef TWIHS1_IRQn

// for the sake of the cpp file to define the handlers
#define HAS_TWIHS1
#endif

#if defined(HAS_TWIHS0) || defined(HAS_TWIHS1)
// This is for the SamS70
template <int8_t twiPeripheralNumber>
struct TWIInfo : TWIInfoBase<twiPeripheralNumber> {
    using TWIInfoBase<twiPeripheralNumber>::twi;

    void enableOnTXReadyInterrupt() { twi->TWIHS_IER = TWIHS_IER_TXRDY; }
    void disableOnTXReadyInterrupt() { twi->TWIHS_IDR = TWIHS_IDR_TXRDY; }

    void enableOnNACKInterrupt() { twi->TWIHS_IER = TWIHS_IER_NACK; }
    void disableOnNACKInterrupt() { twi->TWIHS_IDR = TWIHS_IDR_NACK; }

    void enableOnTXDoneInterrupt() { twi->TWIHS_IER = TWIHS_IER_TXCOMP; }
    void disableOnTXDoneInterrupt() { twi->TWIHS_IDR = TWIHS_IDR_TXCOMP; }

    void enableOnRXReadyInterrupt() {
        twi->TWIHS_IER = TWIHS_IER_RXRDY;
        twi->TWIHS_IER = TWIHS_IER_RXRDY;
    }
    void disableOnRXReadyInterrupt() { twi->TWIHS_IDR = TWIHS_IDR_RXRDY; }

    bool isTxReady(uint32_t sr = twi->TWIHS_SR) { return (sr & TWIHS_SR_TXRDY); }
    bool isTxComp(uint32_t sr = twi->TWIHS_SR) { return (sr & TWIHS_SR_TXCOMP); }
    bool isRxReady(uint32_t sr = twi->TWIHS_SR) { return (sr & TWIHS_SR_RXRDY); }
    bool isNack(uint32_t sr = twi->TWIHS_SR) { return (sr & TWIHS_SR_NACK); }

    auto getSR() { return twi->TWIHS_SR; }

    void resetModule() { twi->TWIHS_CR = TWIHS_CR_SWRST; }

    void enable() { twi->TWIHS_CR = TWIHS_CR_MSEN; }

    void disable() {
        twi->TWIHS_CR = TWIHS_CR_SVDIS;
        twi->TWIHS_CR = TWIHS_CR_MSDIS;
    }

    void setReading() { twi->TWIHS_MMR |= TWIHS_MMR_MREAD; }
    void setWriting() { twi->TWIHS_MMR &= ~TWIHS_MMR_MREAD; }

    void setStart() { twi->TWIHS_CR = TWIHS_CR_START; }

    void setStop() { twi->TWIHS_CR = TWIHS_CR_STOP; }

    void setStartStop() { twi->TWIHS_CR = TWIHS_CR_START | TWIHS_CR_STOP; }

    uint8_t readByte() { return twi->TWIHS_RHR; }

    void transmitChar(uint8_t b) { twi->TWIHS_THR = b; }

   protected:
    void _setAddress(uint8_t  adjusted_address,
                     uint32_t adjusted_internal_address,
                     uint8_t  adjusted_internal_address_size) {
        twi->TWIHS_MMR  = TWIHS_MMR_DADR(adjusted_address) | TWIHS_MMR_IADRSZ(adjusted_internal_address_size);
        twi->TWIHS_IADR = TWIHS_IADR_IADR(adjusted_internal_address);
    }

    /* Low level time limit of I2C Fast Mode. */
    static constexpr uint32_t LOW_LEVEL_TIME_LIMIT = 384000;
    static constexpr uint32_t I2C_FAST_MODE_SPEED  = 400000;
    static constexpr uint32_t TWIHS_CLK_DIVIDER    = 2;
    static constexpr uint32_t TWIHS_CLK_CALC_ARGU  = 3;
    static constexpr uint32_t TWIHS_CLK_DIV_MAX    = 0xFF;
    static constexpr uint32_t TWIHS_CLK_DIV_MIN    = 7;

    void setSpeed(const uint32_t speed = I2C_FAST_MODE_SPEED) {
        uint32_t ckdiv = 0;
        uint32_t c_lh_div;
        uint32_t cldiv, chdiv;
        auto     periph_clock = SamCommon::getPeripheralClockFreq();

        /* High-Speed can be only used in slave mode, 400k is the max speed allowed for master */
        if (speed > I2C_FAST_MODE_SPEED) {
            return;  // FAIL;
        }

        /* Low level time not less than 1.3us of I2C Fast Mode. */
        if (speed > LOW_LEVEL_TIME_LIMIT) {
            /* Low level of time fixed for 1.3us. */
            cldiv = periph_clock / (LOW_LEVEL_TIME_LIMIT * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;
            chdiv = periph_clock / ((speed + (speed - LOW_LEVEL_TIME_LIMIT)) * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;

            /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((cldiv > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                cldiv /= TWIHS_CLK_DIVIDER;
            }
            /* chdiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((chdiv > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                chdiv /= TWIHS_CLK_DIVIDER;
            }

            /* set clock waveform generator register */
            twi->TWIHS_CWGR = TWIHS_CWGR_CLDIV(cldiv) | TWIHS_CWGR_CHDIV(chdiv) | TWIHS_CWGR_CKDIV(ckdiv);
        } else {
            c_lh_div = periph_clock / (speed * TWIHS_CLK_DIVIDER) - TWIHS_CLK_CALC_ARGU;

            /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((c_lh_div > TWIHS_CLK_DIV_MAX) && (ckdiv < TWIHS_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                c_lh_div /= TWIHS_CLK_DIVIDER;
            }

            /* set clock waveform generator register */
            twi->TWIHS_CWGR = TWIHS_CWGR_CLDIV(c_lh_div) | TWIHS_CWGR_CHDIV(c_lh_div) | TWIHS_CWGR_CKDIV(ckdiv);
        }
    }
};  // TWIInfo <generic>
#endif

#if defined(ID_TWI0)
    // This is for the Sam3x
template <>
struct TWIInfoBase<0> {
    static constexpr bool exists = true;

    static constexpr uint32_t  peripheralId = ID_TWI0;
    static constexpr IRQn_Type IRQ          = TWI0_IRQn;

   protected:
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWI0)> twi{};
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_TWI0)> pdc{};
};  // TWIInfo <0>

// #undef TWIHS0 // leave for debugging
#undef ID_TWI0
#undef TWI0_IRQn

// for the sake of the cpp file to define the handlers
#define HAS_TWI0
#endif


#if defined(ID_TWI1)
// This is for the Sam3x
template <>
struct TWIInfoBase<1> {
    static constexpr bool exists = true;

    static constexpr uint32_t  peripheralId = ID_TWI1;
    static constexpr IRQn_Type IRQ          = TWI1_IRQn;

   protected:
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWI1)> twi{};
    static constexpr RegisterPtr<SEPARATE_OFF_CAST(PDC_TWI1)> pdc{};
};  // TWIInfo <1>

// #undef TWIHS1 // leave for debugging
#undef ID_TWI1
#undef TWI1_IRQn

// for the sake of the cpp file to define the handlers
#define HAS_TWI1
#endif

#if defined(HAS_TWI0) || defined(HAS_TWI1)
#define HAS_PDC_TWI
// This is for the Sam3x
template <int8_t twiPeripheralNumber>
struct TWIInfo : TWIInfoBase<twiPeripheralNumber> {
    using TWIInfoBase<twiPeripheralNumber>::twi;

    void enableOnTXReadyInterrupt() const { twi->TWI_IER = TWI_IER_TXRDY; }
    void disableOnTXReadyInterrupt() const { twi->TWI_IDR = TWI_IDR_TXRDY; }

    void enableOnNACKInterrupt() const { twi->TWI_IER = TWI_IER_NACK; }
    void disableOnNACKInterrupt() const { twi->TWI_IDR = TWI_IDR_NACK; }


    void enableOnTXDoneInterrupt() const { twi->TWI_IER = TWI_IER_TXCOMP; }
    void disableOnTXDoneInterrupt() const { twi->TWI_IDR = TWI_IDR_TXCOMP; }

    void enableOnRXReadyInterrupt() const { twi->TWI_IER = TWI_IER_RXRDY; }
    void disableOnRXReadyInterrupt() const { twi->TWI_IDR = TWI_IDR_RXRDY; }

    bool isTxReady(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_TXRDY); }
    bool isTxComp(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_TXCOMP); }
    bool isRxReady(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_RXRDY); }
    bool isNack(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_NACK); }

    bool isTxBufferEmpty(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_TXBUFE); }
    bool isTxBufferEnded(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_ENDTX); }

    bool isRxBufferFull(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_RXBUFF); }
    bool isRxBufferEnded(uint32_t sr = twi->TWI_SR) { return (sr & TWI_SR_ENDRX); }

    auto getSR() { return twi->TWI_SR; }

    void resetModule() { twi->TWI_CR = TWI_CR_SWRST; }

    void enable() { twi->TWI_CR = TWI_CR_MSEN; }

    void disable() {
        twi->TWI_CR = TWI_CR_SVDIS;
        twi->TWI_CR = TWI_CR_MSDIS;
    }

    void setReading() { twi->TWI_MMR |= TWI_MMR_MREAD; }
    void setWriting() { twi->TWI_MMR &= ~TWI_MMR_MREAD; }

    void setStart() { twi->TWI_CR = TWI_CR_START; }

    void setStop() { twi->TWI_CR = TWI_CR_STOP; }

    void setStartStop() { twi->TWI_CR = TWI_CR_START | TWI_CR_STOP; }

    uint8_t readByte() { return twi->TWI_RHR; }

    void transmitChar(uint8_t b) { twi->TWI_THR = b; }

   private:
    void _setAddress(uint8_t  adjusted_address,
                     uint32_t adjusted_internal_address,
                     uint8_t  adjusted_internal_address_size) {
        twi->TWI_MMR  = TWI_MMR_DADR(adjusted_address) | ((TWI_MMR_IADRSZ_Msk & ((adjusted_internal_address_size) << TWI_MMR_IADRSZ_Pos)));
        twi->TWI_IADR = TWI_IADR_IADR(adjusted_internal_address);
    }

    /* Low level time limit of I2C Fast Mode. */
    static constexpr uint32_t LOW_LEVEL_TIME_LIMIT = 384000;
    static constexpr uint32_t I2C_FAST_MODE_SPEED  = 400000;
    static constexpr uint32_t TWI_CLK_DIVIDER    = 2;
    static constexpr uint32_t TWI_CLK_CALC_ARGU  = 3;
    static constexpr uint32_t TWI_CLK_DIV_MAX    = 0xFF;
    static constexpr uint32_t TWI_CLK_DIV_MIN    = 7;

    void setSpeed(const uint32_t speed = I2C_FAST_MODE_SPEED) {
        uint32_t ckdiv = 0;
        uint32_t c_lh_div;
        uint32_t cldiv, chdiv;
        auto     periph_clock = SamCommon::getPeripheralClockFreq();

        /* High-Speed can be only used in slave mode, 400k is the max speed allowed for master */
        if (speed > I2C_FAST_MODE_SPEED) {
            return;  // FAIL;
        }

        /* Low level time not less than 1.3us of I2C Fast Mode. */
        if (speed > LOW_LEVEL_TIME_LIMIT) {
            /* Low level of time fixed for 1.3us. */
            cldiv = periph_clock / (LOW_LEVEL_TIME_LIMIT * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;
            chdiv = periph_clock / ((speed + (speed - LOW_LEVEL_TIME_LIMIT)) * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;

            /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((cldiv > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                cldiv /= TWI_CLK_DIVIDER;
            }
            /* chdiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((chdiv > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                chdiv /= TWI_CLK_DIVIDER;
            }

            /* set clock waveform generator register */
            twi->TWI_CWGR = TWI_CWGR_CLDIV(cldiv) | TWI_CWGR_CHDIV(chdiv) | TWI_CWGR_CKDIV(ckdiv);
        } else {
            c_lh_div = periph_clock / (speed * TWI_CLK_DIVIDER) - TWI_CLK_CALC_ARGU;

            /* cldiv must fit in 8 bits, ckdiv must fit in 3 bits */
            while ((c_lh_div > TWI_CLK_DIV_MAX) && (ckdiv < TWI_CLK_DIV_MIN)) {
                /* Increase clock divider */
                ckdiv++;
                /* Divide cldiv value */
                c_lh_div /= TWI_CLK_DIVIDER;
            }

            /* set clock waveform generator register */
            twi->TWI_CWGR = TWI_CWGR_CLDIV(c_lh_div) | TWI_CWGR_CHDIV(c_lh_div) | TWI_CWGR_CKDIV(ckdiv);
        }
    }
};  // TWIInfo <generic>
#endif

};  // namespace Motate::TWI_internal
