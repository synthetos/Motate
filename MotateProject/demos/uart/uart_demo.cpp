/*
 * uart_demo.cpp - Motate
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
//#include "MotateSPI.h"
#include "MotateBuffer.h"

//#include <iterator>

// This makes the Motate:: prefix unnecessary.
using Motate::timer_number;

//constexpr int16_t write_buffer_size = 128;
//char write_buffer[write_buffer_size];

// Setup an led to blink and show that the board's working...
Motate::OutputPin<Motate::kLED1_PinNumber> led1_pin;
Motate::OutputPin<Motate::kLED2_PinNumber> led2_pin;

/****** Create file-global objects ******/

Motate::UART<Motate::kSerial_RX, Motate::kSerial_TX> Serial {115200}; // 115200 is the default, as well.

//Motate::SPI<10> SPI;

/****** Optional setup() function ******/

void setup() {
//    Serial.setOptions(115200, Motate::UARTMode::As8N1);
    Serial.write("Startup...done.\n", 0, /*autoFlush=*/true);
//    Serial.write("Type 0 to turn the light off, and 1 to turn it on.\n");
//
//    Serial.write("Type: ");
//
    led2_pin = 1;
}

/****** Main run loop() ******/

int16_t currentPos = 0;
//char *write_pos = std::begin(write_buffer);

void loop() {

    int16_t v = Serial.readByte();

    if (v > 0) {
//        *write_pos++ = v;
//
//        if (write_pos+1 == std::end(write_buffer)) {
//            *(write_pos-1) = '\n';
//            v = '\n';
//        }
//
        // Echo:
        Serial.writeByte(v);

        switch ((char)v) {
            case '0':
                led1_pin = 1;
                break;

            case '1':
                led1_pin = 0;
                break;

                //			default:
        }
//
//        switch ((char)v) {
//            case '\n':
//            case '\r':
//                *write_pos = 0;
//                // Write a static string...
//                Serial.write(write_buffer);
////                SPI.write(write_buffer);
//
//                Serial.write("Type: ");
//                write_pos = std::begin(write_buffer);
//                
//                break;
//        }
    }
}
