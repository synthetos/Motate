#include "MotatePins.h"
#include "MotateTimers.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

OutputPin<kLED1_PinNumber> led1_pin;

/****** Optional setup() function ******/

void setup() {
    led1_pin = 1;
}

/****** Main run loop() ******/

void loop() {

    led1_pin.toggle();

    delay(250);
}
