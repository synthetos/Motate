# Motate

Motate: An Arduino-compatible board and software designed with movement in mind.

# Status

Currently the hardware is still in prototyping, but as the software components are closer to ready I'll release them here for review.

**The interface for this code is still in flux.** Before the Motate system is completed I'll freeze the interface.

Please share any suggestions for changes. Pull requests welcome as well.

# Usage

The contents of the Arduino Library folder can be placed the appropriate arduino libraries folder for your system, and the arduino IDE reloaded. The examples will show up in your File-> Examples submenu.

# Libraries

##Motate::Pin

At the top of the file, you need to pull in the header:

	#include <MotatePins.h>
	// save typing Motate::Pin
	using namespace Motate; 

### Creating `Pin`s
Creating a `Pin` (uppercase P) object is easy. You can use the pre-defined types `Pin`X or the template syntax `Pin<`X`>`.

	// create the led Pin object, setup as an output
	Pin13 ledPin = Output;
	// OR
	Pin<13> is acceptable as well.

Also as a convenience you can use the predefined objects `pin`N (lowercase p).

	pin13 = Output;
	pin13 = HIGH;
	if (pin13) {
		// something
	}

You generally cannot use `Pin<variableName>` since `variableName` cannot be changed after compiling.
	
However, `const uint8_t` variables can be used as the pin number, since they cannot be changed and must be set when they're defined.

The typedef `pin_number` was created as a more readable shortcut, and can be used for consolidating configuration of pins, for example.

	pin_number led_num = 13;
	// ... later ...
	Pin<led_num> ledPin = Output;

### Configuring `Pin`s

Setting the pin direction is easy. You assign one of the special values `Input`, `InputWithPullup`, and `Output` to the pin at any time.

	ledPin = Output;
	Pin3 inputPin = Input;
	Pin4 input2 = InputWithPullup;

The `InputWithPullup` is a convenience value for `Input` followed by setting the pin to `HIGH`.

### Setting `Pin`s

Setting the pin value is easy:

	ledPin = HIGH;
	// OR
	ledPin = 1;
	
	// Non-zero is HIGH, so you can use the result of bit-math:
	ledPin = input_value & test_mask;
	
	// Warning, negative is high too!
	int bad_value = -1;
	ledPin = badValue; // <- turns the led ON

**Note:** Setting the pin *value* is not supported while creating the pin.

This is **not** valid:

	Pin3 inputPin = LOW; // <-- NOT VALID

### Reading `Pin`s

Reading the pin is just as easy:

	Pin3 inputPin = InputWithPullup;
	if (inputPin) {
		// it's HIGH
	} else {
		// it's LOW
	}

### Warnings

A Pin is a pin, so every Pin that refers to the same physical pin will have the same attributes, since they are nothing but wrappers to manipulate the internal registers.

	// Set pin 5 to an input
	pin5 = Input; 
	
	// Create a new pin 5 and make in an output
	Pin<5> otherPin = Output;
	
`pin5` and `otherPin` are both Outputs at the end.

Also, `Pin` objects don't set their `Input`/`Output` state automatically. A `Pin` set to `Input` and set to `HIGH` will turn on the pull-up. A `Pin` set to `Output` then read will simply return the value it was last set to.

### Interaction with `digitalWrite()` and `digitalRead()`

`digitalRead()` and `digitalWrite()` are completely compatible with Pin changes. They work differently, but are both changing the same internal pin registers.

### Analog values

`Motate::Pin` currently does not **yet** support analog pin features. Of particular note is that an `analogWrite()` on a pin that is then set with a `Pin` object will not change the PWM ("analog") output value.

##Motate::LiquidCrystal

More info to come.

-Rob