/*
 utility/SamDMA.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2016 Robert Giseburt

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

// These two go outside the guard, so they can happen even though they
// (eventually) include this file.
//#include "MotateUART.h" // pull in definitions of UART enums
#include "SamCommon.h" // pull in defines and fix them
#include "MotateCommon.h"

#ifndef SAMDMA_H_ONCE
#define SAMDMA_H_ONCE

#include <functional>  // for std::function
#include <type_traits> // for std::alignment_of and std::remove_pointer

namespace Motate {

    // DMA template - MUST be specialized
    template<typename periph_t, uint8_t periph_num>
    struct DMA {
        DMA() = delete; // this prevents accidental direct instantiation
        template<typename... T>
        DMA(T...) {}; // this prevents accidental direct instantiation
        static constexpr bool exists = false;
    };

}

// So far there are two proimary types of DMA that we support:
//   - PDC (Peripheral DMA Controller), which is the DMA built into many peripherals of the SAM3X* and SAM4E* lines.
//   - DMAC (DMA Controller), which is the global DMA controller of the SAM3X*, used for peripherals without PDC.
//   - XDMAC (eXtensible DMA Controller), which is the global DMA controller of the SAMS70 (and family).

#include "SamDMAPDC.h"
#include "SamDMADMAC.h"
#include "SamDMAXDMAC.h"

#endif /* end of include guard: SAMDMA_H_ONCE */
