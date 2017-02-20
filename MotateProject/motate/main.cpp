/*
 * main.cpp - Motate
 * This file is part of the Motate project.
 *
 * Copyright (c) 2013 - 2014 Alden S. Hart, Jr.
 * Copyright (c) 2013 - 2014 Robert Giseburt
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
using Motate::delay;

/******************** External interface setup ************************/

#include "MotateDebug.h"
#if DEBUG_SEMIHOSTING == 1
namespace Motate {
    Debug debug;
}
#endif

/******************** Initialization setup ************************/

void setup() __attribute__ ((weak));

extern void loop();

int main(void);

#ifdef __ARM__

// This is used by the anything that may generate a destructor:
void* __dso_handle = nullptr;

#ifdef __cplusplus
extern "C"{
#endif // __cplusplus

    // These routines are defined with C linkage:

    void _init() {
        SystemInit();
    }

    void __libc_init_array(void);

    void _start() {
        SystemInit();
        __libc_init_array();
        main();
    }

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __ARM__

/*
 * _system_init()
 */

void _system_init(void)
{
#ifdef __AVR__
    SystemInit();
#endif // __AVR__

    Motate::WatchDogTimer.disable();
}


/*
 * main()
 */

int main(void) {
    _system_init();

    if (setup)
        setup();

    // main loop
    for (;;) {
        loop();
    }
    return 0;
}
