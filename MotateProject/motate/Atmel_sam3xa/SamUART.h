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

    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.

    using Motate::Private::BitField;

    // This is a mirror of the Atmel Usart struct, with bitfields for simpler access
    typedef struct {

        struct US_CR_t {                     /**< \brief (Usart Offset: 0x0000) Control Register */
            __O  uint32_t raw;

            BitField<bool,  2, 1, decltype(raw)> RSTRX;      // bit  2 - Reset Receiver
            BitField<bool,  3, 1, decltype(raw)> RSTTX;      // bit  3 - Reset Transmitter

            BitField<bool,  4, 1, decltype(raw)> RXEN;       // bit  4 - Receiver Enable
            BitField<bool,  5, 1, decltype(raw)> RXDIS;      // bit  5 - Receiver Disable
            BitField<bool,  6, 1, decltype(raw)> TXEN;       // bit  6 - Transmitter Enable
            BitField<bool,  7, 1, decltype(raw)> TXDIS;      // bit  7 - Transmitter Disable

            BitField<bool,  8, 1, decltype(raw)> RSTSTA;     // bit  8 - Reset Status Bits
            BitField<bool,  9, 1, decltype(raw)> STTBRK;     // bit  9 - Start Break
            BitField<bool, 10, 1, decltype(raw)> STPBRK;    // bit 10 - Stop Break
            BitField<bool, 11, 1, decltype(raw)> STTTO;     // bit 11 - Start Time-out

            BitField<bool, 12, 1, decltype(raw)> SENDA;     // bit 12 - Send Address
            BitField<bool, 13, 1, decltype(raw)> RSTIT;     // bit 13 - Reset Iterations
            BitField<bool, 14, 1, decltype(raw)> RSTNACK;   // bit 14 - Reset Non Acknowledge
            BitField<bool, 15, 1, decltype(raw)> RETTO;     // bit 15 - Rearm Time-out

            BitField<bool, 18, 1, decltype(raw)> RTSEN;     // bit 18 - Request to Send Enable
            BitField<bool, 19, 1, decltype(raw)> RTSDIS;    // bit 19 - Request to Send Disable
            BitField<bool, 20, 1, decltype(raw)> LINABT;    // bit 20 - Abort LIN Transmission
            BitField<bool, 21, 1, decltype(raw)> LINWKUP;   // bit 21 - Send LIN Wakeup Signal

            US_CR_t() :
                RSTRX{raw},
                RSTTX{raw},

                RXEN{raw},
                RXDIS{raw},
                TXEN{raw},
                TXDIS{raw},

                RSTSTA{raw},
                STTBRK{raw},
                STPBRK{raw},
                STTTO{raw},

                SENDA{raw},
                RSTIT{raw},
                RSTNACK{raw},
                RETTO{raw},

                RTSEN{raw},
                RTSDIS{raw},
                LINABT{raw},
                LINWKUP{raw}
            {};
        } US_CR;


        enum class USART_MODE_t : uint32_t {
            USART_NORMAL    = 0x0,
            RS485           = 0x1,
            HW_HANDSHAKING  = 0x2,
            IS07816_T_0     = 0x4,
            IS07816_T_1     = 0x5,
            IRDA            = 0x8,
            LIN_MASTER      = 0xA,
            LIN_SLAVE       = 0xB,
            SPI_MASTER      = 0xE,
            SPI_SLAVE       = 0xF
        };

        enum USCLKS_t : uint32_t {
            MCK = 0x0,
            DIV = 0x1,
            SCK = 0x3
        };

        enum CHRL_t : uint32_t  {
            CH_5_BIT = 0x0,
            CH_6_BIT = 0x1,
            CH_7_BIT = 0x2,
            CH_8_BIT = 0x3
        };

        enum SYNC_t : uint32_t {
            ASYNC = 0x0,
            SYNC  = 0x1
        };

        enum PAR_t : uint32_t {
            EVEN        = 0x0,
            ODD         = 0x1,
            SPACE       = 0x2,
            MARK        = 0x3,
            NO          = 0x4,
            MULTIDROP   = 0x6
        };

        enum NBSTOP_t : uint32_t {
            STOP_1_BIT      = 0x0,
            STOP_1_5_BIT    = 0x1,
            STOP_2_BIT      = 0x2
        };

        enum CHMODE_t : uint32_t {
            CHMODE_NORMAL       = 0x0,
            AUTOMATIC           = 0x1,
            LOCAL_LOOPBACK      = 0x2,
            REMOTE_LOOPBACK     = 0x3
        };

        enum MSBF_t : uint32_t {
            LSB_FIRST = 0x0,
            MSB_FIRST = 0x1
        };

        enum OVER_t : uint32_t {
            x16 = 0x0,
            x8  = 0x1
        };

        struct US_MR_t {                 /**< \brief (Usart Offset: 0x0004) Mode Register */
            __IO uint32_t raw;

            BitField<USART_MODE_t ,  0, 4, decltype(raw)> USART_MODE;     // bits 0-3 - mode
            BitField<USCLKS_t     ,  4, 2, decltype(raw)> USCLKS;         // bits 4-5 - Clock Selection
            BitField<CHRL_t       ,  6, 2, decltype(raw)> CHRL;           // bits 6-7 - Character Length
            BitField<SYNC_t       ,  8, 1, decltype(raw)> SYNC;           // bit 8 - Synchronous Mode Select
            BitField<PAR_t        ,  9, 3, decltype(raw)> PAR;            // bits 9-11 - Parity Type
            BitField<NBSTOP_t     , 12, 2, decltype(raw)> NBSTOP;         // bits 12-13 - Number of Stop Bits
            BitField<CHMODE_t     , 14, 2, decltype(raw)> CHMODE;         // bits 14-15 - Channel Mode
            BitField<MSBF_t       , 16, 1, decltype(raw)> MSBF;           // bit 16 - Bit Order
            BitField<bool         , 17, 1, decltype(raw)> MODE9;          // bit 17 - 9-bit Character Length (CHRL ignored if true)
            BitField<bool         , 18, 1, decltype(raw)> CLKO;           // bit 18 - Clock Output Select
            BitField<OVER_t       , 19, 1, decltype(raw)> OVER;           // bit 19 - Oversampling Mode
            BitField<bool         , 20, 1, decltype(raw)> INACK;          // bit 20 - Inhibit Non Acknowledge
            BitField<bool         , 21, 1, decltype(raw)> DSNACK;         // bit 21 - Disable Successive NACK
            BitField<bool         , 22, 1, decltype(raw)> INVDATA;        // bit 22 - INverted Data
            BitField<bool         , 23, 1, decltype(raw)> VAR_SYNC;       // bit 23 - Variable Synchronization of Command/Data Sync Start Frame Delimiter
            BitField<uint8_t      , 24, 3, decltype(raw)> MAX_ITERATION;  // bits 24-26 - maximum number of iterations in mode ISO7816, protocol T= 0
            // skip bit 27
            BitField<bool         , 28, 1, decltype(raw)> FILTER;         // bit 28 - Infrared Receive Line Filter
            BitField<bool         , 29, 1, decltype(raw)> MAN;            // bit 29 - Manchester Encoder/Decoder Enable
            BitField<bool         , 30, 1, decltype(raw)> MODSYNC;        // bit 30 - Manchester Synchronization Mode
            BitField<bool         , 31, 1, decltype(raw)> ONEBIT;         // bit 31 - Start Frame Delimiter Selector

            US_MR_t() :
                USART_MODE{raw},
                USCLKS{raw},
                CHRL{raw},
                SYNC{raw},
                PAR{raw},
                NBSTOP{raw},
                CHMODE{raw},
                MSBF{raw},
                MODE9{raw},
                CLKO{raw},
                OVER{raw},
                INACK{raw},
                DSNACK{raw},
                INVDATA{raw},
                VAR_SYNC{raw},
                MAX_ITERATION{raw},

                FILTER{raw},
                MAN{raw},
                MODSYNC{raw},
                ONEBIT{raw}
            {};
        } US_MR;

        struct US_IER_t {                  /**< \brief (Usart Offset: 0x0008) Interrupt Enable Register */
            __O  uint32_t raw;

            BitField<bool ,  0, 1, decltype(raw)> RXRDY;       //  0 - RXRDY Interrupt Enable
            BitField<bool ,  1, 1, decltype(raw)> TXRDY;       //  1 - TXRDY Interrupt Enable
            BitField<bool ,  2, 1, decltype(raw)> RXBRK;       //  2 - Receiver Break Interrupt Enable
            BitField<bool ,  3, 1, decltype(raw)> ENDRX;       //  3 - End of Receive Transfer Interrupt Enable

            BitField<bool ,  4, 1, decltype(raw)> ENDTX;       //  4 - End of Transmit Interrupt Enable
            BitField<bool ,  5, 1, decltype(raw)> OVRE;        //  5 - Overrun Error Interrupt Enable
            BitField<bool ,  6, 1, decltype(raw)> FRAME;       //  6 - Framing Error Interrupt Enable
            BitField<bool ,  7, 1, decltype(raw)> PARE;        //  7 - Parity Error Interrupt Enable

            BitField<bool ,  8, 1, decltype(raw)> TIMEOUT;     //  8 - Time-out Interrupt Enable
            BitField<bool ,  9, 1, decltype(raw)> TXEMPTY;     //  9 - TXEMPTY Interrupt Enable
            BitField<bool , 10, 1, decltype(raw)> ITER_UNRE;   // 10 - Max number of Repetitions Reached / SPI Underrun Error
            BitField<bool , 11, 1, decltype(raw)> TXBUFE;      // 11 - Buffer Empty Interrupt Enable

            BitField<bool , 12, 1, decltype(raw)> RXBUFF;      // 12 - Buffer Full Interrupt Enable
            BitField<bool , 13, 1, decltype(raw)> NACK_LINBK;  // 13 - Non Acknowledge Interrupt Enable
                                                       //       / LIN Break Sent or LIN Break Received Interrupt Enable
            BitField<bool , 14, 1, decltype(raw)> LINID;       // 14 - LIN Identifier Sent or LIN Identifier Received Interrupt Enable
            BitField<bool , 15, 1, decltype(raw)> LINTC;       // 15 - LIN Transfer Completed Interrupt Enable

            // skip bits 16-18

            BitField<bool , 19, 1, decltype(raw)> CTSIC;       // 19 - Clear to Send Input Change Interrupt Enable

            // skip bits 20-23

            BitField<bool , 24, 1, decltype(raw)> MANE;        // 24 - Manchester Error Interrupt Enable
            BitField<bool , 25, 1, decltype(raw)> LINBE;       // 25 - LIN Bus Error Interrupt Enable
            BitField<bool , 26, 1, decltype(raw)> LINISFE;     // 26 - LIN Inconsistent Synch Field Error Interrupt Enable
            BitField<bool , 27, 1, decltype(raw)> LINIPE;      // 27 - LIN Identifier Parity Interrupt Enable

            BitField<bool , 28, 1, decltype(raw)> LINCE;       // 28 - LIN Checksum Error Interrupt Enable
            BitField<bool , 29, 1, decltype(raw)> LINSNRE;     // 29 - LIN Slave Not Responding Error Interrupt Enable

            US_IER_t() :
                RXRDY{raw},
                TXRDY{raw},
                RXBRK{raw},
                ENDRX{raw},

                ENDTX{raw},
                OVRE{raw},
                FRAME{raw},
                PARE{raw},

                TIMEOUT{raw},
                TXEMPTY{raw},
                ITER_UNRE{raw},
                TXBUFE{raw},

                RXBUFF{raw},
                NACK_LINBK{raw},
                LINID{raw},
                LINTC{raw},

                CTSIC{raw},

                MANE{raw},
                LINBE{raw},
                LINISFE{raw},
                LINIPE{raw},

                LINCE{raw},
                LINSNRE{raw}
            {};
        } US_IER;

        struct US_IDR_t {                  /**< \brief (Usart Offset: 0x000C) Interrupt Disable Register */
            __O  uint32_t raw;

            BitField<bool ,  0, 1, decltype(raw)> RXRDY;       //  0 - RXRDY Interrupt Disable
            BitField<bool ,  1, 1, decltype(raw)> TXRDY;       //  1 - TXRDY Interrupt Disable
            BitField<bool ,  2, 1, decltype(raw)> RXBRK;       //  2 - Receiver Break Interrupt Disable
            BitField<bool ,  3, 1, decltype(raw)> ENDRX;       //  3 - End of Receive Transfer Interrupt Disable

            BitField<bool ,  4, 1, decltype(raw)> ENDTX;       //  4 - End of Transmit Interrupt Disable
            BitField<bool ,  5, 1, decltype(raw)> OVRE;        //  5 - Overrun Error Interrupt Disable
            BitField<bool ,  6, 1, decltype(raw)> FRAME;       //  6 - Framing Error Interrupt Disable
            BitField<bool ,  7, 1, decltype(raw)> PARE;        //  7 - Parity Error Interrupt Disable

            BitField<bool ,  8, 1, decltype(raw)> TIMEOUT;     //  8 - Time-out Interrupt Disable
            BitField<bool ,  9, 1, decltype(raw)> TXEMPTY;     //  9 - TXEMPTY Interrupt Disable
            BitField<bool , 10, 1, decltype(raw)> ITER_UNRE;   // 10 - Max number of Repetitions Reached / SPI Underrun Error
            BitField<bool , 11, 1, decltype(raw)> TXBUFE;      // 11 - Buffer Empty Interrupt Disable

            BitField<bool , 12, 1, decltype(raw)> RXBUFF;      // 12 - Buffer Full Interrupt Disable
            BitField<bool , 13, 1, decltype(raw)> NACK_LINBK;  // 13 - Non Acknowledge Interrupt Disable
                                                       //       / LIN Break Sent or LIN Break Received Interrupt Disable
            BitField<bool , 14, 1, decltype(raw)> LINID;       // 14 - LIN Identifier Sent or LIN Identifier Received Interrupt Disable
            BitField<bool , 15, 1, decltype(raw)> LINTC;       // 15 - LIN Transfer Completed Interrupt Disable

             // skip bits 16-18

            BitField<bool , 19, 1, decltype(raw)> CTSIC;       // 19 - Clear to Send Input Change Interrupt Disable

            // skip bits 20-23

            BitField<bool , 24, 1, decltype(raw)> MANE;        // 24 - Manchester Error Interrupt Disable
            BitField<bool , 25, 1, decltype(raw)> LINBE;       // 25 - LIN Bus Error Interrupt Disable
            BitField<bool , 26, 1, decltype(raw)> LINISFE;     // 26 - LIN Inconsistent Synch Field Error Interrupt Disable
            BitField<bool , 27, 1, decltype(raw)> LINIPE;      // 27 - LIN Identifier Parity Interrupt Disable

            BitField<bool , 28, 1, decltype(raw)> LINCE;       // 28 - LIN Checksum Error Interrupt Disable
            BitField<bool , 29, 1, decltype(raw)> LINSNRE;     // 29 - LIN Slave Not Responding Error Interrupt Disable

            US_IDR_t() :
                RXRDY{raw},
                TXRDY{raw},
                RXBRK{raw},
                ENDRX{raw},

                ENDTX{raw},
                OVRE{raw},
                FRAME{raw},
                PARE{raw},

                TIMEOUT{raw},
                TXEMPTY{raw},
                ITER_UNRE{raw},
                TXBUFE{raw},

                RXBUFF{raw},
                NACK_LINBK{raw},
                LINID{raw},
                LINTC{raw},

                CTSIC{raw},

                MANE{raw},
                LINBE{raw},
                LINISFE{raw},
                LINIPE{raw},

                LINCE{raw},
                LINSNRE{raw}
            {};
        } US_IDR;

        struct US_IMR_t {                  /**< \brief (Usart Offset: 0x0010) Interrupt Mask Register */
            __I  uint32_t raw;

            BitField<bool ,  0, 1, decltype(raw)> RXRDY;       //  0 - RXRDY Interrupt Mask
            BitField<bool ,  1, 1, decltype(raw)> TXRDY;       //  1 - TXRDY Interrupt Mask
            BitField<bool ,  2, 1, decltype(raw)> RXBRK;       //  2 - Receiver Break Interrupt Mask
            BitField<bool ,  3, 1, decltype(raw)> ENDRX;       //  3 - End of Receive Transfer Interrupt Mask

            BitField<bool ,  4, 1, decltype(raw)> ENDTX;       //  4 - End of Transmit Interrupt Mask
            BitField<bool ,  5, 1, decltype(raw)> OVRE;        //  5 - Overrun Error Interrupt Mask
            BitField<bool ,  6, 1, decltype(raw)> FRAME;       //  6 - Framing Error Interrupt Mask
            BitField<bool ,  7, 1, decltype(raw)> PARE;        //  7 - Parity Error Interrupt Mask

            BitField<bool ,  8, 1, decltype(raw)> TIMEOUT;     //  8 - Time-out Interrupt Mask
            BitField<bool ,  9, 1, decltype(raw)> TXEMPTY;     //  9 - TXEMPTY Interrupt Mask
            BitField<bool , 10, 1, decltype(raw)> ITER_UNRE;   // 10 - Max number of Repetitions Reached / SPI Underrun Error
            BitField<bool , 11, 1, decltype(raw)> TXBUFE;      // 11 - Buffer Empty Interrupt Mask

            BitField<bool , 12, 1, decltype(raw)> RXBUFF;      // 12 - Buffer Full Interrupt Mask
            BitField<bool , 13, 1, decltype(raw)> NACK_LINBK;  // 13 - Non Acknowledge Interrupt Mask
                                                       //       / LIN Break Sent or LIN Break Received Interrupt Mask
            BitField<bool , 14, 1, decltype(raw)> LINID;       // 14 - LIN Identifier Sent or LIN Identifier Received Interrupt Mask
            BitField<bool , 15, 1, decltype(raw)> LINTC;       // 15 - LIN Transfer Completed Interrupt Mask

            // skip bits 16-18

            BitField<bool , 19, 1, decltype(raw)> CTSIC;       // 19 - Clear to Send Input Change Interrupt Mask

            // skip bits 20-23

            BitField<bool , 24, 1, decltype(raw)> MANE;        // 24 - Manchester Error Interrupt Mask
            BitField<bool , 25, 1, decltype(raw)> LINBE;       // 25 - LIN Bus Error Interrupt Mask
            BitField<bool , 26, 1, decltype(raw)> LINISFE;     // 26 - LIN Inconsistent Synch Field Error Interrupt Mask
            BitField<bool , 27, 1, decltype(raw)> LINIPE;      // 27 - LIN Identifier Parity Interrupt Mask

            BitField<bool , 28, 1, decltype(raw)> LINCE;       // 28 - LIN Checksum Error Interrupt Mask
            BitField<bool , 29, 1, decltype(raw)> LINSNRE;     // 29 - LIN Slave Not Responding Error Interrupt Mask

            US_IMR_t() :
                RXRDY{raw},
                TXRDY{raw},
                RXBRK{raw},
                ENDRX{raw},

                ENDTX{raw},
                OVRE{raw},
                FRAME{raw},
                PARE{raw},

                TIMEOUT{raw},
                TXEMPTY{raw},
                ITER_UNRE{raw},
                TXBUFE{raw},

                RXBUFF{raw},
                NACK_LINBK{raw},
                LINID{raw},
                LINTC{raw},

                CTSIC{raw},

                MANE{raw},
                LINBE{raw},
                LINISFE{raw},
                LINIPE{raw},

                LINCE{raw},
                LINSNRE{raw}
            {};
        } US_IMR;

        struct US_CSR_t {                      /**< \brief (Usart Offset: 0x0014) Channel Status Register */
            __I  uint32_t raw;

            BitField<volatile bool ,  0, 1, decltype(raw)> RXRDY;       //  0 - RXRDY Status
            BitField<volatile bool ,  1, 1, decltype(raw)> TXRDY;       //  1 - TXRDY Status
            BitField<volatile bool ,  2, 1, decltype(raw)> RXBRK;       //  2 - Receiver Break Status
            BitField<volatile bool ,  3, 1, decltype(raw)> ENDRX;       //  3 - End of Receive Transfer Status

            BitField<volatile bool ,  4, 1, decltype(raw)> ENDTX;       //  4 - End of Transmit Status
            BitField<volatile bool ,  5, 1, decltype(raw)> OVRE;        //  5 - Overrun Error Status
            BitField<volatile bool ,  6, 1, decltype(raw)> FRAME;       //  6 - Framing Error Status
            BitField<volatile bool ,  7, 1, decltype(raw)> PARE;        //  7 - Parity Error Status

            BitField<volatile bool ,  8, 1, decltype(raw)> TIMEOUT;     //  8 - Time-out Status
            BitField<volatile bool ,  9, 1, decltype(raw)> TXEMPTY;     //  9 - TXEMPTY Status
            BitField<volatile bool , 10, 1, decltype(raw)> ITER_UNRE;   // 10 - Max number of Repetitions Reached / SPI Underrun Error
            BitField<volatile bool , 11, 1, decltype(raw)> TXBUFE;      // 11 - Buffer Empty Status

            BitField<volatile bool , 12, 1, decltype(raw)> RXBUFF;      // 12 - Buffer Full Status
            BitField<volatile bool , 13, 1, decltype(raw)> NACK_LINBK;  // 13 - Non Acknowledge Status
                                                                //       / LIN Break Sent or LIN Break Received Status
            BitField<volatile bool , 14, 1, decltype(raw)> LINID;       // 14 - LIN Identifier Sent or LIN Identifier Received Status
            BitField<volatile bool , 15, 1, decltype(raw)> LINTC;       // 15 - LIN Transfer Completed Status
            // skip bits 16-18
            BitField<volatile bool , 19, 1, decltype(raw)> CTSIC;       // 19 - Clear to Send Input Change Status
            // skip bits 20-23
            BitField<volatile bool , 24, 1, decltype(raw)> CTS;         // 24 - Image of CTS Input

            BitField<volatile bool , 24, 1, decltype(raw)> MANE;        // 24 - Manchester Error Status
            BitField<volatile bool , 25, 1, decltype(raw)> LINBE;       // 25 - LIN Bus Error Status
            BitField<volatile bool , 26, 1, decltype(raw)> LINISFE;     // 26 - LIN Inconsistent Synch Field Error Status
            BitField<volatile bool , 27, 1, decltype(raw)> LINIPE;      // 27 - LIN Identifier Parity Status

            BitField<volatile bool , 28, 1, decltype(raw)> LINCE;       // 28 - LIN Checksum Error Status
            BitField<volatile bool , 29, 1, decltype(raw)> LINSNRE;     // 29 - LIN Slave Not Responding Error Status
            // skip bits 30-31


            US_CSR_t() :
                RXRDY{raw},
                TXRDY{raw},
                RXBRK{raw},
                ENDRX{raw},

                ENDTX{raw},
                OVRE{raw},
                FRAME{raw},
                PARE{raw},

                TIMEOUT{raw},
                TXEMPTY{raw},
                ITER_UNRE{raw},
                TXBUFE{raw},

                RXBUFF{raw},
                NACK_LINBK{raw},
                LINID{raw},
                LINTC{raw},
                CTSIC{raw},
                CTS{raw},

                MANE{raw},
                LINBE{raw},
                LINISFE{raw},
                LINIPE{raw},

                LINCE{raw},
                LINSNRE{raw}
            {};
        } US_CSR;

        struct US_RHR_t {                      /**< \brief (Usart Offset: 0x0018) Receive Holding Register */
            __I  uint32_t raw;

            BitField<uint16_t ,  0, 9, decltype(raw)> RXCHR;    // 0-8 - Received Character
            // skip bits 9-14
            BitField<bool     , 15, 1, decltype(raw)> RXSYNH;  // 15 - Received Sync

            US_RHR_t() :
                RXCHR{raw},
                RXSYNH{raw}
            {};
        } US_RHR;

        struct US_THR_t {                     /**< \brief (Usart Offset: 0x001C) Transmit Holding Register */
            __O  uint32_t raw;

            BitField<uint16_t ,  0, 9, decltype(raw)> TXCHR  ; // 0-8 - Character to be Transmitted
            // skip bits 9-14
            BitField<bool     , 15, 1, decltype(raw)> TXSYNH ; // 15 - Sync Field to be transmitted
            // skip bits 16-31

            US_THR_t() :
                TXCHR{raw},
                TXSYNH{raw}
            {};

        } US_THR;

        struct US_BRGR_t {                      /**< \brief (Usart Offset: 0x0020) Baud Rate Generator Register */
            __IO uint32_t raw;

            BitField<uint32_t ,  0, 16, decltype(raw)> CD     ; // 0-15 - Clock Divider
            BitField<uint32_t , 16,  3, decltype(raw)> FP     ; // 16-18 - Fractional Part. Baudrate resolution, defined by FP x 1/8.
            // skip bits 19-31

            US_BRGR_t() :
                CD{raw},
                FP{raw}
            {};
        }   US_BRGR;
        __IO uint32_t US_RTOR;       /**< \brief (Usart Offset: 0x0024) Receiver Time-out Register */
        __IO uint32_t US_TTGR;       /**< \brief (Usart Offset: 0x0028) Transmitter Timeguard Register */
        __I  uint32_t Reserved1[5];
        __IO uint32_t US_FIDI;       /**< \brief (Usart Offset: 0x0040) FI DI Ratio Register */
        __I  uint32_t US_NER;        /**< \brief (Usart Offset: 0x0044) Number of Errors Register */
        __I  uint32_t Reserved2[1];
        __IO uint32_t US_IF;         /**< \brief (Usart Offset: 0x004C) IrDA Filter Register */
        struct US_MAN_t {                     /**< \brief (Usart Offset: 0x0050) Manchester Configuration Register */
            __IO uint32_t raw;

            // Only bits that pertain to plain RS232 USART are represented.
            // skip bit 0 - 29
            BitField<bool , 30, 1, decltype(raw)> DRIFT      ; // 30 - Drift compensation
            // skip bit 31

            US_MAN_t() :
                DRIFT{raw}
            {};
        } US_MAN;
        __IO uint32_t US_LINMR;      /**< \brief (Usart Offset: 0x0054) LIN Mode Register */
        __IO uint32_t US_LINIR;      /**< \brief (Usart Offset: 0x0058) LIN Identifier Register */
        __I  uint32_t US_LINBRR;     /**< \brief (Usart Offset: 0x005C) LIN Baud Rate Register */
        __I  uint32_t Reserved3[33];
        __IO uint32_t US_WPMR;       /**< \brief (Usart Offset: 0x00E4) Write Protection Mode Register */
        __I  uint32_t US_WPSR;       /**< \brief (Usart Offset: 0x00E8) Write Protection Status Register */
        __I  uint32_t Reserved4[5];
        union {                     /**< \brief (Usart Offset: 0x100) Receive Pointer Register */
            __IO uint32_t _US_RPR;
            char * volatile US_RPR;
        };
        union { /**< \brief (Usart Offset: 0x104) Receive Counter Register */
            __IO uint32_t     _US_RCR;
            volatile uint16_t  US_RCR;
        };
        union {                      /**< \brief (Usart Offset: 0x108) Transmit Pointer Register */
            __IO uint32_t _US_TPR;
            char * volatile US_TPR;
        };
        union { /**< \brief (Usart Offset: 0x10C) Transmit Counter Register */
            __IO uint32_t     _US_TCR;
            volatile uint16_t  US_TCR;
        };

        union {                      /**< \brief (Usart Offset: 0x110) Receive Next Pointer Register */
            __IO uint32_t _US_RNPR;
            char * volatile US_RNPR;
        };
        union {                      /**< \brief (Usart Offset: 0x114) Receive Next Counter Register */
            __IO uint32_t     _US_RNCR;
            volatile uint16_t  US_RNCR;
        };
        union {                     /**< \brief (Usart Offset: 0x118) Transmit Next Pointer Register */
            __IO uint32_t   _US_TNPR;
            char * volatile  US_TNPR;
        };
        union {                     /**< \brief (Usart Offset: 0x11C) Transmit Next Counter Register */
            __IO uint32_t     _US_TNCR;
            volatile uint16_t  US_TNCR;
        };
        struct US_PTCR_t {                     /**< \brief (Usart Offset: 0x120) Transfer Control Register */
            __O  uint32_t raw;

            BitField<bool ,  0, 1, decltype(raw)> RXTEN  ; //  0 - Receiver Transfer Enable
            BitField<bool ,  1, 1, decltype(raw)> RXTDIS ; //  1 - Receiver Transfer Disable
            // skip bits 2-7
            BitField<bool ,  8, 1, decltype(raw)> TXTEN  ; //  8 - Transmitter Transfer Enable
            BitField<bool ,  9, 1, decltype(raw)> TXTDIS ; //  9 - Transmitter Transfer Disable
            // skip bits 10-31

            US_PTCR_t() :
                RXTEN{raw},
                RXTDIS{raw},
                TXTEN{raw},
                TXTDIS{raw}
            {};
        } US_PTCR;
        struct US_PTSR_t {                     /**< \brief (Usart Offset: 0x124) Transfer Status Register */
            __I  uint32_t raw;

            BitField<volatile bool ,  0, 1, decltype(raw)> RXTEN ; //  0 - Receiver Transfer Enable
            // skip bits 1-7
            BitField<volatile bool ,  8, 1, decltype(raw)> TXTEN ; //  8 - Transmitter Transfer Enable
            // skip bits 9-31

            US_PTSR_t() :
                RXTEN{raw},
                TXTEN{raw}
            {};
        } US_PTSR;
    } Usart;


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
            usart->US_CR.raw = US_CR_RSTRX | US_CR_RSTTX | US_CR_RXDIS | US_CR_TXDIS;
        };

        _UARTHardware() {
            // We DON'T init here, because the optimizer is fickle, and will remove this whole area.
            // Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };

        void enable() { usart->US_CR.TXEN = true; usart->US_CR.RXEN = true; };
        void disable () { usart->US_CR.TXDIS = true; usart->US_CR.RXDIS = true; };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

            // Oversampling is either 8 or 16. Depending on the baud, we may need to select 8x in
            // order to get the error low.

            // For all of the speeds up to and including 230400, 16x multiplier worked fine in testing.
            // All yielded a <1% error in final baud.
            usart->US_BRGR.CD = (((SystemCoreClock * 10) / (16 * baud)) + 5)/10;
            usart->US_BRGR.FP = 0;
            usart->US_MR.OVER = Usart::x16;


            if (options & UARTMode::RTSCTSFlowControl) {
                usart->US_MR.USART_MODE = Usart::USART_MODE_t::HW_HANDSHAKING;
            } else {
                usart->US_MR.USART_MODE = Usart::USART_MODE_t::USART_NORMAL;
            }

            if (options & UARTMode::TwoStopBits) {
                usart->US_MR.NBSTOP = Usart::STOP_2_BIT;
            } else {
                usart->US_MR.NBSTOP = Usart::STOP_1_BIT; // set explicitly
            }

            if (options & UARTMode::As9Bit) {
                usart->US_MR.MODE9 = true;
            } else {
                usart->US_MR.MODE9 = false;
                usart->US_MR.CHRL  = Usart::CH_8_BIT;
            }
            if (options & UARTMode::EvenParity) {
                usart->US_MR.PAR = Usart::EVEN;
            } else if (options & UARTMode::OddParity) {
                usart->US_MR.PAR = Usart::ODD;
            } else {
                usart->US_MR.PAR = Usart::NO;
            }

            /* Enable receiver and transmitter */
            enable();

        };

        void setInterrupts(const uint16_t interrupts) {
            if (interrupts != UARTInterrupt::Off) {

                if (interrupts & UARTInterrupt::OnRxDone) {
                    usart->US_IER.RXRDY = true;
                } else {
                    usart->US_IDR.RXRDY = true;
                }
                if (interrupts & UARTInterrupt::OnTxDone) {
                    usart->US_IER.TXRDY = true;
                } else {
                    usart->US_IDR.TXRDY = true;
                }

                if (interrupts & UARTInterrupt::OnRxTransferDone) {
                    usart->US_IER.ENDRX = true;
                } else {
                    usart->US_IDR.ENDRX = true;
                }
                if (interrupts & UARTInterrupt::OnTxTransferDone) {
                    usart->US_IER.ENDTX = true;
                } else {
                    usart->US_IDR.ENDTX = true;
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

        void _setInterruptTxReady(bool value) {
            if (value) {
                usart->US_IER.TXRDY = true;
            } else {
                usart->US_IDR.TXRDY = true;
            }
        };

        void _setInterruptCTSChange(bool value) {
            if (value) {
                usart->US_IER.CTSIC = true;
            } else {
                usart->US_IDR.CTSIC = true;
            }
        };

        void _setInterruptTxTransferDone(bool value) {
            if (value) {
                usart->US_IER.ENDTX = true;
            } else {
                usart->US_IDR.ENDTX = true;
            }
        };

        void _setInterruptRxTransferDone(bool value) {
            if (value) {
                usart->US_IER.ENDRX = true;
            } else {
                usart->US_IDR.ENDRX = true;
            }
        };

        static uint16_t getInterruptCause() __attribute__ (( noinline )) {
            uint16_t status = UARTInterrupt::Unknown;
            if (usart->US_CSR.TXRDY) {
                status |= UARTInterrupt::OnTxReady;
            }
            if (usart->US_CSR.ENDTX) {
                status |= UARTInterrupt::OnTxTransferDone;
            }
            if (usart->US_CSR.RXRDY) {
                status |= UARTInterrupt::OnRxReady;
            }
            if (usart->US_CSR.ENDRX) {
                status |= UARTInterrupt::OnRxTransferDone;
            }
            if (usart->US_CSR.CTSIC) {
                status |= UARTInterrupt::OnCTSChanged;
            }
            return status;
        }

        int16_t readByte() {
            if (usart->US_CSR.RXRDY) {
                return usart->US_RHR.RXCHR;
            }

            return -1;
        };

        int16_t writeByte(const char value) {
            if (usart->US_CSR.TXRDY) {
                usart->US_THR.TXCHR = value;
            }
            return -1;
        };

        void flush() {
            // Wait for the buffer to be empty
            while (!usart->US_CSR.TXEMPTY) {
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
            return usart->US_CSR.CTS;
        };


        // ***** Handle Tranfers
        bool startRXTransfer(char *buffer, const uint16_t length) {
            if (usart->US_RCR == 0) {
                usart->US_RPR = buffer;
                usart->US_RCR = length;
                usart->US_PTCR.RXTEN = true;
                return true;
            } else if (usart->US_RNCR == 0) {
                usart->US_RNPR = buffer;
                usart->US_RNCR = length;
                return true;
            }
            return false;
        };

        char* getRXTransferPosition() {
            return usart->US_RPR;
        };

        bool startTXTransfer(char *buffer, const uint16_t length) {
            if (usart->US_TCR == 0) {
                usart->US_TPR = buffer;
                usart->US_TCR = length;
                usart->US_PTCR.TXTEN = true;
                return true;
            } else if (usart->US_TNCR == 0) {
                usart->US_TNPR = buffer;
                usart->US_TNCR = length;
                return true;
            }
            return false;
        };

        char* getTXTransferPosition() {
            return usart->US_TPR;
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
    template<> const uint32_t  _UARTHardware<1>::peripheralId();
    template<> const uint32_t  _UARTHardware<2>::peripheralId();

    template<pin_number rxPinNumber, pin_number txPinNumber, pin_number rtsPinNumber = -1, pin_number ctsPinNumber = -1>
    struct UART {

        static_assert(UARTRxPin<rxPinNumber>::uartNum >= 0,
                      "USART RX Pin is not on a hardware USART.");

        static_assert(UARTTxPin<txPinNumber>::uartNum >= 0,
                      "USART TX Pin is not on a hardware USART.");

        static_assert(UARTRxPin<rxPinNumber>::uartNum == UARTTxPin<txPinNumber>::uartNum,
                      "USART RX Pin and TX Pin are not on the same hardware USART.");

        static_assert((rtsPinNumber == -1) || (UARTRTSPin<rtsPinNumber>::uartNum == UARTTxPin<rxPinNumber>::uartNum),
                      "USART RX Pin and RTS Pin are not on the same hardware USART.");

        static_assert((ctsPinNumber == -1) || (UARTRTSPin<ctsPinNumber>::uartNum == UARTTxPin<rxPinNumber>::uartNum),
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

        UART(const uint32_t baud = 115200, const uint16_t options = UARTMode::As8N1) {
            hardware.init();
            // Auto-enable RTS/CTS if the pins are provided.
            init(baud, options | (rtsPin.is_real ? UARTMode::RTSCTSFlowControl : 0), /*fromConstructor =*/ true);
        };

        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            setOptions(baud, options, fromConstructor);
            hardware._uartInterruptHandler = [&](uint16_t interruptCause) { // use a closure
                this->uartInterruptHandler(interruptCause);
            };
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

        bool isConnected() {
            // The cts pin allows to know if we're allowed to send,
            // which gives us a reasonable guess, at least.

            // The USART gives us access to that pin.
            return hardware.isConnceted();
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
            if(connection_state_changed_callback && hardware.isConnected()) {
                connection_state_changed_callback(true);
            }
        }


        bool startRXTransfer(char *buffer, const uint16_t length) {
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
            }

            if (interruptCause & UARTInterrupt::OnTxTransferDone) {
                if (transfer_tx_done_callback) {
                    transfer_tx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnRxTransferDone) {
                if (transfer_rx_done_callback) {
                    transfer_rx_done_callback();
                }
            }

            if (interruptCause & UARTInterrupt::OnCTSChanged) {
                if (connection_state_changed_callback) {
                    // We need to throttle this for MCU<->MCU connections.
                    connection_state_changed_callback(hardware.isConnected());
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
