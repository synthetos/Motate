#ifndef MotateLCD_h
#define MotateLCD_h

#include <inttypes.h>
#include "Print.h"
#include "../Motate/MotatePins.h"

#include "Arduino.h" // delay

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// using Motate::Pin; 
// using Motate::Output; 
// using Motate::Input; 
namespace Motate {

namespace implementation {
	// here we'll specialize a few routines for 4 or 8-bit mode
	template<uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
	class LiquidCrystal {
	public:
	  uint8_t _displayfunction;

		// 8-bit mode
		Pin<rs> _rs_pin; // LOW: command.  HIGH: character.
	  Pin<rw> _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
	  Pin<enable> _enable_pin; // activated by a HIGH pulse.
	  Pin<d0> _data_pin0;
	  Pin<d1> _data_pin1;
	  Pin<d2> _data_pin2;
	  Pin<d3> _data_pin3;
	  Pin<d4> _data_pin4;
	  Pin<d5> _data_pin5;
	  Pin<d6> _data_pin6;
	  Pin<d7> _data_pin7;
		PinHolder<d7, d6, d5, d4, d3, d2, d1, d0> _data;

		LiquidCrystal() : _rs_pin(Output), _rw_pin(Output), _enable_pin(Output), _data_pin0(Output), _data_pin1(Output), _data_pin2(Output), _data_pin3(Output), _data_pin4(Output), _data_pin5(Output), _data_pin6(Output), _data_pin7(Output) {
  		// if there is a RW pin indicated, set it low to Write
			_rw_pin = LOW; // non-op if this is set to NullPin
			
			_displayfunction = LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;
		}

		void initMode();

	  void write8bits(uint8_t);
	};

	// here we'll specialize a few routines for 4 or 8-bit mode
	template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3>
	class LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, 255, 255, 255, 255> {
	public:
	  uint8_t _displayfunction;

		// 4-bit mode
		Pin<rs> _rs_pin; // LOW: command.  HIGH: character.
	  Pin<rw> _rw_pin; // LOW: write to LCD.  HIGH: read from LCD.
	  Pin<enable> _enable_pin; // activated by a HIGH pulse.
		PinHolder<d3, d2, d1, d0> _data_high;
		PinHolder<-1, -1, -1, -1, d3, d2, d1, d0> _data_low;
	  Pin<d0> _data_pin0;
	  Pin<d1> _data_pin1;
	  Pin<d2> _data_pin2;
	  Pin<d3> _data_pin3;

		LiquidCrystal() : _rs_pin(Output), _rw_pin(Output), _enable_pin(Output), _data_pin0(Output), _data_pin1(Output), _data_pin2(Output), _data_pin3(Output) {
  		// if there is a RW pin indicated, set it low to Write
			_rw_pin = LOW; // non-op if this is set to NullPin

	    _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
		}

		void initMode();
		
	  void write4bits(uint8_t);
	  void write8bits(uint8_t);
	};
}

template<uint8_t rs, uint8_t rw, uint8_t enable,
	uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
	uint8_t d4 = 255, uint8_t d5 = 255, uint8_t d6 = 255, uint8_t d7 = 255>
class LiquidCrystal
	: public implementation::LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>
	, public Print {

	typedef implementation::LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>  _lcd_base;

	/* The using is required. See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html */
	using  _lcd_base::_displayfunction;

	uint8_t _displaycontrol;
	uint8_t _displaymode;

	uint8_t _initialized;

	uint8_t _numlines,_currline;

public:

  void begin(uint8_t cols, uint8_t rows, uint8_t charsize = LCD_5x8DOTS);
  
  LiquidCrystal() : implementation::LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>() {
	  begin(16, 1);  
	};
    
  void clear();
  void home();

  void noDisplay();
  void display();
  void noBlink();
  void blink();
  void noCursor();
  void cursor();
  void scrollDisplayLeft();
  void scrollDisplayRight();
  void leftToRight();
  void rightToLeft();
  void autoscroll();
  void noAutoscroll();

  void createChar(uint8_t, uint8_t[]);
  void setCursor(uint8_t, uint8_t); 
  void command(uint8_t);

  virtual size_t write(uint8_t);
  using Print::write;
};



// Since this is a templated class, we have to implement it right here and now:

namespace implementation {
	template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3>
	void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, 255, 255, 255, 255>::initMode() {
	  // Now we pull both RS and R/W low to begin commands
	  _rs_pin = LOW;
	  _enable_pin = LOW;
	  if (_rw_pin != 255) { 
	    _rw_pin = LOW;
	  }

    // this is according to the hitachi HD44780 datasheet
    // figure 24, pg 46

    // we start in 8bit mode, try to set 4 bit mode
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms

    // second try
    write4bits(0x03);
    delayMicroseconds(4500); // wait min 4.1ms
  
    // third go!
    write4bits(0x03); 
    delayMicroseconds(150);

    // finally, set to 4-bit interface
    write4bits(0x02); 
	}
	
