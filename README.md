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

```c++
	#include <MotatePins.h>
	// save typing Motate::Pin
	using namespace Motate; 
```

### Creating `Pin`s
Creating a `Pin` (uppercase P) object is easy. You can use the pre-defined types `Pin`X or the template syntax `Pin<`X`>`.

```c++
	// create the led Pin object, setup as an output
	Pin13 ledPin = Output;
	// OR
	Pin<13> ledPin = Output; // is acceptable as well.
```

Also as a convenience you can use the predefined objects `pin`N (lowercase p).

```c++
	pin13 = Output;
	pin13 = HIGH;
	if (pin13) {
		// something
	}
```

You generally cannot use `Pin<variableName>` since `variableName` cannot be changed after compiling.
	
However, `const uint8_t` variables can be used as the pin number, since they cannot be changed and must be set when they're defined.

The typedef `pin_number` was created as a more readable shortcut, and can be used for consolidating configuration of pins, for example.

```c++
	pin_number led_num = 13;
	// ... later ...
	Pin<led_num> ledPin = Output;
```

### Configuring `Pin`s

Setting the pin direction is easy. You assign one of the special values `Input`, `InputWithPullup`, and `Output` to the pin at any time.

```c++
	ledPin = Output;
	Pin3 inputPin = Input;
	Pin4 input2 = InputWithPullup;
```

The `InputWithPullup` is a convenience value for `Input` followed by setting the pin to `HIGH`.

### Setting `Pin`s

Setting the pin value is easy:

```c++
	ledPin = HIGH;
	// OR
	ledPin = 1;
	
	// Non-zero is HIGH, so you can use the result of bit-math:
	ledPin = input_value & test_mask;
	
	// Warning, negative is high too!
	int bad_value = -1;
	ledPin = badValue; // <- turns the led ON
```

**Note:** Setting the pin *value* is not supported while creating the pin.

This is **not** valid:

```c++
	Pin3 inputPin = LOW; // <-- NOT VALID
```

### Reading `Pin`s

Reading the pin is just as easy:

```c++
	Pin3 inputPin = InputWithPullup;
	if (inputPin) {
		// it's HIGH
	} else {
		// it's LOW
	}
```

### Warnings

A Pin is a pin, so every Pin that refers to the same physical pin will have the same attributes, since they are nothing but wrappers to manipulate the internal registers.

```c++
	// Set pin 5 to an input
	pin5 = Input; 
	
	// Create a new pin 5 and make in an output
	Pin<5> otherPin = Output;
	
```
`pin5` and `otherPin` are both Outputs at the end.

Also, `Pin` objects don't set their `Input`/`Output` state automatically. A `Pin` set to `Input` and set to `HIGH` will turn on the pull-up. A `Pin` set to `Output` then read will simply return the value it was last set to.

### Interaction with `digitalWrite()` and `digitalRead()`

`digitalRead()` and `digitalWrite()` are completely compatible with Pin changes. They work differently, but are both changing the same internal pin registers.

### Analog values

`Motate::Pin` currently does not **yet** support analog pin features. Of particular note is that an `analogWrite()` on a pin that is then set with a `Pin` object will not change the PWM ("analog") output value.

##Motate::LiquidCrystal

Use the exact same as the built-in LiquidCrystal, except the header and how the object is created. All of the examples have been converted and are provided.

```c++
#include <MotateLCD.h>

using namespace Motate;

// initialize the library with the numbers of the interface pins
LiquidCrystal</*RS*/12, /*R/W*/-1, /*Enable*/11, /*D4*/5, /*D5*/4, /*D6*/3, /*D7*/2> lcd;
```

Note that the `R/W` pin *position* is mandatory, but the pin itself isn't. If you don't use that pin, then simply use the value `-1`.

##Motate::TWI

This provides I²C/TWI/Two-Wire functionality like the Arduino built-in `Wire` library, but with a simplified interface, more functionality, and with 10-20% smaller binary sketch size.

(More documentation coming soon.)

**Features:**
* Non-blocking - All features will return immediately if possible without losing data. This means you can go do something else (such as check sensors) *while* the TWI hardware is sending or receiving.
* Multimaster-aware: Slave and Master code each has it's own TX and RX buffer. You can configure as both a Slave and a Master. Incoming communications as a Slave that interrupt Master communications will not lose data - the Master communications will resume as soon as the bus is free again.
* Smaller code size.

**Writing example:**
```c++
const uint8_t PCA9685_MODE1 = 0x00;
const uint8_t PCA9685_AUTOINCREMENT = 1<<5;

Master.open(slave_i2caddr, Writing); // Writing is a constant
Master.write(PCA9685_MODE1);
Master.write(PCA9685_AUTOINCREMENT, /*last byte = */true); // MUST indicate the last byte
Master.close(); // Indicates that a STOP is requested
```

The last `Master.close()` is optional if the last write is followed by a `Master.open()`, in which case a RESTART will be issued.

**Reading example:**
```c++
// First we tell the devide what register we want to read from
Master.open(_i2caddr, Writing);
Master.write(PCA9685_MODE1, /*last byte = */true);
// Then we RESTART and read 1 byte
Master.open(_i2caddr, Reading);
Master.setExpecting(1); // MUST indicate how many bytes to read
do {} while (!Master.available()); // wait for them to become available
uint8_t oldmode = Master.read();
Master.close(); // Indicates that a STOP is requested
```


I'll post updates over on the [+Motate](http://plus.google.com/114859135931792455633?prsrc=3) Google+ page, and on my blog: [Tinkerin.gs](http://tinkerin.gs).

-Rob