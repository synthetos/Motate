/*
  MotateUART.h - UART Library for the Motate system
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

#include "MotatePins.h"
#include "MotateCommon.h"

#ifndef MOTATEUART_H_ONCE
#define MOTATEUART_H_ONCE

#include <cinttypes>

/* After some setup, we call the processor-specific bits, then we have the
 * any-processor parts.
 *
 * The processor specific parts MUST define:
 *
 * template<pin_number rxPinNumber, pin_number txPinNumber> using UARTGetHardware<rxPinNumber, txPinNumber> =
 *  and whatever type that returns, such as:
 * template<uint8_t uartPeripheralNumber> struct _UARTHardware
 *
 * template<pin_number rtsPinNumber, pin_number rxPinNumber> constexpr const bool isRealAndCorrectRTSPin()
 * template<pin_number ctsPinNumber, pin_number rxPinNumber> constexpr const bool isRealAndCorrectCTSPin()
 */

namespace Motate {
    struct UARTMode {
        static constexpr uint16_t NoParity           =      0; // Default
        static constexpr uint16_t EvenParity         = 1 << 0;
        static constexpr uint16_t OddParity          = 1 << 1;

        static constexpr uint16_t OneStopBit         =      0; // Default
        static constexpr uint16_t TwoStopBits        = 1 << 2;

        static constexpr uint16_t As8Bit             =      0; // Default
        static constexpr uint16_t As9Bit             = 1 << 3;
        //        static constexpr uint16_t As10Bit            = 1 << 4;

        // Some careful hand math will show that 8N1 == 0
        static constexpr uint16_t As8N1              = As8Bit | NoParity | OneStopBit;

        static constexpr uint16_t RTSCTSFlowControl  = 1 << 5;
        static constexpr uint16_t XonXoffFlowControl = 1 << 6;

        // TODO: Add polarity inversion and bit reversal options
    };

    struct UARTInterrupt : Interrupt {
        /* These are for internal use only: */
        static constexpr uint16_t OnCTSChanged      = 1<<10;
        
    };
} // namespace Motate

#ifdef __AVR_XMEGA__

#include <Atmel_XMega/XMegaUART.h>

#else

#ifdef __AVR__
#include <Atmel_avr/AvrUART.h>
#endif

#endif

#if defined(__SAM3X8E__) || defined(__SAM3X8C__)
#include <SamUART.h>
#endif

#if defined(__SAM4E8E__) || defined(__SAM4E16E__) || defined(__SAM4E8C__) || defined(__SAM4E16C__)
#include <SamUART.h>
#endif

#if defined(__SAMS70N19__) || defined(__SAMS70N20__) || defined(__SAMS70N21__)
#include <SamUART.h>
#endif

#if defined(__KL05Z__)
#include <Freescale_klxx/KL05ZUART.h>
#endif

namespace Motate {

    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1>
    struct UART {

        static_assert(UARTRxPin<rxPinNumber>::uartNum >= 0,
                      "USART RX Pin is not on a hardware USART.");

        static_assert(UARTTxPin<txPinNumber>::uartNum >= 0,
                      "USART TX Pin is not on a hardware USART.");

        static_assert(UARTRxPin<rxPinNumber>::uartNum == UARTTxPin<txPinNumber>::uartNum,
                      "USART RX Pin and TX Pin are not on the same hardware USART.");

        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;

        std::conditional_t<isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>(), UARTRTSPin<rtsPinNumber>, OutputPin<rtsPinNumber>> rtsPin;
        std::conditional_t<isRealAndCorrectCTSPin<ctsPinNumber, rxPinNumber>(), UARTCTSPin<ctsPinNumber>, IRQPin<ctsPinNumber>> ctsPin;

        UARTGetHardware<rxPinNumber, txPinNumber> hardware;

        // Use to handle pass interrupts back to the user
        std::function<void(bool)> connection_state_changed_callback;
        std::function<void(void)> transfer_rx_done_callback;
        std::function<void(void)> transfer_tx_done_callback;

        uint8_t highWaterChars;

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1, const uint8_t highWater=10) : ctsPin{kPullUp, [&]{this->uartInterruptHandler(UARTInterrupt::OnCTSChanged);}}, highWaterChars{highWater} {
            hardware.init();
            // Auto-enable RTS/CTS if the pins are provided.
            setOptions(baud, options | UARTMode::RTSCTSFlowControl, /*fromConstructor =*/ true);
        };

