#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H

#include <Arduino.h>

class TimerInterrupt {
public:
    TimerInterrupt();
    void begin(double sampleTime); // Accepts sampleTime in seconds
    void attachInterruptHandler(void (*isr)());
    static void handleInterrupt();

private:
    static void (*userISR)();
};

#endif