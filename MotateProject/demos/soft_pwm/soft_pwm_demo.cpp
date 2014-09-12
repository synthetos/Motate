#include "MotatePins.h"

// This makes the Motate:: prefix unnecessary.
using namespace Motate;

/****** Create file-global objects ******/

OutputPin<kLED1_PinNumber> led1_pin;

timer_number kBlinkTimerNumber = 0;
timer_number kBlinkTimerChNumber = 0;
TimerChannel<kBlinkTimerNumber, kBlinkTimerChNumber>
                blinkTimer(kTimerUpToTop, /*Hz: */ 10000);

float duty_cycle = 0.0;

/****** Optional setup() function ******/

void setup() {
    led1_pin = 1;

    blinkTimer.setInterrupts(kInterruptOnMatch|kInterruptOnOverflow);
    blinkTimer.setDutyCycle(duty_cycle);
    blinkTimer.start();
}

/****** Main run loop() ******/

void loop() {
    duty_cycle += 0.01;
    if (duty_cycle > 1.0)
        duty_cycle = 0.0;

    blinkTimer.setDutyCycle(duty_cycle);

    delay(100);
}

/****** timer interrupt handler ******/

MOTATE_TIMER_INTERRUPT(kBlinkTimerNumber) {
    int16_t interrupted_channel;
    TimerChannelInterruptOptions interrupt_cause = getInterruptCause(interrupted_channel);
    if (interrupt_cause == kInterruptOnMatch) {
        led1_pin = 1;
    } else if (interrupt_cause == kInterruptOnOverflow) {
        led1_pin = 0;
    }
}