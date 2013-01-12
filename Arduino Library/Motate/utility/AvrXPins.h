/*
  utility/AvrXPins.hpp - Library for the Motate system
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

#ifndef AVRPINS_H_ONCE
#define AVRPINS_H_ONCE

#include <avr/io.h>
#include <util/atomic.h>

namespace Motate {
	enum PinMode {
		kUnchanged    = 0,
		kOutput       = 1,
		kInput        = 2,
	};
	
	enum PinOptions {
		kTotem        = 0,
		kFloat        = 0,
		kBusKeeper    = 1,
		kPulldown     = 2,
		kPullup       = 3,
		kWiredOr      = 4,
		kWiredAnd     = 5,
		kWiredOrPull  = 6,
		kWiredAndPull = 7,
	};
	
	template <unsigned char portLetter>
	struct Port8 {
		typedef uint8_t uintPort_t;
		static const uint8_t letter = (uint8_t) portLetter;
		
		void setModes(const uint8_t value, const uint8_t mask = 0xff) {
			// stub
		};
		void setOptions(const PinOptions options, const uint8_t mask) {
			// stub
		};
		void set(const uint8_t value) {
			// stub
		};
		void clear(const uint8_t value) {
			// stub
		};
		void write(const uint8_t value) {
			// stub
		};
		void write(const uint8_t value, const uint8_t mask) {
			// stub
		};
		uint8_t getInputValues(const uint8_t mask = 0xff) {
			// stub
			return 0;
		};
		uint8_t getOutputValues(const uint8_t mask = 0xff) {
			// stub
			return 0;
		};
	};
	
	template<int8_t pinNum>
	struct Pin {
	};
	template<int8_t pinNum>
	struct InPin {
	};
	template<int8_t pinNum>
	struct OutPin {
	};
	
	
	typedef const int8_t pin_number;
	
	#define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerChar, registerPin)\
		template<>\
		struct Pin<pinNum> {\
			static const int8_t number = pinNum;\
			static const uint8_t portLetter = (uint8_t) registerChar;\
			static const uint8_t mask = (1 << registerPin);\
				\
			Pin(const PinMode type = kUnchanged) {\
				init(type);\
			};\
			void operator=(const bool value) { set(value); return *this; };\
			operator bool() { return (get() != 0); };\
		\
			void init(const PinMode type, const PinOptions options = kTotem) {\
				switch (type) {\
					case kOutput:\
						(PORT ## registerLetter).DIR |= mask;\
						break;\
					case kInput:\
						(PORT ## registerLetter).DIR &= ~mask;\
						break;\
					default:\
						break;\
				}\
				switch (options) {\
					case kTotem:\
					/*case kFloat:*/\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_TOTEM_gc;\
						break;\
					case kBusKeeper:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_BUSKEEPER_gc;\
						break;\
					case kPulldown:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_PULLDOWN_gc;\
						break;\
					case kPullup:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_PULLUP_gc;\
						break;\
					case kWiredOr:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDOR_gc;\
						break;\
					case kWiredAnd:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDAND_gc;\
						break;\
					case kWiredOrPull:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDORPULL_gc;\
						break;\
					case kWiredAndPull:\
						(PORT ## registerLetter).PIN ## registerPin ## CTRL = PORT_OPC_WIREDANDPULL_gc;\
						break;\
					default:\
						break;\
				}\
			};\
			void set(bool value)  {\
				if (value)\
					(PORT ## registerLetter).OUTSET = mask;\
				else\
					(PORT ## registerLetter).OUTCLR = mask;\
			};\
			uint8_t get() {\
				return ((PORT ## registerLetter).DIR & mask) ? ((PORT ## registerLetter).OUT & mask) : ((PORT ## registerLetter).IN & mask);\
			};\
			uint8_t getInput() {\
				return ((PORT ## registerLetter).IN & mask);\
			};\
			uint8_t getOutput() {\
				return ((PORT ## registerLetter).OUT & mask);\
			};\
			bool isNull() { return false; };\
			static uint8_t maskForPort(const uint8_t otherPortLetter) {\
				return portLetter == otherPortLetter ? mask : 0x00;\
			};\
		};\
		template<>\
		struct InputPin<pinNum> : Pin<pinNum> {\
			void init() {init(kInput)};\
			void set() {UnDefinedFunction();};\
			uint8_t get() {\
				return ((PORT ## registerLetter).IN & mask);\
			};\
			/*Override these to pick up new methods */\
			void operator=(const bool value) { /* NULL METHOD */ return *this; };\
			operator bool() { return (get() != 0); };\
		}\
		template<>\
		struct OutputPin<pinNum> : Pin<pinNum> {\
			void init() {init(kOutput)};\
			uint8_t get() {\
				return ((PORT ## registerLetter).OUT & mask);\
			};\
			/*Override these to pick up new methods */\
			void operator=(const bool value) { set(value); return *this; };\
			operator bool() { return (get() != 0); };\
		}\
		typedef Pin<pinNum> Pin ## pinNum;\
		static Pin<pinNum> pin ## pinNum;\
		typedef InputPin<pinNum> InputPin ## pinNum;\
		static InputPin<pinNum> inputPin ## pinNum;\
		typedef OutputPin<pinNum> OutputPin ## pinNum;\
		static OutputPin<pinNum> outputPin ## pinNum;


	#define _MAKE_MOTATE_PORT8(registerLetter, registerChar)\
	template <> inline void Port8<registerChar>::setModes(const uint8_t value, const uint8_t mask) {\
		uint8_t port_value = 0;\
		if (mask != 0xff) {\
			port_value = (PORT ## registerLetter).DIR & mask;\
		}\
		(PORT ## registerLetter).DIR = port_value | value;\
	};\
	template <> inline void Port8<registerChar>::setOptions(const PinOptions options, const uint8_t mask) {\
		PORTCFG.MPCMASK = mask; /*Write the configuration to all the masked pins at once.*/\
		/* MPCMASK is automatically cleared after any PINnCTRL write completes.*/\
		switch (options) {\
			case kTotem:\
			/*case kFloat:*/\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_TOTEM_gc;\
				break;\
			case kBusKeeper:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_BUSKEEPER_gc;\
				break;\
			case kPulldown:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_PULLDOWN_gc;\
				break;\
			case kPullup:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_PULLUP_gc;\
				break;\
			case kWiredOr:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_WIREDOR_gc;\
				break;\
			case kWiredAnd:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_WIREDAND_gc;\
				break;\
			case kWiredOrPull:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_WIREDORPULL_gc;\
				break;\
			case kWiredAndPull:\
				(PORT ## registerLetter).PIN0CTRL = PORT_OPC_WIREDANDPULL_gc;\
				break;\
			default:\
				break;\
		}\
	};\
	template <> inline void Port8<registerChar>::set(const uint8_t value) {\
		(PORT ## registerLetter).OUTSET = value;\
	};\
	template <> inline void Port8<registerChar>::clear(const uint8_t value) {\
		(PORT ## registerLetter).OUTCLR = value;\
	};\
	template <> inline void Port8<registerChar>::write(const uint8_t value) {\
		(PORT ## registerLetter).OUT = value;\
	};\
	template <> inline void Port8<registerChar>::write(const uint8_t value, const uint8_t mask) {\
		uint8_t port_value = 0;\
		if (mask != 0xff) {\
			port_value = (PORT ## registerLetter).OUT & mask;\
		}\
		(PORT ## registerLetter).OUT = port_value | value;\
	};\
	template <> inline uint8_t Port8<registerChar>::getInputValues(const uint8_t mask) {\
		return (PORT ## registerLetter).IN & (mask);\
	}\
	template <> inline uint8_t Port8<registerChar>::getOutputValues(const uint8_t mask) {\
		return (PORT ## registerLetter).OUT & (mask);\
	}\
	typedef Port8<registerChar> Port ## registerLetter;\
	static Port ## registerLetter port ## registerLetter;

	typedef Pin<-1> NullPin;
	static NullPin nullPin;
	
	// TODO: Support other, non-atxmega192a3 XMegas
	
	

	_MAKE_MOTATE_PORT8(A ,'A');
	_MAKE_MOTATE_PORT8(B ,'B');
	_MAKE_MOTATE_PORT8(C ,'C');
	_MAKE_MOTATE_PORT8(D ,'D');
	_MAKE_MOTATE_PORT8(E ,'E');
	_MAKE_MOTATE_PORT8(F ,'F');

	// PinHolder - virtual ports
	template<uint8_t PinBit7num, uint8_t PinBit6num, uint8_t PinBit5num = -1, uint8_t PinBit4num = -1, uint8_t PinBit3num = -1, uint8_t PinBit2num = -1, uint8_t PinBit1num = -1, uint8_t PinBit0num = -1>
	class PinHolder8 {

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

		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(A);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(B);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(C);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(D);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(E);
		_MOTATE_CREATE_CLEAR_AND_COPY_MASKS(F);
				
	public:
		PinHolder8() {
			
		};
		
		void write(const uint8_t in_value, const uint8_t mask = 0xff) {
			uint8_t port_value;
#define _MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, bitNumber, bitMask) \
			if (PinBit ## bitNumber.maskForPort(port ## portLetter.letter) &&\
					(PinBit ## bitNumber.mask != (bitMask)) && (in_value & mask & (bitMask))) {\
				port_value |= PinBit ## bitNumber.mask;\
			}
			
#define _MOTATE_PINHOLDER_SETPORT(portLetter) \
			if (port ## portLetter ## ClearMask && mask) {\
				port_value = 0x00;\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 7, 0b10000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 6, 0b01000000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 5, 0b00100000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 4, 0b00010000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 3, 0b00001000);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 2, 0b00000100);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 1, 0b00000010);\
				_MOTATE_PINHOLDER_CHECKANDSETPIN(portLetter, 0, 0b00000001);\
				port_value |= in_value & port ## portLetter ## CopyMask;\
				port ## portLetter.write(port_value, ~(mask & port ## portLetter ## ClearMask));\
			}
			
			_MOTATE_PINHOLDER_SETPORT(A);
			_MOTATE_PINHOLDER_SETPORT(B);
			_MOTATE_PINHOLDER_SETPORT(C);
			_MOTATE_PINHOLDER_SETPORT(D);
			_MOTATE_PINHOLDER_SETPORT(E);
			_MOTATE_PINHOLDER_SETPORT(F);
		}
		
	};
} // namespace Motate
#endif /* end of include guard: AVRPINS_H_ONCE */