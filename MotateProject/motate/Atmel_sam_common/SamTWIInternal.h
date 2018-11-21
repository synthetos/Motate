/*
 SamTWIInternal.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2018 Robert Giseburt

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

#ifndef SAMTWI_H_ONCE
#error This file should ONLY be included from SamTWI.h, and never included directly
#endif

// for the IDE to pickup these defines
#include "SamCommon.h" // pull in defines and fix them

namespace Motate::TWI_internal {
    template<int8_t twiPeripheralNumber>
    struct TWIInfo {
        static constexpr bool exists = false;
    }; // TWIInfo <generic>


    #if defined(ID_TWIHS0)
        // This is for the Sam3x and SamS70
        template<>
        struct TWIInfo<0>
        {
            static constexpr bool exists = true;

            static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWIHS0)> twi { };

            static constexpr uint32_t peripheralId = ID_TWIHS0;
            static constexpr IRQn_Type IRQ = TWIHS0_IRQn;
        }; // TWIInfo <0>

        // #undef TWIHS0 // leave for debugging
        #undef ID_TWIHS0
        #undef TWIHS0_IRQn
    #endif

    #if defined(ID_TWIHS1)
        // This is for the Sam3x and SamS70
        template<>
        struct TWIInfo<1>
        {
            static constexpr bool exists = true;

            static constexpr RegisterPtr<SEPARATE_OFF_CAST(TWIHS1)> twi { };

            static constexpr uint32_t peripheralId = ID_TWIHS1;
            static constexpr IRQn_Type IRQ = TWIHS1_IRQn;
        }; // TWIInfo <1>

        // #undef TWIHS1 // leave for debugging
        #undef ID_TWIHS1
        #undef TWIHS1_IRQn

        // for the sake of the cpp file to define the handlers
        #define HAS_TWIHS1
    #endif
}; // Motate::SPI_internal
