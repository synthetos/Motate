/*
 MotateDebug.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2016 Robert Giseburt

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

#ifndef MOTATEDEBUG_H_ONCE
#define MOTATEDEBUG_H_ONCE

#include "MotatePins.h" // Grab the platform-specific libraries - Pins is easy, everything has pins!
#include "MotateUtilities.h"

#ifndef IN_DEBUGGER
#define IN_DEBUGGER 0
#endif

#ifndef DEBUG_SEMIHOSTING
#define DEBUG_SEMIHOSTING 0
#endif

#define DEBUG_USE_SWI 1
#define DEBUG_USE_ITM 0

// ITM doesn't seem to work too well. Needs work.

#if DEBUG_SEMIHOSTING == 1
#warning IN_DEBUGGER=1: DEEP DEBUGGING IS ON - this firmware must be used with a debugger attached!
#endif
namespace Motate {

#if DEBUG_USE_SWI == 1
struct Debug {
    int32_t STDOUT_fh = -1; // right now, we only support one filehandle -- STDOUT

    int32_t AngelSWI_Reason_Open   = 0x01;
    int32_t AngelSWI_Reason_Close  = 0x02;
    int32_t AngelSWI_Reason_WriteC = 0x03;
    int32_t AngelSWI_Reason_Write0 = 0x04;
    int32_t AngelSWI_Reason_Write  = 0x05;
    int32_t AngelSWI_Reason_Read   = 0x06;
    int32_t AngelSWI_Reason_ReadC  = 0x07;

    Debug() {
        open();
    };

    // write to the debugger blindly
    void open()
    {
#if DEBUG_SEMIHOSTING == 1
        int32_t block[3];

        //const char * filename = ":tt"; // this ":tt" is special to the ARM to mean "console"
        const char * filename = "./MotateLog.log"; // this ":tt" is special to the ARM to mean "console"
        block[0] = (uint32_t) filename;
        block[2] = Private::c_strlen(filename);     /* length of filename */
        // block[1] = 0;     /* mode "r" -- if we wanted STDIN this is how */
        block[1] = 4;     /* mode "w" */
        STDOUT_fh = _swi(AngelSWI_Reason_Open, block);
#endif // IN_DEBUGGER == 1
    }; // open

    // write to the debugger blindly
    void write(const char* arg, int32_t length)
    {
#if DEBUG_SEMIHOSTING == 1
        if (0 > STDOUT_fh) { return; } // open apparently failed

        int32_t block[3];

        block[0] = STDOUT_fh;
        block[1] = (uint32_t)arg;
        block[2] = length;

        _swi(AngelSWI_Reason_Write, block);
#endif // DEBUG_SEMIHOSTING == 1
    }; // write

    // using template size deduction to simplify the syntax.
    // Note that the (&arg) part is vital. See:
    //   https://theotherbranch.wordpress.com/2011/08/24/template-parameter-deduction-from-array-dimensions/
    // This allows usage as debug.write("blah");
    template <uint32_t length>
    void write(const char (&arg)[length]) {
        write(arg, length);
    }

#if DEBUG_SEMIHOSTING == 1
    #ifdef __thumb__
        #define AngelSWI      0xAB
    #else
        #define AngelSWI_ARM  0x123456
        #define AngelSWI      AngelSWI_ARM
    #endif
    #ifdef __thumb2__
        #define AngelSWIInsn  "bkpt"
        // #define AngelSWIAsm   bkpt
    #else
        #define AngelSWIInsn  "swi"
        // #define AngelSWIAsm   swi
    #endif

    int32_t _swi(int32_t reason, void* arg) {
        int32_t response = 0;
        // Borrowed from newlib rdimon
        asm volatile ("mov r0, %1; mov r1, %2; " AngelSWIInsn " %a3; mov %0, r0"
        : "=r" (response) /* Outputs */
        : "r" (reason), "r" (arg), "i" (AngelSWI) /* Inputs */
        : "r0", "r1", "r2", "r3", "ip", "lr", "memory", "cc"
            /* Clobbers r0 and r1, and lr if in supervisor mode */);
            /* Accordingly to page 13-77 of ARM DUI 0040D other registers
               can also be clobbered.  Some memory positions may also be
               changed by a system call, so they should not be kept in
               registers. Note: we are assuming the manual is right and
               Angel is respecting the APCS.  */
        return response;
    }; // _swi
#endif // DEBUG_SEMIHOSTING == 1
};
#endif // DEBUG_USE_SWI == 1


#if DEBUG_USE_ITM == 1
    struct Debug {
        Debug() {
        };

        // write to the debugger blindly
        void open()
        {
        }; // open

        // write to the debugger blindly
        void write(const char* arg, int32_t length)
        {
#if DEBUG_SEMIHOSTING == 1
            int32_t delay = 100;
            while (delay--) { __asm("NOP"); };
            while (length--) {
                ITM_SendChar(*arg++);
            }
#endif // DEBUG_SEMIHOSTING == 1
        }; // write

        // using template size deduction to simplify the syntax.
        // Note that the (&arg) part is vital. See:
        //   https://theotherbranch.wordpress.com/2011/08/24/template-parameter-deduction-from-array-dimensions/
        // This allows usage as debug.write("blah");
        template <uint32_t length>
        void write(const char (&arg)[length]) {
            write(arg, length);
        }
    };
#endif // DEBUG_USE_ITM == 1

extern Debug debug;

} // namespace Motate

#endif //MOTATEDEBUG_H_ONCE
