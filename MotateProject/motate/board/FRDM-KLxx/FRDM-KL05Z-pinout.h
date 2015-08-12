/*
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2014 Robert Giseburt
 * Copyright (c) 2014 Alden S. Hart Jr.
 *
 * This file is part of the Motate Library.
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software. If not, see <http://www.gnu.org/licenses/>.
 *
 * As a special exception, you may use this file as part of a software library without
 * restriction. Specifically, if other files instantiate templates or use macros or
 * inline functions from this file, or you compile this file and link it with  other
 * files to produce an executable, this file does not by itself cause the resulting
 * executable to be covered by the GNU General Public License. This exception does not
 * however invalidate any other reasons why the executable file might be covered by the
 * GNU General Public License.
 *
 * THE SOFTWARE IS DISTRIBUTED IN THE HOPE THAT IT WILL BE USEFUL, BUT WITHOUT ANY
 * WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
 * SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef gShield_pinout_h
#define gShield_pinout_h

#include <MotatePins.h>

namespace Motate {

    // NOT ALL OF THESE PINS ARE ON ALL PLATFORMS
    // Undefined pins will be equivalent to Motate::NullPin, and return 1 for Pin<>::isNull();

    pin_number kSerial_RX                       =   0;
    pin_number kSerial_TX                       =   1;

    pin_number kSerial0_RX                      =   0;
    pin_number kSerial0_TX                      =   1;

    pin_number kI2C_SDAPinNumber                =  14;
    pin_number kI2C_SCLPinNumber                =  15;

    pin_number kI2C0_SDAPinNumber               =  14;
    pin_number kI2C0_SCLPinNumber               =  15;

    pin_number kSPI_SCKPinNumber                =  13;
    pin_number kSPI_MISOPinNumber               =  12;
    pin_number kSPI_MOSIPinNumber               =  11;
    pin_number kSPI_CS0PinNumber                =  10;
    pin_number kSPI_CS1PinNumber                =  -1;
    pin_number kSPI_CS2PinNumber                =  -1;
    pin_number kSPI_CS3PinNumber                =  -1;

    pin_number kSPI0_SCKPinNumber               =  13;
    pin_number kSPI0_MISOPinNumber              =  12;
    pin_number kSPI0_MOSIPinNumber              =  11;
    pin_number kSPI0_CS0PinNumber                =  10;
    pin_number kSPI0_CS1PinNumber               =  -1;
    pin_number kSPI0_CS2PinNumber               =  -1;
    pin_number kSPI0_CS3PinNumber               =  -1;

    pin_number kLED1_PinNumber                  =  16;
    pin_number kLED2_PinNumber                  =  17;
    pin_number kLED3_PinNumber                  =   8;


    pin_number kLED_USBRXPinNumber              =  -1;
    pin_number kLED_USBTXPinNumber              =  -1;

    pin_number kSerial_RTS                      =  2;
    pin_number kSerial_CTS                      =  3;

    pin_number kInput1_PinNumber                =  4;

    /** NOTE: When adding pin definitions here, they must be
     *        added to ALL board pin assignment files, even if
     *        they are defined as -1.
     **/

} // namespace Motate

#endif
