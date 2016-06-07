/*
 SamPins.cpp - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2014 - 2016 Robert Giseburt

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


#if defined(__SAM3X8E__) || defined(__SAM3X8C__)

#include "Atmel_sam3xa/SamPins.h"

using namespace Motate;

extern _pinChangeInterrupt motate_pin_change_interrupts_start;
extern _pinChangeInterrupt motate_pin_change_interrupts_end;


extern "C" void PIOA_Handler(void) {
    uint32_t isr = PIOA->PIO_ISR;

    _pinChangeInterrupt *current = &motate_pin_change_interrupts_start;
    while (current != &motate_pin_change_interrupts_end) {
        if (current->portLetter == 'A' && isr & current->mask) {
            current->interrupt();
        }
        current++;
    }

    NVIC_ClearPendingIRQ(PIOA_IRQn);
}


extern "C" void PIOB_Handler(void) {
    uint32_t isr = PIOB->PIO_ISR;

    _pinChangeInterrupt *current = &motate_pin_change_interrupts_start;
    while (current != &motate_pin_change_interrupts_end) {
        if (current->portLetter == 'B' && isr & current->mask) {
            current->interrupt();
        }
        current++;
    }

    NVIC_ClearPendingIRQ(PIOB_IRQn);
}

#ifdef PIOC
extern "C" void PIOC_Handler(void) {
    uint32_t isr = PIOC->PIO_ISR;

    _pinChangeInterrupt *current = &motate_pin_change_interrupts_start;
    while (current != &motate_pin_change_interrupts_end) {
        if (current->portLetter == 'C' && isr & current->mask) {
            current->interrupt();
        }
        current++;
    }

    NVIC_ClearPendingIRQ(PIOC_IRQn);
}

#endif // PORTC

#ifdef PIOD
extern "C" void PIOD_Handler(void) {
    uint32_t isr = PIOD->PIO_ISR;

    _pinChangeInterrupt *current = &motate_pin_change_interrupts_start;
    while (current != &motate_pin_change_interrupts_end) {
        if (current->portLetter == 'D' && isr & current->mask) {
            current->interrupt();
        }
        current++;
    }

    NVIC_ClearPendingIRQ(PIOD_IRQn);
}

#endif // PORTD


#ifdef ADC

extern "C" {
    void _null_adc_pin_interrupt() __attribute__ ((unused));
    void _null_adc_pin_interrupt() {};
}

namespace Motate {
    bool ADC_Module::inited_ = false;

    template<> Pio* Port32<'A'>::rawPort = PIOA;
    template<> const uint32_t Port32<'A'>::peripheralId() { return ID_PIOA; };
    template<> const IRQn_Type Port32<'A'>::_IRQn = PIOA_IRQn;

    template<> Pio* Port32<'B'>::rawPort = PIOB;
    template<> const uint32_t Port32<'B'>::peripheralId() { return ID_PIOB; };
    template<> const IRQn_Type Port32<'B'>::_IRQn = PIOA_IRQn;

#ifdef PIOC
    template<> Pio* Port32<'C'>::rawPort = PIOC;
    template<> const uint32_t Port32<'C'>::peripheralId() { return ID_PIOC; };
    template<> const IRQn_Type Port32<'C'>::_IRQn = PIOA_IRQn;
#endif

#ifdef PIOD
    template<> Pio* Port32<'D'>::rawPort = PIOD;
    template<> const uint32_t Port32<'D'>::peripheralId() { return ID_PIOD; };
    template<> const IRQn_Type Port32<'D'>::_IRQn = PIOA_IRQn;
#endif


    template<> void ADCPin< LookupADCPinByADC< 0>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 1>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 2>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 3>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 4>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 5>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 6>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 7>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 8>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC< 9>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC<10>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC<11>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC<12>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC<13>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
    template<> void ADCPin< LookupADCPinByADC<14>::number >::interrupt() __attribute__ ((weak, alias("_null_adc_pin_interrupt")));
}

extern "C" void ADC_Handler(void) {
    uint32_t isr = ADC->ADC_ISR; // read it to clear the ISR

//    uint32_t adc_value = ADC->ADC_LCDR;
//    uint32_t adc_num  = (adc_value & ADC_LCDR_CHNB_Msk) >> ADC_LCDR_CHNB_Pos;
//    adc_value = (adc_value & ADC_LCDR_LDATA_Msk) >> ADC_LCDR_LDATA_Pos;

#define _INTERNAL_MAKE_ADC_CHECK(num) \
if (ADCPin< LookupADCPinByADC<num>::number >::interrupt) { \
    if ((isr & LookupADCPinByADC<num>::adcMask)) { \
        LookupADCPinByADC<num>::interrupt(); \
    } \
}

    _INTERNAL_MAKE_ADC_CHECK( 0)
    _INTERNAL_MAKE_ADC_CHECK( 1)
    _INTERNAL_MAKE_ADC_CHECK( 2)
    _INTERNAL_MAKE_ADC_CHECK( 3)
    _INTERNAL_MAKE_ADC_CHECK( 4)
    _INTERNAL_MAKE_ADC_CHECK( 5)
    _INTERNAL_MAKE_ADC_CHECK( 6)
    _INTERNAL_MAKE_ADC_CHECK( 7)
    _INTERNAL_MAKE_ADC_CHECK( 8)
    _INTERNAL_MAKE_ADC_CHECK( 9)
    _INTERNAL_MAKE_ADC_CHECK(10)
    _INTERNAL_MAKE_ADC_CHECK(11)
    _INTERNAL_MAKE_ADC_CHECK(12)
    _INTERNAL_MAKE_ADC_CHECK(13)
    _INTERNAL_MAKE_ADC_CHECK(14)

//    NVIC_ClearPendingIRQ(ADC_IRQn);
}
#endif // ADC

#endif
