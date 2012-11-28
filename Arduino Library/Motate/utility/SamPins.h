/*
  utility/AvrPins.hpp - Library for the Arduino-compatible Motate system
  http://tinkerin.gs/

  Copyright (c) 2012 Robert Giseburt

	This file is part of the Motate Library.

	The Motate Library is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	The Motate Library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public License
	along with the Motate Library.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SAMPINS_H_ONCE
#define SAMPINS_H_ONCE

namespace Motate {
	enum PinSetupType {
		Unchanged       = 0,
		Output          = 1,
		OutputOpendrain = 2,
		Input           = 3,
		InputWithPullup = 4,
		PeripheralA     = 5,
		PeripheralB     = 6,
	};

	template <unsigned char portLetter>
	struct Port {
	};
	
	template<int8_t pinNum>
	struct Pin {
	};
	
	typedef const int8_t pin_number;
	
	#define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerChar, registerPin)\
		template<>\
		struct Pin<pinNum> {\
			static const int8_t number = pinNum;\
			static const uint8_t portLetter = (uint8_t) registerChar;\
			static const uint32_t mask = (1 << registerPin);\
			static const Pio* portPtr = (PIO ## registerLetter);\
				\
			Pin(const PinSetupType type = Unchanged) {\
				init(type);\
			};\
			Pin<pinNum> &operator=(const bool value) { set(value); return *this; };\
			Pin<pinNum> &operator=(const PinSetupType type) { init(type); return *this; };\
			operator bool() { return (get() != 0); };\
		\
			void init(const PinSetupType type) {\
				switch (type) {\
					case Output:\
					case OutputOpendrain:\
						PIO_Configure(\
							(PIO ## registerLetter),\
							PIO_OUTPUT_0,\
							_BV(registerPin),\
							(type == OutputOpendrain) ? PIO_OPENDRAIN : PIO_DEFAULT ) ;\
		\
						/* if all pins are output, disable PIO Controller clocking, reduce power consumption */\
						if ( portPtr->PIO_OSR == 0xffffffff )\
						{\
							pmc_disable_periph_clk( (ID_PIO ## registerLetter) ) ) ;\
						}\
						break;\
		\
					case InputWithPullup:\
					case Input:\
						pmc_enable_periph_clk( (ID_PIO ## registerLetter) ) ;\
						PIO_Configure(\
							(PIO ## registerLetter),\
							PIO_INPUT,\
							_BV(registerPin),\
							(type == InputWithPullup) ? PIO_PULLUP : PIO_DEFAULT ) ;\
						break;\

					default:\
						break;\
				}\
			};\
			void set(bool value)  {\
				if (!value)\
					portPtr->PIO_CODR = _BV(registerPin) ;\
				else\
					portPtr->PIO_SODR = _BV(registerPin) ;\
			};\
			uint8_t get() {\
				return portPtr->PIO_PDSR & _BV(registerPin);\
			};\
			bool isNull() { return false; };\
			static uint8_t maskForPort(const uint8_t otherPortLetter) {\
				return portLetter == otherPortLetter ? mask : 0x00;\
			};\
		};\
		typedef Pin<pinNum> Pin ## pinNum;\
		static Pin ## pinNum pin ## pinNum;


	#define _MAKE_MOTATE_PORT32(registerLetter, registerChar)\
	template<>\
	struct Port<registerChar> {\
		typedef uint32_t uintPort_t;\
		static const Pio* portPtr = (PIO ## registerLetter);\
		static const uint8_t letter = (uint8_t) portLetter;\
		setDirection(const uintPort_t value, const uintPort_t mask) {\
			if (mask != 0xff) {\
				/* Set the masked 1 bits as ouputs */\
				portPtr->PIO_OER = value & mask;\
				/* Set the masked 0 bits as inputs */\
				portPtr->PIO_ODR = ~value & mask;\
			} else {\
				/* Set the whole port */\
				portPtr->PIO_OSR = value;\
			}\
		};\
		setPins(const uintPort_t value, const uintPort_t mask) {\
			if (mask != ~0x00) {\
				portPtr->PIO_OWDR = ~mask;/*Disable writing to unmasked pins*/\
			}\
			portPtr->PIO_ODSR = value;\
			if (mask != ~0x00) {\
				portPtr->PIO_OWER = ~mask;/*Re-enable writing to unmasked pins*/\
			}\
		};\
		getPins(const uintPort_t mask) {\
			return portPtr->PIO_PDSR & (mask);\
		}\
	};\
	typedef Port<uint32_t, registerChar> Port ## registerLetter;\
	static Port ## registerLetter port ## registerLetter;

	typedef Pin<-1> NullPin;
	static NullPin nullPin;

	#if part_is_defined( SAM3X8E )

	// DUE
		_MAKE_MOTATE_PORT32(A, 'A');
		_MAKE_MOTATE_PORT32(B, 'B');
		_MAKE_MOTATE_PORT32(C, 'C');
		_MAKE_MOTATE_PORT32(D, 'D');

		_MAKE_MOTATE_PIN( 0, A, 'A', 08);
		_MAKE_MOTATE_PIN( 1, A, 'A', 09);
		_MAKE_MOTATE_PIN( 2, B, 'B', 25);
		_MAKE_MOTATE_PIN( 3, C, 'C', 28);
		_MAKE_MOTATE_PIN( 4, C, 'C', 26);
		_MAKE_MOTATE_PIN( 5, C, 'C', 25);
		_MAKE_MOTATE_PIN( 6, C, 'C', 24);
		_MAKE_MOTATE_PIN( 7, C, 'C', 23);
		_MAKE_MOTATE_PIN( 8, C, 'C', 22);
		_MAKE_MOTATE_PIN( 9, C, 'C', 21);
		
		_MAKE_MOTATE_PIN(10, C, 'C', 29);
		_MAKE_MOTATE_PIN(11, D, 'D', 07);
		_MAKE_MOTATE_PIN(12, D, 'D', 08);
		
		_MAKE_MOTATE_PIN(13, B, 'B', 27);
		
		_MAKE_MOTATE_PIN(14, D, 'D', 04);
		_MAKE_MOTATE_PIN(15, D, 'D', 05);
		
		_MAKE_MOTATE_PIN(16, A, 'A', 13);
		_MAKE_MOTATE_PIN(17, A, 'A', 12);
		
		_MAKE_MOTATE_PIN(18, A, 'A', 11);
		_MAKE_MOTATE_PIN(19, A, 'A', 10);
		
		_MAKE_MOTATE_PIN(20, B, 'B', 12);
		_MAKE_MOTATE_PIN(21, B, 'B', 13);
		
		_MAKE_MOTATE_PIN(22, B, 'B', 26);
		_MAKE_MOTATE_PIN(23, A, 'A', 14);
		_MAKE_MOTATE_PIN(24, A, 'A', 15);
		_MAKE_MOTATE_PIN(25, D, 'D', 00);
		
		_MAKE_MOTATE_PIN(26, D, 'D', 01);
		_MAKE_MOTATE_PIN(27, D, 'D', 02);
		_MAKE_MOTATE_PIN(28, D, 'D', 03);
		_MAKE_MOTATE_PIN(29, D, 'D', 06);
		
		_MAKE_MOTATE_PIN(30, D, 'D', 09);
		_MAKE_MOTATE_PIN(31, A, 'A', 07);
		_MAKE_MOTATE_PIN(32, D, 'D', 10);
		_MAKE_MOTATE_PIN(33, C, 'C', 01);
		
		_MAKE_MOTATE_PIN(34, C, 'C', 02);
		_MAKE_MOTATE_PIN(35, C, 'C', 03);
		_MAKE_MOTATE_PIN(36, C, 'C', 04);
		_MAKE_MOTATE_PIN(37, C, 'C', 05);
		
		_MAKE_MOTATE_PIN(38, C, 'C', 06);
		_MAKE_MOTATE_PIN(39, C, 'C', 07);
		_MAKE_MOTATE_PIN(40, C, 'C', 08);
		_MAKE_MOTATE_PIN(41, C, 'C', 09);
		
		_MAKE_MOTATE_PIN(42, A, 'A', 19);
		_MAKE_MOTATE_PIN(43, A, 'A', 20);
		_MAKE_MOTATE_PIN(44, C, 'C', 19);
		_MAKE_MOTATE_PIN(45, C, 'C', 18);
		
		_MAKE_MOTATE_PIN(46, C, 'C', 17);
		_MAKE_MOTATE_PIN(47, C, 'C', 16);
		_MAKE_MOTATE_PIN(48, C, 'C', 15);
		_MAKE_MOTATE_PIN(49, C, 'C', 14);
		
		_MAKE_MOTATE_PIN(50, C, 'C', 13);
		_MAKE_MOTATE_PIN(51, C, 'C', 12);
		_MAKE_MOTATE_PIN(52, B, 'B', 21);
		_MAKE_MOTATE_PIN(53, B, 'B', 14);
		
		_MAKE_MOTATE_PIN(54, A, 'A', 16);
		_MAKE_MOTATE_PIN(55, A, 'A', 24);
		_MAKE_MOTATE_PIN(56, A, 'A', 23);
		_MAKE_MOTATE_PIN(57, A, 'A', 22);
		
		_MAKE_MOTATE_PIN(58, A, 'A', 06);
		_MAKE_MOTATE_PIN(69, A, 'A', 04);
		_MAKE_MOTATE_PIN(60, A, 'A', 03);
		_MAKE_MOTATE_PIN(61, A, 'A', 02);
		
		_MAKE_MOTATE_PIN(62, B, 'B', 17);
		_MAKE_MOTATE_PIN(63, B, 'B', 18);
		_MAKE_MOTATE_PIN(64, B, 'B', 19);
		_MAKE_MOTATE_PIN(65, B, 'B', 20);
		
		_MAKE_MOTATE_PIN(66, B, 'B', 15);
		_MAKE_MOTATE_PIN(67, B, 'B', 16);
		
		_MAKE_MOTATE_PIN(68, A, 'A', 01);
		_MAKE_MOTATE_PIN(69, A, 'A', 00);
		
		_MAKE_MOTATE_PIN(70, A, 'A', 17);
		_MAKE_MOTATE_PIN(71, A, 'A', 18);
		
		_MAKE_MOTATE_PIN(72, C, 'C', 30);
		
		_MAKE_MOTATE_PIN(73, A, 'A', 21);
		
		_MAKE_MOTATE_PIN(74, A, 'A', 25);
		_MAKE_MOTATE_PIN(75, A, 'A', 26);
		_MAKE_MOTATE_PIN(76, A, 'A', 27);
		
		_MAKE_MOTATE_PIN(77, A, 'A', 28);
		_MAKE_MOTATE_PIN(78, B, 'B', 23);
	#endif

// disable pinholder for Due for now -- nned to convert to 32bit
#if 0
	// PinHolder - virtual ports
	template<uint8_t PinBit7num, uint8_t PinBit6num, uint8_t PinBit5num = -1, uint8_t PinBit4num = -1, uint8_t PinBit3num = -1, uint8_t PinBit2num = -1, uint8_t PinBit1num = -1, uint8_t PinBit0num = -1>
	class PinHolder {

		static Pin<PinBit7num> PinBit7;
		static Pin<PinBit6num> PinBit6;
		static Pin<PinBit5num> PinBit5;
		static Pin<PinBit4num> PinBit4;
		static Pin<PinBit3num> PinBit3;
		static Pin<PinBit2num> PinBit2;
		static Pin<PinBit1num> PinBit1;
		static Pin<PinBit0num> PinBit0;

#define _MOTATE_CREATE_CLEAR_AND_COPY_MASKS(aPortLetter) \
		static const uint8_t port ## aPortLetter ## ClearMask =\
			(Pin<PinBit7num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit7num>::mask : 0x00) |\
			(Pin<PinBit6num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit6num>::mask : 0x00) |\
			(Pin<PinBit5num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit5num>::mask : 0x00) |\
			(Pin<PinBit4num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit4num>::mask : 0x00) |\
			(Pin<PinBit3num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit3num>::mask : 0x00) |\
			(Pin<PinBit2num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit2num>::mask : 0x00) |\
			(Pin<PinBit1num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit1num>::mask : 0x00) |\
			(Pin<PinBit0num>::portLetter == Port ## aPortLetter::letter ? Pin<PinBit0num>::mask : 0x00);\
\
		static const uint8_t port ## aPortLetter ## CopyMask =\
			(Pin<PinBit7num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit7num>::mask == (1 << 7 ) \
				? Pin<PinBit7num>::mask : 0x00) |\
			(Pin<PinBit6num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit6num>::mask == (1 << 6 )\
				? Pin<PinBit6num>::mask : 0x00) |\
			(Pin<PinBit5num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit5num>::mask == (1 << 5 )\
				? Pin<PinBit5num>::mask : 0x00) |\
			(Pin<PinBit4num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit4num>::mask == (1 << 4 )\
				? Pin<PinBit4num>::mask : 0x00) |\
			(Pin<PinBit3num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit3num>::mask == (1 << 3 )\
				? Pin<PinBit3num>::mask : 0x00) |\
			(Pin<PinBit2num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit2num>::mask == (1 << 2 )\
				? Pin<PinBit2num>::mask : 0x00) |\
			(Pin<PinBit1num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit1num>::mask == (1 << 1 )\
				? Pin<PinBit1num>::mask : 0x00) |\
			(Pin<PinBit0num>::portLetter == Port ## aPortLetter::letter && Pin<PinBit0num>::mask == (1 << 0 )\
				? Pin<PinBit0num>::mask : 0x00);

		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(B);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(C);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(D);
				
	public:
		PinHolder() {
			
		};
		
		void set(uint8_t in_value) {
			uint8_t port_value    = 0x00; // Port<> handles reading the port and setting the masked pins
#define _MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, bitNumber, bitMask) \
			if (PinBit ## bitNumber.maskForPort(port ## portLetter.letter) &&\
					(PinBit ## bitNumber.mask != (bitMask)) && (in_value & (bitMask))) {\
				port_value |= PinBit ## bitNumber.mask;\
			}
			
#define _MOTATE_PINHOLDER_SETPORT(portLetter) \
			if (port ## portLetter ## ClearMask != 0x00) {\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 7, 0b10000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 6, 0b01000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 5, 0b00100000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 4, 0b00010000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 3, 0b00001000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 2, 0b00000100);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 1, 0b00000010);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 0, 0b00000001);\
				port_value |= in_value & port ## portLetter ## CopyMask;\
				port ## portLetter.setPins(port_value, ~port ## portLetter ## ClearMask);\
			}
			
			_MOTATE_PINHOLDER_SETPORT(B);
			_MOTATE_PINHOLDER_SETPORT(C);
			_MOTATE_PINHOLDER_SETPORT(D);
		}
		
	};
#endif
}
#endif /* end of include guard: SAMPINS_H_ONCE */