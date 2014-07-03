/*
 KL05ZUART.h - Library for the Motate system
 http://tinkerin.gs/
 
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

#ifndef KL05ZUART_H_ONCE
#define KL05ZUART_H_ONCE

#include "MotatePins.h"
#include <type_traits>

namespace Motate {

    enum UARTMode {

        kUARTNoParity          =      0, // Default
        kUARTEvenParity        = 1 << 0,
        kUARTOddParity         = 1 << 1,

		kUARTOneStopBit        =      0, // Default
		kUARTTwoStopBits       = 1 << 2,

        kUART8Bit              =      0, // Default
        kUART9Bit              = 1 << 3,
        kUART10Bit             = 1 << 4,

		 // Some careful hand math will show that kUART8N1 == 0
        kUART8N1              = kUART8Bit | kUARTNoParity | kUARTOneStopBit

		// TODO: Add polarity inversion and bit reversal options
	};


    // This is an internal representation of the peripheral.
    // This is *not* to be used externally.

	template<uint8_t uartPeripheralNumber, pin_number rxPinNumber, pin_number txPinNumber, typename T> struct _UARTHardware;

	template<uint8_t uartPeripheralNumber, pin_number rxPinNumber, pin_number txPinNumber, typename = void>
    struct _UARTHardware {
		static const uint8_t uartPeripheralNum=0xFF;

        // BITBANG HERE!
    };

	template<pin_number rxPinNumber, pin_number txPinNumber>
	using IsValidUART = typename std::enable_if<IsUARTRxPin<rxPinNumber>() && IsUARTTxPin<txPinNumber>() && rxPinNumber != txPinNumber && UARTTxPin<txPinNumber>::moduleId == UARTRxPin<rxPinNumber>::moduleId>::type;
    
    template<pin_number rxPinNumber, pin_number txPinNumber>
    struct _UARTHardware<0u, rxPinNumber, txPinNumber, IsValidUART<rxPinNumber, txPinNumber>> {
		static inline UART0_Type * const uart() { return UART0; };
		static const IRQn_Type spiIRQ() { return UART0_IRQn; };
        
        static const uint8_t uartPeripheralNum=0;
        
//        typedef _UARTHardware<0u, rxPinNumber, txPinNumber> this_type_t;

        void init() {
			// Enable the UART0 Clock Gate
			SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1); // select the FLLFLLCLK as UART0 clock source
            SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;

            disable();
            
        };
        
        _UARTHardware() {
			// We DON'T init here, because the optimizer is fickle, and will remove this whole area.
			// Instead, we call init from UART<>::init(), so that the optimizer will keep it.
        };

        void enable() {
			uart()->C2 |= (UART0_C2_TE_MASK | UART0_C2_RE_MASK);
        };
        
        void disable () {
			uart()->C2 &= ~(UART0_C2_TE_MASK | UART0_C2_RE_MASK);
        };

        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            disable();

			// This all breaks down to the SBR ("Baud Rate Modulo Divisor" -- how does SBR stand for that?)
			// formula, which sets the baud rate. One key component is the OSR ("Over Sampling Ratio"), which
			// is a number between 3 and 31, where the actual rate of oversampling is OSR+1.
			//
			// Final formula:
			//   baud = clock / ((OSR+1) × SBR).
			// Higher OSR is better.
			// Manual also says that for 4 <= OSR+1 <= 7, set sampling to "both edges."

            uint32_t best_baud_diff = 0xFFFFFF; // Start it high, we'll change in in a second...
			uint32_t sbr_value = 0;

            uint32_t oversample_rate = 4;
			for (uint32_t test_oversample_rate = 4; test_oversample_rate <= 32; test_oversample_rate++) {
				uint32_t temp_sbr_value = SystemCoreClock / (baud * test_oversample_rate);
				uint32_t calculated_baud = SystemCoreClock / (temp_sbr_value * test_oversample_rate);

				uint32_t baud_diff = calculated_baud > baud ? calculated_baud - baud : baud - calculated_baud;

				if (baud_diff <= best_baud_diff) {
					best_baud_diff = baud_diff;
					oversample_rate = test_oversample_rate;
					sbr_value = temp_sbr_value;
				}
			}

			// If the OSR is between 4x and 8x then both
			// edge sampling MUST be turned on.
			if (/*(oversample_rate > 3) && */(oversample_rate < 9))
				uart()->C5|= UART0_C5_BOTHEDGE_MASK;

			// Setup OSR value
			uart()->C4 = ( uart()->C4 & ~UART0_C4_OSR_MASK ) | UART0_C4_OSR(oversample_rate-1);

			/* Save off the current value of the uartx_BDH except for the SBR field */
			uart()->BDH = ( uart()->BDH & ~(UART0_BDH_SBR_MASK | UART0_BDH_SBNS_MASK) )
				| UART0_BDH_SBR( sbr_value >> 8 )
				| (options & kUARTTwoStopBits ? UART0_BDH_SBNS_MASK : 0);
			uart()->BDL = (uint8_t)(sbr_value & UART0_BDL_SBR_MASK);

			if (options & kUART10Bit) {
				uart()->C4 |= UART0_C4_M10_MASK;
			} else if (options & kUART9Bit) {
				uart()->C1 |= UART0_C1_M_MASK;
				uart()->C4 &= ~UART0_C4_M10_MASK;
			} else {
				uart()->C1 &= ~UART0_C1_M_MASK;
				uart()->C4 &= ~UART0_C4_M10_MASK;
			}

			if (options & (kUARTEvenParity|kUARTOddParity)) {
				// Do we need to enforce 9-bit mode here? The manual is ambiguous...
				uart()->C1 = (uart()->C1 & ~UART0_C1_PT_MASK) | UART0_C1_PE_MASK | (options & kUARTEvenParity ? 0 : UART0_C1_PT_MASK);
			} else {
				uart()->C1 &= ~UART0_C1_PE_MASK;
			}

			/* Enable receiver and transmitter */
            enable();
			
        };

        int16_t read() {
			if (uart()->S1 & UART0_S1_RDRF_MASK) {
				return uart()->D;
			}
			return -1;
        };

        int16_t write(uint8_t value) __attribute__((noinline)) {
            while (!(uart()->S1 & UART0_S1_TDRE_MASK)) {
				;
            }
			uart()->D = value;

            return 1;
        };

		void flush() {
			// Wait for the buffer to be empty
			while(!(uart()->S1 & UART0_S1_TDRE_MASK));
		}
    };
    
	template<pin_number rxPinNumber = ReversePinLookup<'B',  2>::number, pin_number txPinNumber = ReversePinLookup<'B',  1>::number>
	struct UART {
        UARTRxPin<rxPinNumber> rxPin;
        UARTTxPin<txPinNumber> txPin;

        static _UARTHardware< 0u, rxPinNumber, txPinNumber > hardware;
        static const uint8_t uartPeripheralNum() { return hardware.moduleId; };

        UART(const uint32_t baud = 115200, const uint16_t options = kUART8N1) {
            hardware.init();
            init(baud, options, /*fromConstructor =*/ true);
        };
        
        void init(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            setOptions(baud, options, fromConstructor);
        };
        
        void setOptions(const uint32_t baud, const uint16_t options, const bool fromConstructor=false) {
            hardware.setOptions(baud, options, fromConstructor);
        };

		int16_t read() {
            return hardware.read();
		};
        
        // WARNING: Currently only reads in bytes. For more-that-byte size data, we'll need another call.
		int16_t read(const uint8_t *buffer, const uint16_t length) {
			int16_t total_read = 0;
			int16_t to_read = length;
			const uint8_t *read_ptr = buffer;

			// BLOCKING!!
			while (to_read > 0) {
                
				int16_t ret = read();
                
                if (ret >= 0) {
                    *read_ptr++ = ret;
                    total_read++;
                    to_read--;
                }
			};
            
			return total_read;
		};
        
        int16_t write(uint16_t data) {
            return hardware.write(data);
		};

        void flush() {
			// Wait for the buffer to be empty...
			hardware.flush();
		};
        
        // WARNING: Currently only writes in bytes. For more-that-byte size data, we'll need another call.
//		int16_t write(const char *data, const uint16_t length, bool autoFlush = false) {
//			return write(data, length, autoFlush);
//		};
//
		int16_t write(const uint8_t *data, const uint16_t length, bool autoFlush = false) __attribute__((noinline)) {
            int16_t total_written = 0;
			const uint8_t *out_buffer = data;
			int16_t to_write = length;

			// BLOCKING!!
			do {
                /* int16_t ret = */hardware.write(*out_buffer);
				
//                if (ret > 0) {
                    out_buffer++;
                    total_written++;
                    to_write--;
//                }
			} while (to_write);
            
			if (autoFlush && total_written > 0)
                flush();

			return total_written;
		}
	};
    
}

#endif /* end of include guard: KL05ZUART_H_ONCE */