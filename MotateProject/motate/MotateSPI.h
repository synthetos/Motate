/*
  MotateSPI.hpp - SPI Library for the Motate system
  http://github.com/synthetos/motate/

  Copyright (c) 2013 Robert Giseburt

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

#ifndef MOTATESPI_H_ONCE
#define MOTATESPI_H_ONCE

#include <inttypes.h>


/* After some setup, we call the processor-specific bits, then we have the
 * any-processor parts.
 *
 * The processor specific parts MUST define:
 *
 * template<pin_number rxPinNumber, pin_number txPinNumber> using SPIGetHardware<rxPinNumber, txPinNumber> =
 *  and whatever type that returns, such as:
 * template<uint8_t uartPeripheralNumber> struct _SPIHardware
 *
 * template<pin_number rtsPinNumber, pin_number rxPinNumber> constexpr const bool isRealAndCorrectSPICLKPin()
 * template<pin_number ctsPinNumber, pin_number rxPinNumber> constexpr const bool isRealAndCorrectSPIMISOPin()
 * template<pin_number ctsPinNumber, pin_number rxPinNumber> constexpr const bool isRealAndCorrectSPIMOSIPin()
 *

 * Using the wikipedia deifinition of "normal phase," see:
 *   http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
 * Wikipedia, in turn, sites Freescale's SPI Block Guide:
 *   http://www.ee.nmt.edu/~teare/ee308l/datasheets/S12SPIV3.pdf

 * 
 *
 */

namespace Motate {
    enum SPIDeviceMode {

        // Polarity: 1 bit
        kSPIPolarityNormal     = 0<<0,
        kSPIPolarityReversed   = 1<<0,

        // Using the wikipedia deifinition of "normal phase," see:
        //   http://en.wikipedia.org/wiki/Serial_Peripheral_Interface_Bus#Clock_polarity_and_phase
        // Wikipedia, in turn, sites Freescale's SPI Block Guide:
        //   http://www.ee.nmt.edu/~teare/ee308l/datasheets/S12SPIV3.pdf

        // This makes the Phase flag INVERTED from that of the SAM3X/A datasheet.
        // Phase: 1 bit
        kSPIClockPhaseNormal   = 0<<1,
        kSPIClockPhaseReversed = 1<<1,

        // Using the wikipedia/freescale mode numbers (and the SAM3X/A datashgeet agrees).
        // The arduino mode settings appear to mirror that of wikipedia as well,
        //  so we should all be in agreement here.

        // kSPIMode0 - data are captured on the clock's rising edge (low→high transition) and data is output on a falling edge (high→low clock transition)
        kSPIMode0              = kSPIPolarityNormal   | kSPIClockPhaseNormal,
        // kSPIMode1 - data are captured on the clock's falling edge and data is output on a rising edge.
        kSPIMode1              = kSPIPolarityNormal   | kSPIClockPhaseReversed,
        // kSPIMode2 - data are captured on clock's falling edge and data is output on a rising edge.
        kSPIMode2              = kSPIPolarityReversed | kSPIClockPhaseNormal,
        // kSPIMode3 - data are captured on clock's rising edge and data is output on a falling edge.
        kSPIMode3              = kSPIPolarityReversed | kSPIClockPhaseReversed,

        // Transfer Size: 4 bits
        kSPI8Bit               = 0 << 2,
        kSPI9Bit               = 1 << 2,
        kSPI10Bit              = 2 << 2,
        kSPI11Bit              = 3 << 2,
        kSPI12Bit              = 4 << 2,
        kSPI13Bit              = 5 << 2,
        kSPI14Bit              = 6 << 2,
        kSPI15Bit              = 7 << 2,
        kSPI16Bit              = 8 << 2,
        kSPIBitsMask         = 0xf << 2
    }; // SPIDeviceMode

//    enum SPIBusMode {
//        // External CS Decoder: 1 bit
//        kSPICSDirectSelectiopn  = 0 << 0;
//        kSPICSExternalDecoder   = 1 << 0;
//    }; // SPIBusMode


    struct SPIInterrupt {
        static constexpr uint16_t Off               = 0<<0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr uint16_t Unknown           = 0<<0;

        static constexpr uint16_t OnTxReady         = 1<<0;
        static constexpr uint16_t OnTransmitReady   = 1<<0;
        static constexpr uint16_t OnTxDone          = 1<<0;
        static constexpr uint16_t OnTransmitDone    = 1<<0;

        static constexpr uint16_t OnRxReady         = 1<<1;
        static constexpr uint16_t OnReceiveReady    = 1<<1;
        static constexpr uint16_t OnRxDone          = 1<<1;
        static constexpr uint16_t OnReceiveDone     = 1<<1;

        static constexpr uint16_t OnTxTransferDone  = 1<<2;
        static constexpr uint16_t OnRxTransferDone  = 1<<3;

        /* Set priority levels here as well: */
        static constexpr uint16_t PriorityHighest   = 1<<5;
        static constexpr uint16_t PriorityHigh      = 1<<6;
        static constexpr uint16_t PriorityMedium    = 1<<7;
        static constexpr uint16_t PriorityLow       = 1<<8;
        static constexpr uint16_t PriorityLowest    = 1<<9;

