/*
 * SamPower.h - library for controlling the power on a Atmel sam3x8
 * This file is part of the Motate project, imported from the TinyG project
 *
 * Copyright (c) 2015 Robert Giseburt
 *
 * This file ("the software") is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2 as published by the
 * Free Software Foundation. You should have received a copy of the GNU General Public
 * License, version 2 along with the software.  If not, see <http://www.gnu.org/licenses/>.
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
 */

#include <sam.h>
#include "SamPower.h"

#if !defined(EFC) && defined(EFC0)
#define EFC EFC0
#endif

#if !defined(EEFC_FCR_FKEY_PASSWD)
#define EEFC_FCR_FKEY_PASSWD EEFC_FCR_FKEY(0x5A)
#endif

#if !defined(RSTC_CR_KEY_PASSWD)
#define RSTC_CR_KEY_PASSWD RSTC_CR_KEY(0xA5)
#endif

#ifndef EEFC_FCR_FCMD_CGPB
#define   EEFC_FCR_FCMD_CGPB (0xCu << 0) /**< \brief (EEFC_FCR) Clear GPNVM bit */
#endif

namespace Motate {

    // This is dangerous, let's add another level of namespace in case "use Motate" is in effect.
    namespace System {

        void reset(bool bootloader)
        {
            // Disable all interrupts
            __disable_irq();

            if (bootloader) {
                // Set bootflag to run SAM-BA bootloader at restart
                while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);

                //
                EFC->EEFC_FCR = EEFC_FCR_FCMD_CGPB | EEFC_FCR_FARG(1) | EEFC_FCR_FKEY_PASSWD;

                while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);

                // From here flash memory is no longer available.

                // Memory swap needs some time to stabilize
//                for (uint32_t i=0; i<1000000; i++) {
//                    // force compiler to not optimize this -- NOPs don't work!
//                    __asm__ __volatile__("");
//                }
            }

#ifdef RSTC_CR_PERRST
            RSTC->RSTC_CR = RSTC_CR_KEY_PASSWD | RSTC_CR_PROCRST | RSTC_CR_PERRST; // reset the processor and the peripherals
#else
            RSTC->RSTC_CR = RSTC_CR_KEY_PASSWD | RSTC_CR_PROCRST; // reset everything
#endif
            while (true);
        }
    }
}
