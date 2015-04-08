/*
 * buffered_uart_demo.cpp - Motate
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
#include "MotateSerial.h"
#include "MotateBuffer.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

// Setup an led to blink and show that the board's working...
OutputPin<kLED1_PinNumber> led1_pin;
OutputPin<kLED2_PinNumber> led2_pin;
IRQPin<kInput1_PinNumber> button { Motate::kPullUp };

// Create a buffer to hold the data to blast
Motate::Buffer<1024> blast_buffer;

/****** Create file-global objects ******/

namespace Motate {
MOTATE_PIN_INTERRUPT(kInput1_PinNumber) {
    led2_pin = button;
}
}


/****** Optional setup() function ******/

void setup() {
    Serial.write("Startup...done.\n");
    Serial.write("Type 0 to turn the light off, and 1 to turn it on.\n");

    Serial.write("Type: ");


    led2_pin = 1;

    button.setInterrupts(kPinInterruptOnChange | kInterruptPriorityLowest);
}

/****** Main run loop() ******/


void loop() {

    int16_t v = Serial.readByte();

    if (v > 0) {
	// Echo:
	blast_buffer.write(v);

	switch ((char)v) {
	    case '0':
		led1_pin = 1;
		break;

	    case '1':
		led1_pin = 0;
		break;

	    case '\n':
	    case '\r':
		Serial.write(blast_buffer);
		break;
	}
        delay(1);
    }
}