        /* These are for internal use only: */
        static constexpr uint16_t OnCTSChanged      = 1<<10;
        
    };


} // namespace Motate

#ifdef __AVR_XMEGA__

#include <Atmel_avr/AvrXSPI.h>

#else

#ifdef __AVR__
#include <Atmel_avr/AvrSPI.h>
#endif

#endif

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
#include <SamSPI.h>
#endif

#if defined(__SAM4E8E__) || defined(__SAM4E16E__) || defined(__SAM4E8C__) || defined(__SAM4E16C__)
#include <SamSPI.h>
#endif


#if defined(__KL05Z__)
#include <Freescale_klxx/KL05ZSPI.h>
#endif

namespace Motate {




#pragma mark SPIBus
    /**************************************************
     *
     * SPI Bus with arbitration
     *
     **************************************************/

    template<pin_number spiMISOPinNumber, pin_number spiMOSIPinNumber, pin_number spiSCKPinNumber>
    struct SPIBus
    {

        static_assert(IsSPIMISOPin<spiMISOPinNumber>(),
                      "SPI MISO Pin is not on a hardware SPI.");

        static_assert(IsSPIMOSIPin<spiMOSIPinNumber>(),
                      "SPI MOSI Pin is not on a hardware SPI.");

        static_assert(IsSPISCKPin<spiSCKPinNumber>(),
                      "SPI SCK Pin is not on a hardware SPI.");

        static_assert((SPIMISOPin<spiMISOPinNumber>::spiNum == SPIMOSIPin<spiMOSIPinNumber>::spiNum) &&
                      (SPIMOSIPin<spiMOSIPinNumber>::spiNum == SPISCKPin<spiSCKPinNumber>::spiNum) &&
                      (SPIMISOPin<spiMISOPinNumber>::spiNum == SPISCKPin<spiSCKPinNumber>::spiNum),
                      "SPI MISO, MOSI, and SCK pins are not all on the same SPI hardware peripheral.");

        SPIMISOPin<spiMISOPinNumber> misoPin {};
        SPIMOSIPin<spiMOSIPinNumber> mosiPin {};
        SPISCKPin<spiSCKPinNumber> sckPin {};

        SPIGetHardware<spiMISOPinNumber, spiMOSIPinNumber, spiSCKPinNumber> hardware;

        std::function<void(void)> message_done_callback;


        SPIBus() : hardware{} {
            hardware.init();
        }


        // WARNING!!
        // This must be called later, outside of the contructors, to ensure that all dependencies are contructed.
        void init() {
            hardware.setInterruptHandler([&](uint16_t interruptCause) { // use a closure
                this->spiInterruptHandler(interruptCause);
            });
            hardware.setInterrupts(kInterruptPriorityHigh); // enable interrupts and set the priority
            hardware.enable();
        };

        void _TMP_setUsingCSDecoder(bool v) { hardware.setUsingCSDecoder(v); };

        void _TMP_setChannelOptions(const uint8_t channel, const uint32_t baud, const uint16_t options, uint32_t min_between_cs_delay_ns, uint32_t cs_to_sck_delay_ns, uint32_t between_word_delay_ns) {
            hardware.setChannelOptions(channel, baud, options, min_between_cs_delay_ns, cs_to_sck_delay_ns, between_word_delay_ns);
        };

        bool _TMP_startTransfer(uint8_t *tx_buffer, uint8_t *rx_buffer, uint16_t size) {
            return hardware.startTransfer(tx_buffer, rx_buffer, size);
        };

        void _TMP_setChannel(const uint8_t channel) {
            hardware.setChannel(channel);
        };

        int16_t _TMP_write(uint8_t value, int16_t &readValue, const bool lastXfer = false) {
            return hardware.write(value, readValue, lastXfer);
        };
        void spiInterruptHandler(uint16_t interruptCause) {
            if (interruptCause & SPIInterrupt::OnTxReady) {
                // ready to transfer...
            }

            if (interruptCause & SPIInterrupt::OnRxReady) {
                // something is in the buffer right now...
            }

            if (interruptCause & SPIInterrupt::OnTxTransferDone) {
//                if (transfer_tx_done_callback) {
//                    hardware._setInterruptTxTransferDone(false);
//                    transfer_tx_done_callback();
//                }
            }

            if (interruptCause & SPIInterrupt::OnRxTransferDone) {
//                if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
//                    rtsPin = true; // active low
//                }
//                if (transfer_rx_done_callback) {
//                    hardware._setInterruptRxTransferDone(false);
//                    hardware._setInterruptRxReady(true);
//                    transfer_rx_done_callback();
//                }
            }

            if (interruptCause & SPIInterrupt::OnCTSChanged) {
//                if (connection_state_changed_callback && isConnected()) {
//                    // We only report when it's connected, NOT disconnected
//                    connection_state_changed_callback(isConnected());
//                }
            }
        };

    }; // SPIBus

} // namespace Motate
#endif /* end of include guard: MOTATESPI_H_ONCE */