/*
  MotatePins.hpp - Library for the Arduino-compatible Motate system
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

#ifndef MOTATEPINS_H_ONCE
#define MOTATEPINS_H_ONCE

#include <avr/io.h>
#include <util/atomic.h>
#include <inttypes.h>

namespace Motate {
	typedef volatile uint8_t & reg8_t;	
	
	enum PinSetupType {
		Unchanged       = 0,
		Output          = 1,
		Input           = 2,
		InputWithPullup = 3,
	};


	namespace implementation {
		// valid_pin_check is a partial specialization based on pinNum
		// default is that the pin is not null
		template <uint8_t, class T = void> 
		struct valid_pin_check
		{
			bool isNull() { return false; };
		};

		// pin number -1 (255, technically, since it unsigned) is NULL
		template <class T> 
		struct valid_pin_check<-1, T> 
		{
			bool isNull() { return true; };
		};
	}
	
	struct PinBase {
		virtual void init(const PinSetupType type) = 0;
		virtual void set(bool value);
		virtual uint8_t get();
		
	};
	
	template<uint8_t pinNum>
	struct Pin : public PinBase, public implementation::valid_pin_check< pinNum > {
		enum { number = pinNum };
				
		Pin(const PinSetupType type = Unchanged) {
			init(type);
		};
		Pin<pinNum> &operator=(bool value) { set(value); return *this; };
		operator bool() { return (get() != 0); };
		void init(const PinSetupType type) {
			// stub
		};
		void set(bool value) {
			// stub
		};
		uint8_t get() {
			// stub
			return 0;
		};
	};
	
	// Explicitly pre-define a NullPin to compare to
	Pin<-1> NullPin;
	
	typedef const uint8_t pin_number;
	
	#define _MAKE_MOTATE_PIN(pinNum, registerLetter, registerPin)\
		template <> void Pin<pinNum>::init(const PinSetupType type) { \
			switch (type) {\
				case Output:\
					(DDR ## registerLetter) |= (1<<registerPin);\
					break;\
				case InputWithPullup:\
				case Input:\
					(DDR ## registerLetter) &= ~(1<<registerPin);\
					break;\
				default:\
					break;\
			}\
			if (type == InputWithPullup)  (PORT ## registerLetter) &= ~(1<<registerPin);\
		};\
		template <> void Pin<pinNum>::set(bool value)  { \
			ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {\
				if (!value)\
					(PORT ## registerLetter) &= ~(1<<registerPin);\
				else\
					(PORT ## registerLetter) |= (1<<registerPin);\
			}\
		};\
		template <> uint8_t Pin<pinNum>::get() { \
			return (PIN ## registerLetter) & (1<<registerPin);\
		};\
		typedef Pin<pinNum> Pin ## pinNum;\
		Pin<pinNum> pin ## pinNum;

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega328P__)

// standard (UNO, etc)
	_MAKE_MOTATE_PIN( 0, D, 0);
	_MAKE_MOTATE_PIN( 1, D, 1);
	_MAKE_MOTATE_PIN( 2, D, 2);
	_MAKE_MOTATE_PIN( 3, D, 3);
	_MAKE_MOTATE_PIN( 4, D, 4);
	_MAKE_MOTATE_PIN( 5, D, 5);
	_MAKE_MOTATE_PIN( 6, D, 6);
	_MAKE_MOTATE_PIN( 7, D, 7);
	_MAKE_MOTATE_PIN( 8, B, 0);
	_MAKE_MOTATE_PIN( 9, B, 1);
	_MAKE_MOTATE_PIN(10, B, 2);
	_MAKE_MOTATE_PIN(11, B, 3);
	_MAKE_MOTATE_PIN(12, B, 4);
	_MAKE_MOTATE_PIN(13, B, 5);
	_MAKE_MOTATE_PIN(14, C, 0);
	_MAKE_MOTATE_PIN(15, C, 1);
	_MAKE_MOTATE_PIN(16, C, 2);
	_MAKE_MOTATE_PIN(17, C, 3);
	_MAKE_MOTATE_PIN(18, C, 4);
	_MAKE_MOTATE_PIN(19, C, 5);
	
#elif defined(__AVR_ATmega32U4__)

// leonardo
	_MAKE_MOTATE_PIN(0 , D, 2);
	_MAKE_MOTATE_PIN(1 , D, 3);
	_MAKE_MOTATE_PIN(2 , D, 1);
	_MAKE_MOTATE_PIN(3 , D, 0);
	_MAKE_MOTATE_PIN(4 , D, 4);
	_MAKE_MOTATE_PIN(5 , C, 6);
	_MAKE_MOTATE_PIN(6 , D, 7);
	_MAKE_MOTATE_PIN(7 , E, 6);
                          
	_MAKE_MOTATE_PIN(8 , B, 4);
	_MAKE_MOTATE_PIN(9 , B, 5);
	_MAKE_MOTATE_PIN(10, B, 6);
	_MAKE_MOTATE_PIN(11, B, 7);
	_MAKE_MOTATE_PIN(12, D, 6);
	_MAKE_MOTATE_PIN(13, C, 7);
                          
	_MAKE_MOTATE_PIN(14, B, 3);
	_MAKE_MOTATE_PIN(15, B, 1);
	_MAKE_MOTATE_PIN(16, B, 2);
	_MAKE_MOTATE_PIN(17, B, 0);
                          
	_MAKE_MOTATE_PIN(18, F, 7);
	_MAKE_MOTATE_PIN(19, F, 6);
	_MAKE_MOTATE_PIN(20, F, 5);
	_MAKE_MOTATE_PIN(21, F, 4);
	_MAKE_MOTATE_PIN(22, F, 1);
	_MAKE_MOTATE_PIN(23, F, 0);
                          
	_MAKE_MOTATE_PIN(24, D, 4);
	_MAKE_MOTATE_PIN(25, D, 7);
	_MAKE_MOTATE_PIN(26, B, 4);
	_MAKE_MOTATE_PIN(27, B, 5);
	_MAKE_MOTATE_PIN(28, B, 6);
	_MAKE_MOTATE_PIN(29, D, 6);

#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

// MEGA
	_MAKE_MOTATE_PIN(0 , E, 0);
	_MAKE_MOTATE_PIN(1 , E, 1);
	_MAKE_MOTATE_PIN(2 , E, 4);
	_MAKE_MOTATE_PIN(3 , E, 5);
	_MAKE_MOTATE_PIN(4 , G, 5);
	_MAKE_MOTATE_PIN(5 , E, 3);
	_MAKE_MOTATE_PIN(6 , H, 3);
	_MAKE_MOTATE_PIN(7 , H, 4);
	_MAKE_MOTATE_PIN(8 , H, 5);
	_MAKE_MOTATE_PIN(9 , H, 6);
	_MAKE_MOTATE_PIN(10, B, 4);
	_MAKE_MOTATE_PIN(11, B, 5);
	_MAKE_MOTATE_PIN(12, B, 6);
	_MAKE_MOTATE_PIN(13, B, 7);
	_MAKE_MOTATE_PIN(14, J, 1);
	_MAKE_MOTATE_PIN(15, J, 0);
	_MAKE_MOTATE_PIN(16, H, 1);
	_MAKE_MOTATE_PIN(17, H, 0);
	_MAKE_MOTATE_PIN(18, D, 3);
	_MAKE_MOTATE_PIN(19, D, 2);
	_MAKE_MOTATE_PIN(20, D, 1);
	_MAKE_MOTATE_PIN(21, D, 0);
	_MAKE_MOTATE_PIN(22, A, 0);
	_MAKE_MOTATE_PIN(23, A, 1);
	_MAKE_MOTATE_PIN(24, A, 2);
	_MAKE_MOTATE_PIN(25, A, 3);
	_MAKE_MOTATE_PIN(26, A, 4);
	_MAKE_MOTATE_PIN(27, A, 5);
	_MAKE_MOTATE_PIN(28, A, 6);
	_MAKE_MOTATE_PIN(29, A, 7);
	_MAKE_MOTATE_PIN(30, C, 7);
	_MAKE_MOTATE_PIN(31, C, 6);
	_MAKE_MOTATE_PIN(32, C, 5);
	_MAKE_MOTATE_PIN(33, C, 4);
	_MAKE_MOTATE_PIN(34, C, 3);
	_MAKE_MOTATE_PIN(35, C, 2);
	_MAKE_MOTATE_PIN(36, C, 1);
	_MAKE_MOTATE_PIN(37, C, 0);
	_MAKE_MOTATE_PIN(38, D, 7);
	_MAKE_MOTATE_PIN(39, G, 2);
	_MAKE_MOTATE_PIN(40, G, 1);
	_MAKE_MOTATE_PIN(41, G, 0);
	_MAKE_MOTATE_PIN(42, L, 7);
	_MAKE_MOTATE_PIN(43, L, 6);
	_MAKE_MOTATE_PIN(44, L, 5);
	_MAKE_MOTATE_PIN(45, L, 4);
	_MAKE_MOTATE_PIN(46, L, 3);
	_MAKE_MOTATE_PIN(47, L, 2);
	_MAKE_MOTATE_PIN(48, L, 1);
	_MAKE_MOTATE_PIN(49, L, 0);
	_MAKE_MOTATE_PIN(50, B, 3);
	_MAKE_MOTATE_PIN(51, B, 2);
	_MAKE_MOTATE_PIN(52, B, 1);
	_MAKE_MOTATE_PIN(53, B, 0);
	_MAKE_MOTATE_PIN(54, F, 0);
	_MAKE_MOTATE_PIN(55, F, 1);
	_MAKE_MOTATE_PIN(56, F, 2);
	_MAKE_MOTATE_PIN(57, F, 3);
	_MAKE_MOTATE_PIN(58, F, 4);
	_MAKE_MOTATE_PIN(59, F, 5);
	_MAKE_MOTATE_PIN(60, F, 6);
	_MAKE_MOTATE_PIN(61, F, 7);
	_MAKE_MOTATE_PIN(62, K, 0);
	_MAKE_MOTATE_PIN(63, K, 1);
	_MAKE_MOTATE_PIN(64, K, 2);
	_MAKE_MOTATE_PIN(65, K, 3);
	_MAKE_MOTATE_PIN(66, K, 4);
	_MAKE_MOTATE_PIN(67, K, 5);
	_MAKE_MOTATE_PIN(68, K, 6);
	_MAKE_MOTATE_PIN(69, K, 7);
	
#endif

}

#endif /* end of include guard: MOTATEPINS_H_ONCE */