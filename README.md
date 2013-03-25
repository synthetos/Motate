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
Creating a `Pin` (uppercase P) object is easy. You can use the pre-defined types `Pin`_X_ or the template syntax `Pin<`_X_`>`. Specify in the constructor if the pin will be used as an input (`kInput`) or an output (`kOutput`). If you don't provide a value, the pine will be setup as an output.

```c++
	// create the led Pin object, setup as an output
	Pin13 ledPin(kOutput);
	// OR
	Pin<13> ledPin(kOutput); // is acceptable as well.
```

Also as a convenience you can use the predefined objects `pin`_N_ (lowercase p).

```c++
	pin13.setMode(kOutput);
	pin13 = HIGH;
	if (pin13) {
		// something
	}
```

You generally cannot use `Pin<variableName>` since `variableName` cannot be changed after compiling.
	
However, `const ` variables can be used as the pin number, since they cannot be changed and must be set when they're defined.

The typedef `pin_number` was created for this purpose, and can be used for consolidating configuration of pins, for example.

```c++
	pin_number led_num = 13;
	// ... later ...
	Pin<led_num> ledPin(kOutput);
```

### Configuring `Pin`s

You can either define a pin as in `InputPin<>` or an `OutputPin<>`, which provides a minor performance gain and also provides some compile-time checks to prevent you from use a pin the wrong way. 

For normal `Pin` objects, you pass `kInput` or `kOutput` to the constructor, and you set the mode with `Pin<>.setMode(`_mode_`)` during runtime. ** Note: The `Pin`_X_ predefined types and the `pin`_X_ predefined objects are normal, bidirectional `Pin`s, and default to being output pins. **

```c++
	ledPin.setMode(kOutput);
	Pin3 inputPin(kInput);
```

You can also set addition options for pins, depending on the hardware options available. All platforms support `kNormal` (no options) and `kPullUp` (to turn on the internal pull-up resistor).

```c++
	// For normal Pins, pass the mode then the option parameter
	Pin<13> buttonPin1(kInput, kPullUp);
	
	// For InputPins or OutputPins, pass the option as the only parameter
	InputPin<13> buttonPin2(kPullUp);
	
	// Set the options at any time:
	buttonPin2.setOptions(kNormal); // disable the pullup
```

### Setting `Pin`s

Setting the pin value is easy:

```c++
	// Use the Arduino defined constants HIGH or LOW.
	ledPin = HIGH;
	// OR, use 0 or 1
	ledPin = 1;
	
	// Anything that not zero is HIGH.
	// This allows such things as using the result of bit-math:
	ledPin = input_value & test_mask;
	
	// Warning, negative is HIGH too!
	int bad_value = -1; // -1 is not zero!
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
	InputPin<3> inputPin(kPullup);
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
	pin5.setMode(kInput);
	
	// Create a new pin 5 and make in an output
	Pin<5> otherPin(kOutput);
```

This leaves `pin5` and `otherPin` both as outputs.

_On some architectures:_ If all of the pins on the same physical port are outputs, then normal `Pin` objects will **read** as an undefined value, since the input clock of that port is disabled. To prevent this, either use an `OutputPin` or read the value with `getOutputValue()`.

Also, `Pin` objects don't set their input/output state automatically. A `Pin` set to `kInput` and then assigned an output value will have different results on different architectures. `InputPin` objects will not allow setting the value for this reason.

### Interaction with Arduino `digitalWrite()` and `digitalRead()`

`digitalRead()` and `digitalWrite()` are completely compatible with `Pin` objects. They work differently, but are both changing the same internal pin registers.

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