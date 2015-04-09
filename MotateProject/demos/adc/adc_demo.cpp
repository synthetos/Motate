/*
 * adc_demo.cpp - Motate
 * This file is part of the Motate project.
 *
 * Copyright (c) 2015c Robert Giseburt
 *
 *  This file is part of the Motate Library.
 *
 *  This file ("the software") is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License, version 2 as published by the
 *  Free Software Foundation. You should have received a copy of the GNU General Public
 *  License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 *  As a special exception, you may use this file as part of a software library without
 *  restriction. Specifically, if other files instantiate templates or use macros or
 *  inline functions from this file, or you compile this file and link it with  other
 *  files to produce an executable, this file does not by itself cause the resulting
 *  executable to be covered by the GNU General Public License. This exception does not
 *  however invalidate any other reasons why the executable file might be covered by the
 *  GNU General Public License.
 *
 *  THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 *  WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *  OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 *  SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 *  OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. *
 */


// Here, we'll take an ADC pin, and mirror the value on a PWM output pin.

#include "MotatePins.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

PWMOutputPin<kLED1_PinNumber> led1_pin;
ADCPin<kADCInput1_PinNumber> adc1_pin;
void ADCPin<kADCInput1_PinNumber>::interrupt() {
    led1_pin = (float)adc1_pin;
}


PWMOutputPin<kLED2_PinNumber> led2_pin;
ADCPin<kADCInput2_PinNumber> adc2_pin;
void ADCPin<kADCInput2_PinNumber>::interrupt() {
    led2_pin = (float)adc2_pin;
}



/****** Optional setup() function ******/

void setup() {
    led1_pin = 0.75;
    led1_pin = 0.25;

    adc1_pin.setInterrupts(kPinInterruptOnChange|kInterruptPriorityLow);
    adc2_pin.setInterrupts(kPinInterruptOnChange|kInterruptPriorityLow);
}

/****** Main run loop() ******/

void loop() {

    ADC_Module::startSampling();

    delay(25);
}
