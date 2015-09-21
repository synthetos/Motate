/*
 * http://github.com/synthetos/motate/
 *
 * Copyright (c) 2015 Robert Giseburt
 * Copyright (c) 2015 Alden S. Hart Jr.
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

#ifndef pin_assignments_h
#define pin_assignments_h

namespace Motate {

    /** NOTE: When adding pin definitions here, they must be added to
     *        ALL board pin assignment files, even if they are defined as -1.
     **/

    pin_number kLED1_PinNumber                  =  13;
    pin_number kLED2_PinNumber                  =  35;
    pin_number kLED3_PinNumber                  =  73;

    pin_number kFET1_PinNumber                  =  35;
    pin_number kFET2_PinNumber                  =  -1;
    pin_number kFET3_PinNumber                  =  -1;

    pin_number kInput1_PinNumber                =  2;


    pin_number kADCInput1_PinNumber             =  54;
    pin_number kADCInput2_PinNumber             =  55;
    pin_number kADCInput3_PinNumber             =  56;
    pin_number kADCInput4_PinNumber             =  57;
    pin_number kADCInput5_PinNumber             =  58;
    pin_number kADCInput6_PinNumber             =  59;
    pin_number kADCInput7_PinNumber             =  60;
    pin_number kADCInput8_PinNumber             =  61;
    pin_number kADCInput9_PinNumber             =  62;
    pin_number kADCInput10_PinNumber            =  63;
    pin_number kADCInput11_PinNumber            =  64;
    pin_number kADCInput12_PinNumber            =  65;
    pin_number kADCInput13_PinNumber            =  52;
    pin_number kADCInput14_PinNumber            =  20;
    pin_number kADCInput15_PinNumber            =  21;

    //    pin_number kPWM11_PinNumber                 =  53;


    pin_number kDebug1_PinNumber                =  12;
    pin_number kDebug2_PinNumber                =  11;;

    /** NOTE: When adding pin definitions here, they must be
     *        added to ALL board pin assignment files, even if
     *        they are defined as -1.
     **/
    
} // namespace Motate

#endif
