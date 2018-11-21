/*
 SamSPIInternal.h - Library for the Motate system
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

#ifndef SAMSPI_H_ONCE
#error This file should ONLY be included from SamSPI.h, and never included directly
#endif

// for the IDE to pickup these defines
#include "SamCommon.h" // pull in defines and fix them

namespace Motate::SPI_internal {
    template<int8_t spiPeripheralNumber>
    struct SPIInfo {
        static constexpr bool exists = false;
    }; // SPIInfo <generic>


    #if defined(SPI)
        #define CAN_SPI_PDC_DMA 1
        template<>
        struct SPIInfo<0>
        {
            static constexpr bool exists = true;

            static constexpr RegisterPtr<SEPARATE_OFF_CAST(SPI)> spi { };

            static constexpr uint32_t peripheralId = ID_SPI;
            static constexpr IRQn_Type IRQ = SPI_IRQn;
        }; // SPIInfo <0>
        #undef SPI
        #undef ID_SPI
        #undef SPI_IRQn

        // This define is because, on this processor, they chose to call it "SPI_Handler" instead
        // and saves an ifdef and duplicated code in the SamSPI.cpp
        #define SPI0_Handler SPI_Handler
    #elif defined(SPI0)
        // This is for the Sam3x and SamS70
        template<>
        struct SPIInfo<0>
        {
            static constexpr bool exists = true;

            static constexpr RegisterPtr<SEPARATE_OFF_CAST(SPI0)> spi { };

            static constexpr uint32_t peripheralId = ID_SPI0;
            static constexpr IRQn_Type IRQ = SPI0_IRQn;
        }; // SPIInfo <0>

        #undef SPI0
        #undef ID_SPI0
        #undef SPI0_IRQn
    #endif

    #if defined(SPI1)
        // This is for the Sam3x and SamS70
        template<>
        struct SPIInfo<1>
        {
            static constexpr bool exists = true;

            static constexpr RegisterPtr<SEPARATE_OFF_CAST(SPI1)> spi { };

            static constexpr uint32_t peripheralId = ID_SPI1;
            static constexpr IRQn_Type IRQ = SPI1_IRQn;
        }; // SPIInfo <0>

        #undef SPI1
        #undef ID_SPI1
        #undef SPI1_IRQn

        // for the sake of the cpp file to define the handlers
        #define HAS_SPI1
    #endif
}; // Motate::SPI_internal
