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

#include "MotatePins.h"

using namespace Motate;

template<> _pinChangeInterrupt * PortHardware<'A'>::_firstInterrupt = nullptr;
extern "C" void PIOA_Handler(void) {
    uint32_t isr = PIOA->PIO_ISR;

    _pinChangeInterrupt *current = PortHardware<'A'>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(PIOA_IRQn);
}

#ifdef PIOB
template<> _pinChangeInterrupt * PortHardware<'B'>::_firstInterrupt = nullptr;
extern "C" void PIOB_Handler(void) {
    uint32_t isr = PIOB->PIO_ISR;

    _pinChangeInterrupt *current = PortHardware<'B'>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(PIOB_IRQn);
}
#endif // PIOB

#ifdef PIOC
template<> _pinChangeInterrupt * PortHardware<'C'>::_firstInterrupt = nullptr;
extern "C" void PIOC_Handler(void) {
    uint32_t isr = PIOC->PIO_ISR;

    _pinChangeInterrupt *current = PortHardware<'C'>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(PIOC_IRQn);
}
#endif // PIOC

#ifdef PIOD
template<> _pinChangeInterrupt * PortHardware<'D'>::_firstInterrupt = nullptr;
extern "C" void PIOD_Handler(void) {
    uint32_t isr = PIOD->PIO_ISR;

    _pinChangeInterrupt *current = PortHardware<'D'>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(PIOD_IRQn);
}
#endif // PIOD

#ifdef ADC

extern "C" {
    void _null_adc_pin_interrupt() __attribute__ ((unused));
    void _null_adc_pin_interrupt() {};
}

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
namespace Motate {
    bool ADC_Module::_inited = false;
    bool ADC_Module::_firstInterrupt;
}

extern "C"
void ADC_Handler(void) {
    uint32_t isr = ADC->ADC_ISR; // read it to clear the ISR

    _pinChangeInterrupt *current = ADC_Module::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(ADC_IRQn);
} // ADC_Handler

#warning here (NOO)
#endif // sam3x
#endif // ADC

#ifdef AFEC0

//extern "C" {
//    void _null_adc_pin_interrupt() __attribute__ ((unused));
//    void _null_adc_pin_interrupt() {};
//}

namespace Motate {
    template<> bool ADC_Module<0l>::_inited = false;
    template<> _pinChangeInterrupt* ADC_Module<0l>::_firstInterrupt {};

    template<> bool ADC_Module<1l>::_inited = false;
    template<> _pinChangeInterrupt* ADC_Module<1l>::_firstInterrupt {};
}

extern "C"
void AFEC0_Handler(void) {
    uint32_t isr = AFEC0->AFEC_ISR; // read it to clear the ISR

    _pinChangeInterrupt *current = ADC_Module<0>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(AFEC0_IRQn);
} // AFEC0_Handler

extern "C"
void AFEC1_Handler(void) {
    uint32_t isr = AFEC1->AFEC_ISR; // read it to clear the ISR

    _pinChangeInterrupt *current = ADC_Module<1>::_firstInterrupt;
    while (current != nullptr) {
        if ((isr & current->pc_mask) && (current->interrupt_handler)) {
            current->interrupt_handler();
        }
        current = current->next;
    }

    NVIC_ClearPendingIRQ(AFEC1_IRQn);
} // AFEC1_Handler

#undef _INTERNAL_MAKE_AFEC_CHECK

#endif // AFEC0
