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

    pin_number kSerial_RX                       =   ReversePinLookup<'C',  2>::number;
    pin_number kSerial_TX                       =   ReversePinLookup<'C',  3>::number;
    pin_number kSerial_CTS                      =   ReversePinLookup<'C',  1>::number;
    pin_number kSerial_RTS                      =   ReversePinLookup<'C',  0>::number;


    pin_number kSerial0_RX                      =   kSerial_RX;
    pin_number kSerial0_TX                      =   kSerial_TX;
    pin_number kSerial1_RX                      =   ReversePinLookup<'C',  6>::number;
    pin_number kSerial1_TX                      =   ReversePinLookup<'C',  7>::number;
    pin_number kSerial2_RX                      =   ReversePinLookup<'D',  2>::number;
    pin_number kSerial2_TX                      =   ReversePinLookup<'D',  3>::number;
    pin_number kSerial3_RX                      =   ReversePinLookup<'D',  6>::number;
    pin_number kSerial3_TX                      =   ReversePinLookup<'D',  7>::number;
    pin_number kSerial4_RX                      =   ReversePinLookup<'E',  2>::number;
    pin_number kSerial4_TX                      =   ReversePinLookup<'E',  3>::number;
    pin_number kSerial5_RX                      =   ReversePinLookup<'E',  6>::number;
    pin_number kSerial5_TX                      =   ReversePinLookup<'E',  7>::number;
    pin_number kSerial6_RX                      =   ReversePinLookup<'F',  2>::number;
    pin_number kSerial6_TX                      =   ReversePinLookup<'F',  3>::number;

//    pin_number kI2C_SDAPinNumber                =  20;
//    pin_number kI2C_SCLPinNumber                =  21;
//
//    pin_number kI2C0_SDAPinNumber               =  20;
//    pin_number kI2C0_SCLPinNumber               =  21;
//
//    pin_number kSPI_SCKPinNumber                =  76;
//    pin_number kSPI_MISOPinNumber               =  74;
//    pin_number kSPI_MOSIPinNumber               =  75;
//    pin_number kSPI_CS0PinNumber                =  77;
//    pin_number kSPI_CS1PinNumber                =  65;
//    pin_number kSPI_CS2PinNumber                =  52;
//    pin_number kSPI_CS3PinNumber                =  78;
//
//    pin_number kSPI0_SCKPinNumber               =  76;
//    pin_number kSPI0_MISOPinNumber              =  74;
//    pin_number kSPI0_MOSIPinNumber              =  75;
//    pin_number kSPI0_CS0PinNumber               =  77;
//    pin_number kSPI0_CS1PinNumber               =  68;
//    pin_number kSPI0_CS2PinNumber               =  52;
//    pin_number kSPI0_CS3PinNumber               =  78;


    pin_number kPWM0_PinNumber                  =  110;
    pin_number kPWM1_PinNumber                  =  111;
    pin_number kPWM2_PinNumber                  =  112;
    pin_number kPWM3_PinNumber                  =  113;
    pin_number kPWM4_PinNumber                  =  114;
    pin_number kPWM5_PinNumber                  =  115;
    pin_number kPWM6_PinNumber                  =  116;
    pin_number kPWM7_PinNumber                  =  117;
    pin_number kPWM8_PinNumber                  =  118;
    pin_number kPWM9_PinNumber                  =  119;
    pin_number kPWM10_PinNumber                 =  120;
    pin_number kPWM11_PinNumber                 =  121;
    pin_number kPWM12_PinNumber                 =  122;
    pin_number kPWM13_PinNumber                 =  123;
    pin_number kPWM14_PinNumber                 =  124;
    pin_number kPWM15_PinNumber                 =  125;
    pin_number kPWM16_PinNumber                 =  126;
    pin_number kPWM17_PinNumber                 =  127;
    pin_number kPWM18_PinNumber                 =  128;
    pin_number kPWM19_PinNumber                 =  129;
    pin_number kPWM20_PinNumber                 =  130;
    pin_number kPWM21_PinNumber                 =  131;


    pin_number kGPIO_XMin                       =  151;
    pin_number kInput1_PinNumber                =  151;

    pin_number kLED1_PinNumber                  =  150;
    pin_number kLED2_PinNumber                  =  127;
    pin_number kLED3_PinNumber                  =  -1;

    pin_number kLED_USBRXPinNumber              =  -1;
    pin_number kLED_USBTXPinNumber              =  -1;


    /** NOTE: When adding pin definitions here, they must be
     *        added to ALL board pin assignment files, even if
     *        they are defined as -1.
     **/

} // namespace Motate

#endif
