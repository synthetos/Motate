/*
 utility/KL05ZSPI.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2014 Robert Giseburt

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

#ifndef KL05ZSPI_H_ONCE
#define KL05ZSPI_H_ONCE

#include "MotatePins.h"
#include "MKL05Z4.h" // Redundant, but best to be explicit
#include <type_traits>

namespace Motate {

    // WHOA!! We only support master mode ... for now.


    enum SPIMode {

        kSPIPolarityNormal     = 0,
        kSPIPolarityReversed   = SPI_C1_CPOL_MASK,

        // Using the wikipedia deifinition of "normal phase," see:
        //   http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
        // Wikipedia, in turn, sites Freescale's SPI Block Guide:
        //   http://www.ee.nmt.edu/~teare/ee308l/datasheets/S12SPIV3.pdf

        kSPIClockPhaseNormal   = 0,
        kSPIClockPhaseReversed = SPI_C1_CPHA_MASK,

        // Using the wikipedia/freescale mode numbers.
        // The arduino mode settings appear to mirror that of wikipedia as well,
        //  so we should all be in agreement here.
        kSPIMode0              = kSPIPolarityNormal   | kSPIClockPhaseNormal,
        kSPIMode1              = kSPIPolarityNormal   | kSPIClockPhaseReversed,
        kSPIMode2              = kSPIPolarityReversed | kSPIClockPhaseNormal,
        kSPIMode3              = kSPIPolarityReversed | kSPIClockPhaseReversed,

        // ONLY 8-bit is supported -- TODO: Find a better way to handle this.
        kSPI8Bit               = 0
//        kSPI9Bit               = SPI_CSR_BITS_9_BIT,
//        kSPI10Bit              = SPI_CSR_BITS_10_BIT,
//        kSPI11Bit              = SPI_CSR_BITS_11_BIT,
//        kSPI12Bit              = SPI_CSR_BITS_12_BIT,
//        kSPI13Bit              = SPI_CSR_BITS_13_BIT,
//        kSPI14Bit              = SPI_CSR_BITS_14_BIT,
//        kSPI15Bit              = SPI_CSR_BITS_15_BIT,
//        kSPI16Bit              = SPI_CSR_BITS_16_BIT
    };

    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.

    template<uint8_t spiPeripheralNumber>
    struct _SPIHardware {
        static const uint8_t spiPeripheralNum=0xFF;

        // BITBANG HERE!
    };

    template<int8_t spiMISOPinNumber, int8_t spiMOSIPinNumber, int8_t spiSCKPinNumber>
    using SPIGetPeripheralNum = typename std::conditional<
        IsSPIMISOPin<spiMISOPinNumber>() &&
        IsSPIMOSIPin<spiMOSIPinNumber>() &&
        IsSPISCKPin<spiSCKPinNumber>() &&
        spiMISOPinNumber != spiMOSIPinNumber &&
        spiMISOPinNumber != spiSCKPinNumber &&
        spiMOSIPinNumber != spiSCKPinNumber &&
        SPIMISOPin<spiMISOPinNumber>::moduleId == SPIMOSIPin<spiMOSIPinNumber>::moduleId &&
        SPIMISOPin<spiMISOPinNumber>::moduleId == SPISCKPin<spiSCKPinNumber>::moduleId,
        /* True:  */ _SPIHardware<0>,
        /* False: */ _SPIHardware<0xff>
    >::type;

    template <int8_t spiMISOPinNumber, int8_t spiMOSIPinNumber, int8_t spiSCKPinNumber>
    using IsValidSPIHardware = typename std::enable_if<IsSPIMISOPin<spiMISOPinNumber>() &&
    IsSPIMOSIPin<spiMOSIPinNumber>() &&
    IsSPISCKPin<spiSCKPinNumber>() >::type;

    template<>
    struct _SPIHardware<0u> {

        // TODO: Template specialize this for KLxx with multiple SPIs
        struct spi_proxy_t {
            static constexpr volatile char &C1() { return (char &)(SPI0->C1); };
            static constexpr volatile char &C2() { return (char &)(SPI0->C2); };
            static constexpr volatile char &BR() { return (char &)(SPI0->BR); };
            static constexpr volatile char &S() { return (char &)(SPI0->S); };
            static constexpr volatile char &D() { return (char &)(SPI0->D); };
            static constexpr volatile char &M() { return (char &)(SPI0->M); };
        };

        static spi_proxy_t spi_proxy;

        static constexpr const IRQn_Type spiIRQ() { return SPI0_IRQn; };

        static constexpr const uint8_t spiPeripheralNum=0;

        void init() {
            // Enable the UART0 Clock Gate
            SIM->SCGC4 |= SIM_SCGC4_SPI0_MASK;

            disable();
        };

        _SPIHardware() {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from SPI<>::init(), so that the optimizer will keep it.
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            spi_proxy.C1() = SPI_C1_MSTR_MASK | (options & (SPI_C1_CPOL_MASK | SPI_C1_CPHA_MASK));
            //spi_proxy.C2() is okay in boot configuration, for now

            // BaudRateDivisor = (SPPR + 1) * 2^(SPR + 1)
            // SPPR is 3-bits
            // SPR is 3-bits

            uint32_t best_baud_diff = 0xFFFFFF; // Start it high, we'll change in in a second...
            uint32_t sppr_value = 0;
            uint32_t bus_clock = SystemCoreClock / 4;

            uint32_t spr_value = 7;
            for (uint32_t test_spr = 8; test_spr > 0; test_spr--) {
                // test_spr == spr+1
                uint32_t two_to_the_spr = pow(2, test_spr);

                // BaudRateDivisor = (SPPR + 1) * 2^(test_spr)
                // BaudRateDivisor = temp_sppr_value * 2^(test_spr)
                // baud = clock/(temp_sppr_value * 2^(test_spr))
                // temp_sppr_value = clock/(baud * 2^(test_spr))

                uint32_t temp_sppr_value =  bus_clock / (baud * two_to_the_spr);
                uint32_t calculated_baud = bus_clock / (temp_sppr_value * two_to_the_spr);

                uint32_t baud_diff = calculated_baud > baud ? calculated_baud - baud : baud - calculated_baud;

                if (baud_diff <= best_baud_diff) {
                    best_baud_diff = baud_diff;
                    spr_value = test_spr-1;
                    sppr_value = temp_sppr_value-1;
                }
            }

            spi_proxy.BR() = SPI_BR_SPPR(sppr_value) | SPI_BR_SPR(spr_value);
        };

        static void enable() {
            spi_proxy.C1() |= SPI_C1_SPE_MASK;
        };

        static void disable () {
            spi_proxy.C1() &= ~SPI_C1_SPE_MASK;
        };

        static int16_t read(const bool lastXfer = false, uint8_t toSendAsNoop = 0) {
            // Yay SPI! As master we must write in order to read!
            // Logic here:
            //  1) If this is not the lastXfer and we are not already sending, then
            //     transmit the Noop.
            //  2) If there is data available, read it and return it or -1.
            if (spi_proxy.S() & SPI_S_SPTEF_MASK && !lastXfer) {
                spi_proxy.D() = toSendAsNoop;
            }
            if (spi_proxy.S() & SPI_S_SPRF_MASK) {
                return spi_proxy.D();
            }

            return -1;
        }

        static int16_t write(const uint8_t value, const bool lastXfer = false) {
            int16_t throw_away;
            // Let's see what the optimizer does with this...
            return write(value, throw_away, lastXfer);
        };

        static int16_t write(const uint8_t value, int16_t &readValue, const bool lastXfer = false) {
            if (spi_proxy.S() & SPI_S_SPRF_MASK) {
                readValue = spi_proxy.D();
            } else {
                readValue = -1;
            }

            if (spi_proxy.S() & SPI_S_SPTEF_MASK) {
                spi_proxy.D() = value;

                return 1;
            }

            return -1;
        };

        int16_t transmit(uint8_t channel, const uint16_t data, const bool lastXfer = false) {
            while (!(spi_proxy.S() & SPI_S_SPTEF_MASK))
                ;
            spi_proxy.D() = data;

            while (!(spi_proxy.S() & SPI_S_SPRF_MASK))
                ;

            uint16_t outdata = spi_proxy.D();
            return outdata;
        };
    };

    pin_number _default_MISOPinNumber = ReversePinLookup<'A', 6>::number;
    pin_number _default_MOSIPinNumber = ReversePinLookup<'A', 7>::number;
    pin_number _default_SCKPinNumber = ReversePinLookup<'B', 0>::number;

    template<int8_t spiCSPinNumber, int8_t spiMISOPinNumber=_default_MISOPinNumber, int8_t spiMOSIPinNumber=_default_MOSIPinNumber, int8_t spiSCKSPinNumber=_default_SCKPinNumber>
    struct SPI {
        typedef SPIChipSelectPin<spiCSPinNumber> csPinType;
        csPinType csPin;

        SPIMISOPin<spiMISOPinNumber> misoPin;
        SPIMOSIPin<spiMOSIPinNumber> mosiPin;
        SPISCKPin<spiSCKSPinNumber> sckPin;

        static _SPIHardware< SPIGetPeripheralNum< spiMISOPinNumber, spiMOSIPinNumber, spiSCKSPinNumber>::spiPeripheralNum > hardware;
        static const uint8_t spiPeripheralNum() { return csPinType::moduleId; };
        static const uint8_t spiChannelNumber() { return SPIChipSelectPin<spiCSPinNumber>::csOffset; };


        SPI(const uint32_t baud = 4000000, const uint16_t options = kSPI8Bit | kSPIMode0) {
            hardware.init();
            init(baud, options, /*fromConstructor =*/ true);
        };

        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

        bool setChannel() {
//            return hardware.setChannel(spiChannelNumber());
            return true;
        };

//        uint16_t getOptions() {
//            // To Do
//        };

        int16_t readByte(const bool lastXfer = false, uint8_t toSendAsNoop = 0) {
            return hardware.read(lastXfer, toSendAsNoop);
        };

        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
        int16_t read(const uint8_t *buffer, const uint16_t length) {
            if (!setChannel())
                return -1;


            int16_t total_read = 0;
            int16_t to_read = length;
            const uint8_t *read_ptr = buffer;

            bool lastXfer = false;

            // BLOCKING!!
            while (to_read > 0) {

                if (to_read == 1)
                    lastXfer = true;

                int16_t ret = read(lastXfer);

                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
            };

            return total_read;
        };

        int16_t writeByte(uint16_t data, const bool lastXfer = false) {
            return hardware.write(data, lastXfer);
        };

        int16_t writeByte(uint8_t data, int16_t &readValue, const bool lastXfer = false) {
            return hardware.write(data, lastXfer);
        };

        void flush() {
//            hardware.disable();
//            hardware.enable();
        };

        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
        int16_t write(const char *data, const uint16_t length = 0, bool autoFlush = true) {
            if (!setChannel())
                return -1;

            int16_t total_written = 0;
            const char *out_buffer = data;
            int16_t to_write = length;

            bool lastXfer = false;

            // BLOCKING!!
            do {
                if (autoFlush && to_write == 1)
                    lastXfer = true;
                
                int16_t ret = writeByte(*out_buffer, lastXfer);
                
                if (ret > 0) {
                    out_buffer++;
                    total_written++;
                    to_write--;
                    if (length==0 && *out_buffer==0) {
                        break;
                    }
                }
            } while (to_write);
            
            // HACK! Autoflush forced...
            if (autoFlush && total_written > 0)
                flush();
            
            return total_written;
        }
    };
    
}

#endif /* end of include guard: KL05ZSPI_H_ONCE */