	template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3>
	void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, 255, 255, 255, 255>::write4bits(uint8_t value) {
		// _data_pin0 = value & 0x01;
		// value >>= 1;
		// _data_pin1 = value & 0x01;
		// value >>= 1;
		// _data_pin2 = value & 0x01;
		// value >>= 1;
		// _data_pin3 = value & 0x01;
		_data_low.set(value);
		
	  _enable_pin = LOW;
	  delayMicroseconds(1);    
	  _enable_pin = HIGH;
	  delayMicroseconds(1);    // enable pulse must be >450ns
	  _enable_pin = LOW;
	  delayMicroseconds(100);   // commands need > 37us to settle
	}

	template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3>
	void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, 255, 255, 255, 255>::write8bits(uint8_t value) {
		// still in four-bit mode
		// write the most significant 4 bits first, then pulse the enable
		// note that value_high_4 >>=4 is expensive, where (0x01 << 4) is cheap
		// uint8_t value_high_4 = value >> 4;
		// _data_pin0 = value_high_4 & 0x01;
		// value_high_4 >>= 1;
		// _data_pin1 = value_high_4 & 0x01;
		// value_high_4 >>= 1;
		// _data_pin2 = value_high_4 & 0x01;
		// value_high_4 >>= 1;
		// _data_pin3 = value_high_4 & 0x01;

		_data_high.set(value);
		
	  _enable_pin = LOW;
	  delayMicroseconds(1);    
	  _enable_pin = HIGH;
	  delayMicroseconds(1);    // enable pulse must be >450ns
	  _enable_pin = LOW;
	  delayMicroseconds(100);   // commands need > 37us to settle
	
		// // still in four-bit mode
		// _data_pin0 = value & 0x01;
		// value >>= 1;
		// _data_pin1 = value & 0x01;
		// value >>= 1;
		// _data_pin2 = value & 0x01;
		// value >>= 1;
		// _data_pin3 = value & 0x01;

		_data_low.set(value);
		
	  _enable_pin = LOW;
	  delayMicroseconds(1);    
	  _enable_pin = HIGH;
	  delayMicroseconds(1);    // enable pulse must be >450ns
	  _enable_pin = LOW;
	  delayMicroseconds(100);   // commands need > 37us to settle
	}

	template<uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
	void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::initMode() {
	  // Now we pull both RS and R/W low to begin commands
	  _rs_pin = LOW;
	  _enable_pin = LOW;
	  if (_rw_pin != 255) { 
	    _rw_pin = LOW;
	  }
 
   // this is according to the hitachi HD44780 datasheet
    // page 45 figure 23

    // Send function set command sequence
		// we haven't defined command() yet, but that's ok
		// we'll just write8bits() after setting _rs_pin = LOW

	  _rs_pin = LOW;
	  
    write8bits(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(4500);  // wait more than 4.1ms

    // second try
    write8bits(LCD_FUNCTIONSET | _displayfunction);
    delayMicroseconds(150);

    // third go
    write8bits(LCD_FUNCTIONSET | _displayfunction);
	}
	
	
	template<uint8_t rs, uint8_t rw, uint8_t enable,
		uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
		uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
	void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::write8bits(uint8_t value) {
		// 	  _data_pin0 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin1 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin2 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin3 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin4 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin5 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin6 = value & 0x01;
		// value >>= 1;
		// 	  _data_pin7 = value & 0x01;
		
		_data.set(value);
		
	  _enable_pin = LOW;
	  delayMicroseconds(1);    
	  _enable_pin = HIGH;
	  delayMicroseconds(1);    // enable pulse must be >450ns
	  _enable_pin = LOW;
	  delayMicroseconds(100);   // commands need > 37us to settle
	}	
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
	if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;
  _currline = 0;

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != 0) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
  // according to datasheet, we need at least 40ms after power rises above 2.7V
  // before sending commands. Arduino can turn on way befer 4.5V so we'll wait 50
  delayMicroseconds(50000); 
  
	/* The this-> is required. See: http://gcc.gnu.org/onlinedocs/gcc/Name-lookup.html */
	this->initMode();
	
  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);  

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::command(uint8_t value) {
  this->_rs_pin = LOW;

	// we never set it high!
	// _rw_pin = LOW;
  
	this->write8bits(value);
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
size_t LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::write(uint8_t value) {
  this->_rs_pin = HIGH;

	// we never set it high!
	// _rw_pin = LOW;
  
	this->write8bits(value);
	
	// assume success
	return 1;
}


/********** high level commands, for the user! */
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::setCursor(uint8_t col, uint8_t row)
{
  int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };
  if ( row >= _numlines ) {
    row = _numlines-1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

// Turn the display on/off (quickly)
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
template<uint8_t rs, uint8_t rw, uint8_t enable, uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3, uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7>
void LiquidCrystal<rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7>::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

} // namespace Motate
#endif
