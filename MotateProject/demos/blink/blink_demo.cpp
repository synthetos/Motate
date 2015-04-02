#include "MotatePins.h"
#include "MotateTimers.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

OutputPin<kLED1_PinNumber> led1_pin;
OutputPin<kLED2_PinNumber> led2_pin;
InputPin<kGPIO_XMin> input_pin;


namespace Motate {
MOTATE_PIN_INTERRUPT(kGPIO_XMin) {
    led2_pin.toggle();
}
}

/****** Optional setup() function ******/

void setup() {
    led1_pin = 1;
}

/****** Main run loop() ******/

void loop() {

    led1_pin.toggle();

    delay(250);
}
