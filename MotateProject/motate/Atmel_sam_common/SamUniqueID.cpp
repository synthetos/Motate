/*
 * SamID.h - motate function to retrieve the processor unique ID
 * This file is part of the Motate project, imported from the TinyG project
 *
 * Copyright (c) 2015-2016 Robert Giseburt
 * Copyright (c) 2014 Tom Cauchois
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
#include "SamUniqueID.h"

#if !defined(EFC) && defined(EFC0)
#define EFC EFC0
#endif

#if !defined(EEFC_FCR_FKEY_PASSWD)
#define EEFC_FCR_FKEY_PASSWD EEFC_FCR_FKEY(0x5A)
#endif

#define   EEFC_FCR_FCMD_STUI (0xEu << 0) /**< \brief (EEFC_FCR) Start read unique identifier */
#define   EEFC_FCR_FCMD_SPUI (0xFu << 0) /**< \brief (EEFC_FCR) Stop read unique identifier */

namespace Motate {
    // Declare our static values for storage
    uint32_t UUID_t::_d[4] = {0, 0, 0, 0};
    char UUID_t::_stringval[20] { "0000-0000-0000-0000" };

    // Define the static global Motate::UUID object;
    UUID_t UUID;

    volatile uint32_t *_UUID_REGISTER = (volatile  uint32_t *)0x00080000;

    void _readUUID()  __attribute__ ((noinline,long_call,section (".ramfunc")));
    void _readUUID() {
        // GAH! We disable ALL IRQs, since the Unique ID is actually placed
        // in the same place in RAM as the interrupt handler table!
        __disable_irq();

        // Run EEFC uuid sequence
        while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);

        EFC->EEFC_FCR = EEFC_FCR_FCMD_STUI | EEFC_FCR_FKEY_PASSWD;
        while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == 1);
        // Read unique id @ 0x00080000
        UUID._d[0] = _UUID_REGISTER[0];
        UUID._d[1] = _UUID_REGISTER[1];
        UUID._d[2] = _UUID_REGISTER[2];
        UUID._d[3] = _UUID_REGISTER[3];

        EFC->EEFC_FCR = EEFC_FCR_FCMD_SPUI | EEFC_FCR_FKEY_PASSWD;
        while ((EFC->EEFC_FSR & EEFC_FSR_FRDY) == 0);

        // Memory swap needs some time to stabilize
        for (uint32_t i=0; i<1000000; i++) {
            // force compiler to not optimize this -- NOPs don't work!
            __asm__ __volatile__("");
        }

        __enable_irq();
    }

    UUID_t::UUID_t()
    {
        _readUUID();

        // Precalculate the _stringval
        char *p =_stringval;
        for (int i = 0; i < 16; ++i) {
            // Network/Big-endian
            uint8_t byte = (_d[i/4] >> (i%4)) & 0xF;

            // Put a dash every four characters
            if (i > 0 && (i%4) == 0) {
                *p++ = '-';
            }

            // Put the HEX ASCII in the string
            if (byte < 0xA) {
                *p++ = byte + '0';
            }
            else
            {
                *p++ = (byte - 0xA) + 'a';
            }
        }
    }


    UUID_t::operator const char*()
    {
        return _stringval;
    }
}
