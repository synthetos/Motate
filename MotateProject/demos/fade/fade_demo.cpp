/*
 * fade_demo.cpp - Motate
 * This file is part of the Motate project.
 *
 * Copyright (c) 2014 Robert Giseburt
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

#include "MotatePins.h"
#include "MotateTimers.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

PWMOutputPin<kLED1_PinNumber> led1_pin(kPWMPinInverted);
PWMOutputPin<kLED2_PinNumber> led2_pin(kPWMPinInverted);
PWMOutputPin<kLED3_PinNumber> led3_pin(kPWMPinInverted);

static const float change_per_cycle = 0.05;

float red_pwm     = 1.0;
float red_pwm_d   = 0.0;

float green_pwm   = 0.0;
float green_pwm_d = change_per_cycle;

float blue_pwm    = 0.0;
float blue_pwm_d  = 0.0;

/****** Optional setup() function ******/

void setup() {

    led1_pin.setFrequency(1000);

    led1_pin = blue_pwm;
    led2_pin = green_pwm;
    led3_pin = red_pwm;
}

/****** Main run loop() ******/

void loop() {
    led3_pin = red_pwm;
    led2_pin = green_pwm;
    led1_pin = blue_pwm;

    red_pwm += red_pwm_d;
    green_pwm += green_pwm_d;
    blue_pwm += blue_pwm_d;

    // This is the dnace for color cycles through all hues.
    // see http://en.wikipedia.org/wiki/HSV_color_space#mediaviewer/File:HSV-RGB-comparison.svg
    // via http://en.wikipedia.org/wiki/HSV_color_space#Converting_to_RGB
    if (red_pwm > 1.0) {
        red_pwm = 1.0;
        red_pwm_d = 0.0;
        green_pwm_d = -change_per_cycle;
    }
    else if (red_pwm < 0.0) {
        red_pwm = 0.0;
        red_pwm_d = 0.0;
        green_pwm_d = change_per_cycle;
    }
    else if (blue_pwm > 1.0) {
        blue_pwm = 1.0;
        blue_pwm_d = 0.0;
        red_pwm_d = -change_per_cycle;
    }
    else if (blue_pwm < 0.0) {
        blue_pwm = 0.0;
        blue_pwm_d = 0.0;
        red_pwm_d = change_per_cycle;
    }
    else if (green_pwm > 1.0) {
        green_pwm = 1.0;
        green_pwm_d = 0.0;
        blue_pwm_d = -change_per_cycle;
    }
    else if (green_pwm < 0.0) {
        green_pwm = 0.0;
        green_pwm_d = 0.0;
        blue_pwm_d = change_per_cycle;
    }
    
    delay(100);
    
    //	led1_pin.toggle();
}
