#include "MotatePins.h"
#include "MotateTimers.h"

// This makes the Motate:: prefix unnecessary.
/****** Create file-global objects ******/

OutputPin<Motate::kLED1_PinNumber> led1_pin;
OutputPin<Motate::kLED2_PinNumber> led2_pin;
InputPin<Motate::kInput1_PinNumber> input_pin(kPullUp);


namespace Motate {
    MOTATE_PIN_INTERRUPT(kInput1_PinNumber) {
        led2_pin.toggle();
    }
}

/****** Optional setup() function ******/

void setup() {
    led1_pin = 1;
    led2_pin = 1;
    input_pin.setInterrupts(kPinInterruptOnChange|kPinInterruptPriorityLow);
}

/****** Main run loop() ******/

void loop() {

    led1_pin.toggle();

    delay(250);
}
