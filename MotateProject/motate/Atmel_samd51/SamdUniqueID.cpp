/*
 * SamdUniqueID.cpp - utility function for getting processor unique id
 * This file is part of the Motate project
 *
 * Copyright (c) 2026 Riley Porter
 * Copyright (c) 2026 Synthetos LLC
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
#include "SamdUniqueID.h"

namespace Motate {
    // Declare our static values for storage
    uint32_t UUID_t::_d[4] = {0, 0, 0, 0};
    char UUID_t::_stringval[40] { "0000-0000-0000-0000-0000-0000-0000-0000" };

    // Define the static global Motate::UUID object;
    UUID_t UUID;

    // SAMD51 serial number is located at these memory-mapped addresses
    // See SAMD51 datasheet section 9.6 "Serial Number"
    // The serial number is 128 bits (16 bytes) stored at:
    //   Word 0: 0x008061FC
    //   Word 1: 0x00806010
    //   Word 2: 0x00806014
    //   Word 3: 0x00806018
    static const uint32_t SERIAL_WORD0_ADDR = 0x008061FC;
    static const uint32_t SERIAL_WORD1_ADDR = 0x00806010;
    static const uint32_t SERIAL_WORD2_ADDR = 0x00806014;
    static const uint32_t SERIAL_WORD3_ADDR = 0x00806018;

    UUID_t::UUID_t()
    {
        // Read the unique ID directly from memory-mapped locations
        // SAMD51 stores the serial number in NVM at fixed addresses
        _d[0] = *((volatile uint32_t *)SERIAL_WORD0_ADDR);
        _d[1] = *((volatile uint32_t *)SERIAL_WORD1_ADDR);
        _d[2] = *((volatile uint32_t *)SERIAL_WORD2_ADDR);
        _d[3] = *((volatile uint32_t *)SERIAL_WORD3_ADDR);

        // Precalculate the _stringval
        char *p = _stringval;
        for (int i = 0; i < 32; ++i) {
            // Network/Big-endian
            uint8_t byte = (_d[i/8] >> ((7-i%8)*4)) & 0xF;

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
