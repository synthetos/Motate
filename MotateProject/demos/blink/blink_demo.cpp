#include "MotatePins.h"
#include "MotateTimers.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

OutputPin<kLED1_PinNumber> led1_pin;
OutputPin<kLED2_PinNumber> led2_pin;
InputPin<kGPIO_XMin> input_pin(kPullUp);


namespace Motate {
    MOTATE_PIN_INTERRUPT(kGPIO_XMin) {
        led2_pin.toggle();
    }
}

/****** Optional setup() function ******/

void setup() {
    led1_pin = 1;
    led2_pin = 1;
//    input_pin.setInterrupts();
}

/****** Main run loop() ******/

void loop() {

    led1_pin.toggle();

    delay(250);
}
