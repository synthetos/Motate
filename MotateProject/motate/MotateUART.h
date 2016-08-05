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

#ifndef MOTATEUART_H_ONCE
#define MOTATEUART_H_ONCE

#include <inttypes.h>

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

        Buffer<16> overflowBuffer;

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) : ctsPin{kPullUp, [&]{this->uartInterruptHandler(UARTInterrupt::OnCTSChanged);}} {
            hardware.init();
            // Auto-enable RTS/CTS if the pins are provided.
            setOptions(baud, options | UARTMode::RTSCTSFlowControl, /*fromConstructor =*/ true);
        };

        // WARNING!!
        // This must be called later, outside of the contructors, to ensure that all dependencies are contructed.
        void init() {
            hardware.setInterruptHandler([&](uint16_t interruptCause) { // use a closure
                this->uartInterruptHandler(interruptCause);
            });
            hardware.setInterrupts(kInterruptPriorityHigh); // enable interrupts and set the priority
            if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                rtsPin = true; // active low
            }
            if (!isRealAndCorrectCTSPin<ctsPinNumber, rxPinNumber>()) {
                ctsPin.setInterrupts(kInterruptPriorityHigh); // enable interrupts and set the priority
            }
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

        char* _manual_rx_position = nullptr;
        bool startRXTransfer(char *&buffer, uint16_t length) {
            hardware.setInterruptRxReady(false);

            int16_t overflow;
            while (((overflow = overflowBuffer.read()) > 0) && (length > 0)) {
                *buffer = (char)overflow;
                buffer++;
                length--;
            }

            if (length == 0) {
                _manual_rx_position = buffer;

            } else {
                _manual_rx_position = nullptr;
                if (hardware.startRXTransfer(buffer, length)) {
                    if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                        rtsPin = false; // active low
                    }
                    return true;
                }
            }

            if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                rtsPin = true; // active low
            }

            hardware.setInterruptRxReady(true);
            return false;
        };

        char* getRXTransferPosition() {
            if (_manual_rx_position) {
                return _manual_rx_position;
            }
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

        void uartInterruptHandler(uint16_t interruptCause) {
            if (interruptCause & UARTInterrupt::OnTxReady) {
                // ready to transfer...
            }

            if (interruptCause & UARTInterrupt::OnRxReady) {
                // new data is ready to read. If we're between transfers we need to squirrel away the value
                overflowBuffer.write(hardware.readByte());
            }

            if (interruptCause & UARTInterrupt::OnTxTransferDone) {
                if (transfer_tx_done_callback) {
                    hardware.setInterruptTxTransferDone(false);
                    transfer_tx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnRxTransferDone) {
                if (!isRealAndCorrectRTSPin<rtsPinNumber, rxPinNumber>()) {
                    rtsPin = true; // active low
                }
                if (transfer_rx_done_callback) {
                    hardware.setInterruptRxTransferDone(false);
                    hardware.setInterruptRxReady(true);
                    transfer_rx_done_callback();
                }
            }
            
            if (interruptCause & UARTInterrupt::OnCTSChanged) {
                if (connection_state_changed_callback && isConnected()) {
                    // We only report when it's connected, NOT disconnected
                    connection_state_changed_callback(isConnected());
                }
            }
        };
        
    };

}
#endif /* end of include guard: MOTATEUART_H_ONCE */