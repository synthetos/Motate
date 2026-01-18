/*
 * SamdPower.cpp - library for controlling the power on Atmel SAMD51
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
#include "SamdPower.h"

namespace Motate {

    namespace System {

        void reset(bool bootloader)
        {
            // Disable all interrupts
            __disable_irq();

            if (bootloader) {
                // SAMD51 bootloader entry:
                // The UF2 bootloader checks a magic value in RAM at a specific location.
                // We write the "double tap" magic value to trigger bootloader mode.
                // The bootloader looks for 0xf01669ef at the end of RAM.

                // For SAMD51J19A: 192KB SRAM = 0x20000000 to 0x20030000
                // Magic location is at end of RAM - 4 bytes
                volatile uint32_t *bootloader_flag = (volatile uint32_t *)(HSRAM_ADDR + HSRAM_SIZE - 4);
                *bootloader_flag = 0xf01669ef;  // UF2 bootloader magic value
            }

            // Use NVIC System Reset Request
            // This is the standard ARM Cortex-M way to reset
            NVIC_SystemReset();

            // Should never reach here
            while (true);
        }
    }
}