        // WARNING!!
        // This must be called later, outside of the contructors, to ensure that all dependencies are contructed.
        void init() {
            _stopRX();

            if (!isRealAndCorrectCTSPin<ctsPinNumber, rxPinNumber>()) {
                ctsPin.setInterrupts(Interrupt::PriorityHigh); // enable interrupts and set the priority
            }
            hardware.setInterruptHandler([&](uint16_t interruptCause) { // use a closure
                this->uartInterruptHandler(interruptCause);
            });
            hardware.setInterrupts(Interrupt::PriorityHigh); // enable interrupts and set the priority
            hardware.setInterruptRxReady(false);

        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // The USART gives us access to that pin.
            return isRealAndCorrectCTSPin<ctsPinNumber, rxPinNumber>() ? hardware.isConnected() : !ctsPin;
        };

        int16_t readByte() {
            return hardware.readByte();
        };

        void _startRX() {
            if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                rtsPin = false; // active low, so this means go
            }
        };

        void _stopRX() {
            if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                rtsPin = true; // active low, so this means stop
            }
        };

        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
        int16_t read(const uint8_t *buffer, const uint16_t length) {
            int16_t total_read = 0;
            int16_t to_read = length;
            const uint8_t *read_ptr = buffer;

            // BLOCKING!!
            while (to_read > 0) {
                int16_t ret = readByte();

                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
            };

            return total_read;
        };

        int16_t writeByte(uint8_t data) {
            hardware.flush();
            return hardware.writeByte(data);
        };

        void flush() {
            // Wait for the buffer to be empty...
            hardware.flush();
        };

        void flushRead() {
            hardware.flushRead();
        };

        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
        int16_t write(const char* data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            const char* out_ptr = data;
            int16_t to_write = length;

            if (length==0 && *out_ptr==0) {
                return 0;
            }

            do {
                int16_t ret = hardware.writeByte(*out_ptr);

                if (ret > 0) {
                    out_ptr++;
                    total_written++;
                    to_write--;

                    if (length==0 && *out_ptr==0) {
                        break;
                    }
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write);

            if (autoFlush && total_written > 0)
                flush();

            return total_written;
        };

        template<uint16_t _size>
        int16_t write(Motate::Buffer<_size> &data, const uint16_t length = 0, bool autoFlush = false) {
            int16_t total_written = 0;
            int16_t to_write = length;

            do {
                int16_t value = data.peek();
                if (value < 0) // no more data
                    break;

                int16_t ret = hardware.writeByte(value);
                if (ret > 0) {
                    data.pop();
                    to_write--;
                } else if (autoFlush) {
                    flush();
                } else {
                    break;
                }
            } while (to_write != 0);

            if (autoFlush && total_written > 0)
                flush();

            return total_written;
        };


        // **** Transfers and handling transfers

        void setConnectionCallback(std::function<void(bool)> &&callback) {
            connection_state_changed_callback = std::move(callback);
            hardware._setInterruptCTSChange((bool)connection_state_changed_callback);

            // Call it immediately if it's connected
            if (connection_state_changed_callback && isConnected()) {
                connection_state_changed_callback(true);
            }

            // pretend we're ALWAYS connected:
            //            connection_state_changed_callback(true);
        }


        // We need highWaterChars of high-water, so start the transfers such that it will split
        // at the high water mark. When the DMA for the high-water mark starts, we deassert
        // RTS (to stop the transmission) and call transfer_rx_done_callback() to hopefully
        // get a new transfer request soon.

        // This also means that this may be called with buffer that is in the region of
        // or one past the high-water mark DMA. If it's one past then there's nothing to adjust,
        // but we probably lost characters.
        // If it's in the region of the high-water-mark then we extend the high water mark DMA,
        // assert RTS (to start transmission again), making a new high-water mark region after it.

        // Cases:
        //   Notes:
        //     sections marked with (HW) are to be marked as high-water regions
        //     sections marked with (B) are to start from buffer
        //     sections marked with (B2) are to start from buffer2
        //     sections marked with (E) are to start from the end of the previous region
        //     sections marked with (X) are the previous region extended to the new end position
        //     It is assumed that if length2 > 0, length > 0.
        //     length2 may be zero, but length may not.
        //     Sections outlined below that are 0 length are simply not made.

        // 0 - length <= highWaterChars && length2 < highWaterChars
        //     Do nothing, return false.
        // 1 - length > 0 && length2 >= highWaterChars
        //     Make: length (B,X), highWaterChars (B2,HW)
        // 2 - length > highWaterChars && length2 < highWaterChars
        //     Make: length-highWaterChars (B,X), highWaterChars (E,HW)


        bool _addTransfer(char *start, uint16_t length, char *high_water_start) {
            if (!hardware.startRXTransfer(start, length)) { return false; }
            if (!hardware.startRXTransfer(high_water_start, highWaterChars)) { return false; }
            hardware.setInterruptRxTransferDone(true);
            _startRX();
            return true;
        }

        void _transactionEnded() {
            // this is called from the interupt when a transaction is done
            _stopRX();
            hardware.setInterruptRxTransferDone(false);
            if (transfer_rx_done_callback) {
                // when we enter the high-water region, we need to tell the upper layers
                transfer_rx_done_callback();
            }
        }

        volatile uint32_t _coverage_testing = 0;

        bool startRXTransfer(char *&buffer, uint16_t length, char *&buffer2, uint16_t length2) {
            // case 0 - no room
            if ((length <= highWaterChars) && (length2 < highWaterChars)) {
                _coverage_testing |= 1<<0;
                return false;
            }

            // case 1 - we can wrap around
            if (length2 >= highWaterChars) {
                _coverage_testing |= 1<<1;
                return _addTransfer(buffer, length, buffer2);
            }

            // case 2 - we can make a buffer and a high-water buffer in length
            // we know from the previous tests that:
            // length > highWaterChars && length2 < highWaterChars
            _coverage_testing |= 1<<2;
            return _addTransfer(buffer, length-highWaterChars, buffer+(length-highWaterChars));
        };

        char* getRXTransferPosition() {
            return hardware.getRXTransferPosition();
        };

        void setRXTransferDoneCallback(std::function<void()> &&callback) {
            transfer_rx_done_callback = std::move(callback);
        }


        bool startTXTransfer(char *buffer, const uint16_t length) {
            return hardware.startTXTransfer(buffer, length);
            return false;

        };

        char* getTXTransferPosition() {
            return hardware.getTXTransferPosition();
            return nullptr;
        };

        void setTXTransferDoneCallback(std::function<void()> &&callback) {
            transfer_tx_done_callback = std::move(callback);
        }

        // *** Handling interrupts

        Motate::Timeout connectionTimeout;

        void uartInterruptHandler(uint16_t interruptCause) {
            if (interruptCause & UARTInterrupt::OnTxReady) {
                // ready to transfer...
            }

            if (interruptCause & UARTInterrupt::OnRxReady) {
                // uh oh, we just lost data!
#if IN_DEBUGGER == 1
                __asm__("BKPT"); // UART buffer overflow!
#endif
            }

            if (interruptCause & UARTInterrupt::OnTxTransferDone) {
                if (transfer_tx_done_callback) {
                    hardware.setInterruptTxTransferDone(false);
                    transfer_tx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnRxTransferDone) {
                _transactionEnded();
            }
            
            if (interruptCause & UARTInterrupt::OnCTSChanged) {
                if (!isRealAndCorrectCTSPin<ctsPinNumber, rxPinNumber>()) {
                    if (isConnected()) {
                        hardware.resumeTX();
                    } else {
                        hardware.pauseTX();
                    }
                }
                if (connection_state_changed_callback && isConnected()) {
                    if (isConnected()) {
                        // if we were told to hold for more that 5 seconds, we treat it as a disconnection
                        if (connectionTimeout.isPast()) {
                            connection_state_changed_callback(false);
                        }
                        connectionTimeout.clear();
                        connection_state_changed_callback(true);
                    } else {
                        // start keeping track of when we we paused to later know if we disconnected
                        connectionTimeout.set(5000);
                    }
                }
            }
        };
        
    };

}
#endif /* end of include guard: MOTATEUART_H_ONCE */
