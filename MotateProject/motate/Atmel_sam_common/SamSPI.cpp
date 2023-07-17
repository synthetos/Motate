/*
 Atmel_sam_common/SamSPI.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2013 - 2018 Robert Giseburt

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

#include "MotateSPI.h"

namespace Motate {
    template<> std::function<void()> _SPIHardware<0>::_spiInterruptHandlerJumper {};
#if defined(HAS_SPI1)
    template<> std::function<void()> _SPIHardware<1>::_spiInterruptHandlerJumper {};
#endif // HAS_SPI1
}

extern "C" void SPI0_Handler(void)  {
    if (Motate::_SPIHardware<0u>::_spiInterruptHandlerJumper) {
        Motate::_SPIHardware<0u>::_spiInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
}

#if defined(HAS_SPI1)
extern "C" void SPI1_Handler(void)  {
    if (Motate::_SPIHardware<1u>::_spiInterruptHandlerJumper) {
        Motate::_SPIHardware<1u>::_spiInterruptHandlerJumper();
        return;
    }
#if IN_DEBUGGER == 1
    __asm__("BKPT");
#endif
}
#endif // HAS_SPI1
