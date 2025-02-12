#ifndef TIMER_INTERRUPT_H
#define TIMER_INTERRUPT_H

#include <Arduino.h>

class TimerInterrupt {
public:
    TimerInterrupt();  // Constructor

    void begin(uint16_t compareMatch);  // Initialize the timer with the given compare match value

    static void attachInterruptHandler(void (*isr)());  // Attach a user-defined ISR
    static void handleInterrupt();  // Call the user-defined ISR in the main ISR

private:
    static void (*userISR)();  // Pointer to user ISR function
};

#endif  // TIMER_INTERRUPT_H
