/*
 SamUART.h - Library for the Motate system
 http://github.com/synthetos/motate/

 Copyright (c) 2016 Robert Giseburt

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

#ifndef SAMUART_H_ONCE
#define SAMUART_H_ONCE

#include "MotatePins.h"
#include "MotateBuffer.h"
#include "MotateUtilities.h" // for BitField
#include <type_traits>
#include <algorithm> // for std::max, etc.
#include <functional>

#include "sam.h"

// Damn C defines. UART is a DEFINE, so we can't use it as an object type.
// We will undefine it here.
#undef UART

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

    struct UARTInterrupt {
        static constexpr uint16_t Off              = 0;
        /* Alias for "off" to make more sense
         when returned from setInterruptPending(). */
        static constexpr uint16_t Unknown           = 0;

        static constexpr uint16_t OnTxReady         = 1<<1;
        static constexpr uint16_t OnTransmitReady   = 1<<1;
        static constexpr uint16_t OnTxDone          = 1<<1;
        static constexpr uint16_t OnTransmitDone    = 1<<1;

        static constexpr uint16_t OnRxReady         = 1<<2;
        static constexpr uint16_t OnReceiveReady    = 1<<2;
        static constexpr uint16_t OnRxDone          = 1<<2;
        static constexpr uint16_t OnReceiveDone     = 1<<2;

        static constexpr uint16_t OnTxTransferDone  = 1<<3;
        static constexpr uint16_t OnRxTransferDone  = 1<<4;

        /* Set priority levels here as well: */
        static constexpr uint16_t PriorityHighest   = 1<<5;
        static constexpr uint16_t PriorityHigh      = 1<<6;
        static constexpr uint16_t PriorityMedium    = 1<<7;
        static constexpr uint16_t PriorityLow       = 1<<8;
        static constexpr uint16_t PriorityLowest    = 1<<9;

        /* These are for internal use only: */
        static constexpr uint16_t OnCTSChanged      = 1<<10;

    };

    // Convenience template classes for specialization:

    template<pin_number rxPinNumber, pin_number txPinNumber>
    using IsValidUART = typename std::enable_if<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::moduleId == UARTRxPin<rxPinNumber>::moduleId
	>::type;

    static const char kUARTXOn  = 0x11;
    static const char kUARTXOff = 0x13;

    enum class USART_MODE_t : uint32_t {
        USART_NORMAL    = 0x0 << US_MR_USART_MODE_Pos,
        RS485           = 0x1 << US_MR_USART_MODE_Pos,
        HW_HANDSHAKING  = 0x2 << US_MR_USART_MODE_Pos,
        IS07816_T_0     = 0x4 << US_MR_USART_MODE_Pos,
        IS07816_T_1     = 0x5 << US_MR_USART_MODE_Pos,
        IRDA            = 0x8 << US_MR_USART_MODE_Pos,
        LIN_MASTER      = 0xA << US_MR_USART_MODE_Pos,
        LIN_SLAVE       = 0xB << US_MR_USART_MODE_Pos,
        SPI_MASTER      = 0xE << US_MR_USART_MODE_Pos,
        SPI_SLAVE       = 0xF << US_MR_USART_MODE_Pos
    };

    enum class USCLKS_t : uint32_t {
        MCK = 0x0 << US_MR_USCLKS_Pos,
        DIV = 0x1 << US_MR_USCLKS_Pos,
        SCK = 0x3 << US_MR_USCLKS_Pos
    };

    enum class CHRL_t : uint32_t  {
        CH_5_BIT = 0x0 << US_MR_CHRL_Pos,
        CH_6_BIT = 0x1 << US_MR_CHRL_Pos,
        CH_7_BIT = 0x2 << US_MR_CHRL_Pos,
        CH_8_BIT = 0x3 << US_MR_CHRL_Pos
    };

    enum class PAR_t : uint32_t {
        EVEN        = 0x0 << US_MR_PAR_Pos,
        ODD         = 0x1 << US_MR_PAR_Pos,
        SPACE       = 0x2 << US_MR_PAR_Pos,
        MARK        = 0x3 << US_MR_PAR_Pos,
        NO          = 0x4 << US_MR_PAR_Pos,
        MULTIDROP   = 0x6 << US_MR_PAR_Pos
    };

    enum class NBSTOP_t : uint32_t {
        STOP_1_BIT      = 0x0 << US_MR_NBSTOP_Pos,
        STOP_1_5_BIT    = 0x1 << US_MR_NBSTOP_Pos,
        STOP_2_BIT      = 0x2 << US_MR_NBSTOP_Pos
    };

    enum class CHMODE_t : uint32_t {
        CHMODE_NORMAL       = 0x0 << US_MR_CHMODE_Pos,
        AUTOMATIC           = 0x1 << US_MR_CHMODE_Pos,
        LOCAL_LOOPBACK      = 0x2 << US_MR_CHMODE_Pos,
        REMOTE_LOOPBACK     = 0x3 << US_MR_CHMODE_Pos
    };


    template<uint8_t uartPeripheralNumber>
    struct _UARTHardware : SamCommon< _UARTHardware<uartPeripheralNumber> > {

        static Usart * const usart;
        static const uint32_t peripheralId();
        static const IRQn_Type uartIRQ;

        static constexpr const uint8_t uartPeripheralNum=uartPeripheralNumber;

        typedef _UARTHardware<uartPeripheralNumber> this_type_t;
        typedef SamCommon< this_type_t > common;

        static std::function<void(uint16_t)> _uartInterruptHandler;

        void init() {
            // init is called once after reset, so clean up after a reset
            common::enablePeripheralClock();

            // Reset and disable TX and RX
            usart->US_CR = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;

            // reset PCR to zero
            usart->US_RPR = 0;
            usart->US_RNPR = 0;
            usart->US_RCR = 0;
            usart->US_RNCR = 0;
            usart->US_TPR = 0;
            usart->US_TNPR = 0;
            usart->US_TCR = 0;
            usart->US_TNCR = 0;
        };

        _UARTHardware() {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };

        void enable() { usart->US_CR = US_CR_TXEN | US_CR_RXEN; };
        void disable () { usart->US_CR = US_CR_TXDIS | US_CR_RXDIS; };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            // Oversampling is either 8 or 16. Depending on the baud, we may need to select 8x in
            // order to get the error low.

            // For all of the speeds up to and including 230400, 16x multiplier worked fine in testing.
            // All yielded a <1% error in final baud.
            usart->US_BRGR = US_BRGR_CD((((SystemCoreClock * 10) / (16 * baud)) + 5)/10) | US_BRGR_FP(0);
            usart->US_MR &= ~US_MR_OVER;


            if (options & UARTMode::RTSCTSFlowControl) {
                usart->US_MR = (usart->US_MR & ~US_MR_USART_MODE_Msk) | US_MR_USART_MODE_HW_HANDSHAKING;
            } else {
                usart->US_MR = (usart->US_MR & ~US_MR_USART_MODE_Msk) | US_MR_USART_MODE_NORMAL;
            }

            if (options & UARTMode::TwoStopBits) {
                usart->US_MR = (usart->US_MR & ~(US_MR_NBSTOP_Msk)) | US_MR_NBSTOP_2_BIT;
            } else {
                usart->US_MR = (usart->US_MR & ~(US_MR_NBSTOP_Msk)) | US_MR_NBSTOP_1_BIT;
            }

            if (options & UARTMode::As9Bit) {
                usart->US_MR |= US_MR_MODE9;
            } else {
                usart->US_MR = (usart->US_MR & ~(US_MR_MODE9|US_MR_CHRL_Msk)) | static_cast<uint32_t>(CHRL_t::CH_8_BIT);
            }
            if (options & UARTMode::EvenParity) {
                usart->US_MR = (usart->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_EVEN;
            } else if (options & UARTMode::OddParity) {
                usart->US_MR = (usart->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_ODD;
            } else {
                usart->US_MR = (usart->US_MR & ~(US_MR_PAR_Msk)) | US_MR_PAR_NO;
            }

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & UARTInterrupt::OnRxDone) {
                    usart->US_IER = US_IER_RXRDY;
                } else {
                    usart->US_IDR = US_IDR_RXRDY;
                }
                if (interrupts & UARTInterrupt::OnTxDone) {
                    usart->US_IER = US_IER_TXRDY;
                } else {
                    usart->US_IDR = US_IDR_TXRDY;
                }

                if (interrupts & UARTInterrupt::OnRxTransferDone) {
                    usart->US_IER = US_IER_RXBUFF;
                } else {
                    usart->US_IDR = US_IDR_RXBUFF;
                }
                if (interrupts & UARTInterrupt::OnTxTransferDone) {
                    usart->US_IER = US_IER_TXBUFE;
                } else {
                    usart->US_IDR = US_IDR_TXBUFE;
                }


                /* Set interrupt priority */
                if (interrupts & UARTInterrupt::PriorityHighest) {
                    NVIC_SetPriority(uartIRQ, 0);
                }
                else if (interrupts & UARTInterrupt::PriorityHigh) {
                    NVIC_SetPriority(uartIRQ, 3);
                }
                else if (interrupts & UARTInterrupt::PriorityMedium) {
                    NVIC_SetPriority(uartIRQ, 7);
                }
                else if (interrupts & UARTInterrupt::PriorityLow) {
                    NVIC_SetPriority(uartIRQ, 11);
                }
                else if (interrupts & kInterruptPriorityLowest) {
                    NVIC_SetPriority(uartIRQ, 15);
                }

                NVIC_EnableIRQ(uartIRQ);
            } else {

                NVIC_DisableIRQ(uartIRQ);
            }
        };

        void setInterruptHandler(std::function<void(uint16_t)> &&handler) {
            _uartInterruptHandler = std::move(handler);
        }

        void _setInterruptTxReady(bool value) {
            if (value) {
                usart->US_IER = US_IER_TXRDY;
            } else {
                usart->US_IDR = US_IDR_TXRDY;
            }
        };

        void _setInterruptRxReady(bool value) {
            if (value) {
                usart->US_IER = US_IER_RXRDY;
            } else {
                usart->US_IDR = US_IDR_RXRDY;
            }
        };

        void _setInterruptCTSChange(bool value) {
            if (value) {
                usart->US_IER = US_IER_CTSIC;
            } else {
                usart->US_IDR = US_IDR_CTSIC;
            }
        };

        void _setInterruptTxTransferDone(bool value) {
            if (value) {
                usart->US_IER = US_IER_TXBUFE;
            } else {
                usart->US_IDR = US_IDR_TXBUFE;
            }
        };

        void _setInterruptRxTransferDone(bool value) {
            if (value) {
                usart->US_IER = US_IER_RXBUFF;
            } else {
                usart->US_IDR = US_IDR_RXBUFF;
            }
        };

        static uint16_t getInterruptCause() { // __attribute__ (( noinline ))
            uint16_t status = UARTInterrupt::Unknown;
            auto US_CSR_hold = usart->US_CSR;
            if (US_CSR_hold & US_CSR_TXRDY) {
                status |= UARTInterrupt::OnTxReady;
            }
            if (US_CSR_hold & US_IER_TXBUFE) {
                status |= UARTInterrupt::OnTxTransferDone;
            }
            if (US_CSR_hold & US_CSR_RXRDY) {
                status |= UARTInterrupt::OnRxReady;
            }
            if (US_CSR_hold & US_IER_RXBUFF) {
                status |= UARTInterrupt::OnRxTransferDone;
            }
            if (US_CSR_hold & US_CSR_CTSIC) {
                status |= UARTInterrupt::OnCTSChanged;
            }
            return status;
        }

        int16_t readByte() {
            if (usart->US_CSR & US_CSR_RXRDY) {
                return (usart->US_RHR & US_RHR_RXCHR_Msk);
            }

            return -1;
        };

        int16_t writeByte(const char value) {
            if (usart->US_CSR & US_CSR_TXRDY) {
                usart->US_THR = US_THR_TXCHR(value);
            }
            return -1;
        };

        void flush() {
            // Wait for the buffer to be empty
            while (!usart->US_CSR & US_CSR_TXEMPTY) {
                ;
            }
        };

        void flushRead() {
            // kill any incoming transfers
            usart->US_RNCR = 0;
            usart->US_RCR = 0;
        };


        // ***** Connection status check (simple)
        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // The USART gives us access to that pin.
            return (usart->US_CSR & US_CSR_CTS) == 0; // active LOW
        };


        // ***** Handle Tranfers
        bool startRXTransfer(char *buffer, const uint16_t length) {
            if (usart->US_RCR == 0) {
                usart->US_RPR = (uint32_t)buffer;
                usart->US_RCR = length;
                usart->US_PTCR = US_PTCR_RXTEN;
                _setInterruptRxTransferDone(true);
                return true;
            }
            else if (usart->US_RNCR == 0) {
                usart->US_RNPR = (uint32_t)buffer;
                usart->US_RNCR = length;
                _setInterruptRxTransferDone(true);
                return true;
            }
            return false;
        };

        char* getRXTransferPosition() {
            return (char*)usart->US_RPR;
        };

        bool startTXTransfer(char *buffer, const uint16_t length) {
            if (usart->US_TCR == 0) {
                usart->US_TPR = (uint32_t)buffer;
                usart->US_TCR = length;
                usart->US_PTCR = US_PTCR_TXTEN;
                _setInterruptTxTransferDone(true);
                return true;
            }
            else if (usart->US_TNCR == 0) {
                usart->US_TNPR = (uint32_t)buffer;
                usart->US_TNCR = length;
                _setInterruptTxTransferDone(true);
                return true;
            }
            return false;
        };

        char* getTXTransferPosition() {
            return (char*)usart->US_TPR;
        };

    };


    template<pin_number rxPinNumber, pin_number txPinNumber>
    using UARTGetHardware = typename std::conditional<
        IsUARTRxPin<rxPinNumber>() &&
        IsUARTTxPin<txPinNumber>() &&
        rxPinNumber != txPinNumber &&
        UARTTxPin<txPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum,
        /* True:  */ _UARTHardware<UARTTxPin<txPinNumber>::uartNum>,
        /* False: */ _UARTHardware<0xff> // static_assert below should prevent this
    >::type;


    template<uint8_t uartPeripheralNumber>
    struct _UARTHardwareProxy {
    };

    // Declare that these are specilized
    template<> const uint32_t  _UARTHardware<0>::peripheralId();
    template<> std::function<void(uint16_t)> _UARTHardware<0>::_uartInterruptHandler;

    template<> const uint32_t  _UARTHardware<1>::peripheralId();
    template<> std::function<void(uint16_t)> _UARTHardware<1>::_uartInterruptHandler;

    template<> const uint32_t  _UARTHardware<2>::peripheralId();
    template<> std::function<void(uint16_t)> _UARTHardware<2>::_uartInterruptHandler;

    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1>
    struct UART {

        static_assert(UARTRxPin<rxPinNumber>::uartNum >= 0,
                      "USART RX Pin is not on a hardware USART.");

        static_assert(UARTTxPin<txPinNumber>::uartNum >= 0,
                      "USART TX Pin is not on a hardware USART.");

        static_assert(UARTRxPin<rxPinNumber>::uartNum == UARTTxPin<txPinNumber>::uartNum,
                      "USART RX Pin and TX Pin are not on the same hardware USART.");

        static_assert((UARTRTSPin<rtsPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum), // (rtsPinNumber == -1) || 
                      "USART RX Pin and RTS Pin are not on the same hardware USART.");

        static_assert((UARTCTSPin<ctsPinNumber>::uartNum == UARTRxPin<rxPinNumber>::uartNum), // (ctsPinNumber == -1) ||
                      "USART RX Pin and CTS Pin are not on the same hardware USART.");

        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;
        UARTRTSPin<rtsPinNumber> rtsPin;
        UARTCTSPin<ctsPinNumber> ctsPin;

        UARTGetHardware<rxPinNumber, txPinNumber> hardware;

        // Use to handle pass interrupts back to the user
        std::function<void(bool)> connection_state_changed_callback;
        std::function<void(void)> transfer_rx_done_callback;
        std::function<void(void)> transfer_tx_done_callback;

        Buffer<16> overflowBuffer;

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
            hardware.init();
            // Auto-enable RTS/CTS if the pins are provided.
            setOptions(baud, options | (rtsPin.is_real ? UARTMode::RTSCTSFlowControl : 0), /*fromConstructor =*/ true);
        };

        // WARNING!!
        // This must be called later, outside of the contructors, to ensure that all dependencies are contructed.
        void init() {
            hardware.setInterruptHandler([&](uint16_t interruptCause) { // use a closure
                this->uartInterruptHandler(interruptCause);
            });
            hardware.setInterrupts(kInterruptPriorityLowest); // enable interrupts and set the priority
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // The USART gives us access to that pin.
            return hardware.isConnected();
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
            //hardware._setInterruptCTSChange((bool)connection_state_changed_callback);

            // Call it immediately if it's connected
//            if(connection_state_changed_callback && hardware.isConnected()) {
//                connection_state_changed_callback(true);
//            }

            // pretend we're ALWAYS connected:
            connection_state_changed_callback(true);
        }


        bool startRXTransfer(char *buffer, uint16_t length) {
            hardware._setInterruptRxReady(false);

            int16_t overflow;
            while (((overflow = overflowBuffer.read()) > 0) && (length > 0)) {
                *buffer = (char)overflow;
                buffer++;
                length--;
            }

            // what happens if length == 0?

            return hardware.startRXTransfer(buffer, length);
            return false;
        };

        char* getRXTransferPosition() {
            return hardware.getRXTransferPosition();
            return nullptr;
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
                    hardware._setInterruptTxTransferDone(false);
                    transfer_tx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnRxTransferDone) {
                if (transfer_rx_done_callback) {
                    hardware._setInterruptRxTransferDone(false);
                    hardware._setInterruptRxReady(true);
                    transfer_rx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnCTSChanged) {
                if (connection_state_changed_callback) {
                    // We need to throttle this for MCU<->MCU connections.
                    //connection_state_changed_callback(hardware.isConnected());
                }
            }
        };

    };


    template<uint8_t uartPeripheralNumber, pin_number rtsPinNumber, pin_number ctsPinNumber, typename rxBufferClass, typename txBufferClass>
    struct _BufferedUARTHardware : _UARTHardware<uartPeripheralNumber> {
        rxBufferClass rxBuffer;
        txBufferClass txBuffer;

        OutputPin<rtsPinNumber> rtsPin;
        IRQPin<ctsPinNumber> ctsPin;

        uint32_t txDelayAfterResume = 3;
        uint32_t txDelayUntilTime   = 0;

        bool _rtsCtsFlowControl    = false;
        bool _xonXoffFlowControl   = false;
        volatile bool _xonXoffCanSend       = true;
        volatile char _xonXoffStartStop     = kUARTXOn;
        volatile bool _xonXoffStartStopSent = true;

        typedef _UARTHardware<uartPeripheralNumber> parent;

        _BufferedUARTHardware() {
        };

        void init() {
            parent::init();
            parent::setInterrupts(UARTInterrupt::OnRxReady | UARTInterrupt::PriorityLowest);
            _UARTHardwareProxy<uartPeripheralNumber>::uartInterruptHandler = [&](uint16_t interruptCause) { // use a closure
                uartInterruptHandler(interruptCause);
            };
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            parent::setOptions(baud, options, fromConstructor);

            if (options & UARTMode::RTSCTSFlowControl && IsIRQPin<ctsPinNumber>() && !rtsPin.isNull()) {
                _rtsCtsFlowControl = true;
//                parent::setInterruptTxReady(!canSend());
                ctsPin.setInterrupts(kPinInterruptOnChange);
            } else {
                _rtsCtsFlowControl = false;
            }
//            if (options & UARTMode::XonXoffFlowControl) {
//                _xonXoffFlowControl = true;
//            } else {
//                _xonXoffFlowControl = false;
//                _xonXoffStartStopSent = true;
//            }
        }

//        void stopRx() {
//            if (_rtsCtsFlowControl) {
//                rtsPin = true;
//            }
//            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOff) {
//                _xonXoffStartStop = kUARTXOff;
//                _xonXoffStartStopSent = false;
//                parent::setInterruptTxReady(true);
//            }
//        };
//
//        void startRx() {
//            if (_rtsCtsFlowControl) {
//                rtsPin = false;
//            }
//            if (_xonXoffFlowControl && _xonXoffStartStop != kUARTXOn) {
//                _xonXoffStartStop = kUARTXOn;
//                _xonXoffStartStopSent = false;
//                parent::setInterruptTxReady(true);
//            }
//        };

//        bool canSend() {
//            if (_rtsCtsFlowControl) {
//                return !ctsPin;
//            }
//            if (_xonXoffFlowControl) {
//                return _xonXoffCanSend;
//            }
//            return true;
//        };

        void setTxDelayAfterResume(uint32_t newDelay) { txDelayAfterResume = newDelay; };

        int16_t readByte() {
            return rxBuffer.read();
        };

        int16_t writeByte(const uint8_t data) {
            int16_t ret = txBuffer.write(data);
            return ret;
        };

        void uartInterruptHandler(uint16_t interruptCause) {
//            if ((interruptCause & (UARTInterrupt::OnTxReady /*| UARTInterrupt::OnTxDone*/))) {
//                if (txDelayUntilTime && SysTickTimer.getValue() < txDelayUntilTime)
//                    return;
//                txDelayUntilTime = 0;
//                if (_xonXoffFlowControl) {
//                    if (_xonXoffStartStopSent == false) {
//                        parent::writeByte(_xonXoffStartStop);
//                        _xonXoffStartStopSent = true;
//                        return;
//                    }
//                }
//                int16_t value = txBuffer.read();
//                if (value >= 0) {
//                    parent::writeByte(value);
//                }
//            }
//            if (txBuffer.isEmpty() || txBuffer.isLocked()) {
//                // This is tricky: If it's write locked, we have to bail, and SHUT OFF TxReady interrupts.
//                // On the ARM, it won't return to the main code as long as there's a pending interrupt,
//                // and the txReady interrupt will continue to fire, causing deadlock.
//                parent::setInterruptTxReady(false);
//            }
//
//            if ((interruptCause & UARTInterrupt::OnRxReady) && !rxBuffer.isFull()) {
//                int16_t value = parent::readByte();
//                if (_xonXoffFlowControl) {
//                    if (value == kUARTXOn) {
//                        _xonXoffCanSend = true;
//                        return;
//                    } else if (value == kUARTXOff) {
//                        _xonXoffCanSend = false;
//                        return;
//                    }
//                }
//                // We don't double check to ensure value is not -1 -- should we?
//                rxBuffer.write(value);
//                if (rxBuffer.available() < 4) {
//                    stopRx();
//                }
//            }
        };

//        void pinChangeInterrupt() {
//            txDelayUntilTime = SysTickTimer.getValue() + txDelayAfterResume;
//            parent::setInterruptTxReady(canSend());
//        };

        void flush() {
            // Wait for the buffer to be empty...
            while(!txBuffer.isEmpty());
        };
    };



    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1, typename rxBufferClass = Buffer<128>, typename txBufferClass = rxBufferClass>
    struct BufferedUART {
        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;


        _BufferedUARTHardware< UARTGetHardware<rxPinNumber, txPinNumber>::uartPeripheralNum, rtsPinNumber, ctsPinNumber, rxBufferClass, txBufferClass > hardware;


        const uint8_t uartPeripheralNum() { return hardware.uartPeripheralNum; };

        BufferedUART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
            hardware.init();
            init(baud, options, /*fromConstructor =*/ true);
        };

        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            setOptions(baud, options, fromConstructor);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
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
                int16_t ret = hardware.readByte();

                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
            };

            return total_read;
        };

        int16_t writeByte(uint8_t data) {
            return hardware.writeByte(data);
        };

        void flush() {
            hardware.flush();
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

        void pinChangeInterrupt() {
            hardware.pinChangeInterrupt();
        };

        //	// Placeholder for user code.
        //	static void interrupt() __attribute__ ((weak));
    };
}

#endif /* end of include guard: SAMUART_H_ONCE */
