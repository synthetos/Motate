/*
 * blink_demo.cpp - Motate
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
#include "MotateUART.h"
#include "MotateBuffer.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

char write_buffer[8] = "static\n";

//Buffer<8> write_buffer = { "static\n" };

// Setup an led to blink and show that the board's working...
OutputPin<kLED1_PinNumber> led1_pin;

/****** Create file-global objects ******/

UART<> serialPort {9600};

/****** Optional setup() function ******/

void setup() {
//	serialPort.write("Startup...\n", 11);
}

/****** Main run loop() ******/

std::size_t currentPos = 0;
int16_t v;

void loop() {
	// Blink the led...

	// Write a byte, then "flush()" to make sure it's completely sent.
	/*
	 serialPort.write('T'); 	serialPort.flush();
	serialPort.write('e'); 	serialPort.flush();
	serialPort.write('s'); 	serialPort.flush();
	serialPort.write('t'); 	serialPort.flush();
	serialPort.write('\n'); serialPort.flush();

	// Write a static string...
	serialPort.write(write_buffer);

//	for (char X: write_buffer) {
//		if (!X)
//			break;
//		serialPort.write(X); 	serialPort.flush();
//	}
*/

	v = serialPort.read();
//	if (value > 0) {
		led1_pin.toggle();

		serialPort.write("--> "); serialPort.flush();
		serialPort.write((uint8_t)v); serialPort.flush();
		serialPort.write("\n");

		delay(250);

		//write_buffer[currentPos] = value;
		if (++currentPos > 7)
			currentPos = 0;
//	}

	//delay(250);
}
