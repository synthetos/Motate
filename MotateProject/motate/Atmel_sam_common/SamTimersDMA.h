/*
 SamTimersDMA.h - Library for the Motate system
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

#ifndef SAMTIMERS_H_ONCE
#error This file should ONLY be included from SamSPI.h, and never included directly
#endif

#include "SamDMA.h"

namespace Motate {
#if defined(PWM1) && !defined(PWM_PTCR_TXTEN)

// TODO: PDC DMA implementation, which is when PWM_PTCR_TXTEN is defined

#pragma mark DMA_XDMAC PWM implementation

    template<uint8_t timerNum>
    struct DMA_XDMAC_hardware<Pwm*, timerNum>
    {
        static constexpr Pwm * const pwm() {
            if (timerNum < 8) { return PWM0; }
            else              { return PWM1; }
        };
        static constexpr PwmCh_num * const pwmChan()
        {
            if (timerNum < 8) { return PWM0->PWM_CH_NUM + timerNum; }
            else              { return PWM1->PWM_CH_NUM + (timerNum-8); }
        };

        typedef uint16_t* buffer_t;
    };

    template<uint8_t timerNum>
    struct DMA_XDMAC_TX_hardware<Pwm*, timerNum> : virtual DMA_XDMAC_hardware<Pwm*, timerNum>, virtual DMA_XDMAC_common {
        using DMA_XDMAC_hardware<Pwm*, timerNum>::pwm;

        static constexpr uint8_t const xdmaTxPeripheralId()
        {
            if (timerNum < 8) { return 13; }
            else              { return 39; }
        };
        static constexpr uint8_t const xdmaTxChannelNumber()
        {
            switch (timerNum) {
                case (0): return  16;
                case (1): return  17;
            };
            return 0;
        };
        static constexpr XdmacChid * const xdmaTxChannel()
        {
            return xdma()->XDMAC_CHID + xdmaTxChannelNumber();
        };
        static constexpr volatile void * const xdmaPeripheralTxAddress()
        {
            return &(pwm()->PWM_DMAR);
        };
    };

    // Construct a DMA specialization that uses the PDC
    template<uint8_t periph_num>
    struct DMA<Pwm*, periph_num> : DMA_XDMAC_TX<Pwm*, periph_num> {
        // nothing to do here, except for a constxpr constructor
        constexpr DMA(const std::function<void(Interrupt::Type)> &handler) : DMA_XDMAC_TX<Pwm*, periph_num>{handler} {};
        constexpr DMA() : DMA_XDMAC_TX<Pwm*, periph_num>{nullptr} {};

        void setInterrupts(const Interrupt::Type interrupts) const
        {
            DMA_XDMAC_common::setInterrupts(interrupts);

            if (interrupts != Interrupt::Off) {
                if (interrupts & Interrupt::OnTxTransferDone) {
                    DMA_XDMAC_TX<Pwm*, periph_num>::startTxDoneInterrupts();
                } else {
                    DMA_XDMAC_TX<Pwm*, periph_num>::stopTxDoneInterrupts();
                }

//                if (interrupts & Interrupt::OnRxTransferDone) {
//                    startRxDoneInterrupts();
//                } else {
//                    stopRxDoneInterrupts();
//                }
            }
        };


        void reset() const {
            DMA_XDMAC_TX<Pwm*, periph_num>::resetTX();
        };

    };
#endif // PWM + XDMAC
} // namespace Motate
