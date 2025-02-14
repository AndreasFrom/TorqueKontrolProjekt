#ifndef TIMER_INTERRUPT_CPP
#define TIMER_INTERRUPT_CPP

#include "timer_interrupt.h"

void (*TimerInterrupt::userISR)() = nullptr;

TimerInterrupt::TimerInterrupt() {}

void TimerInterrupt::begin(double sampleTime) {
    // Calculate compareMatch value based on sampleTime
    // Timer1 configuration for 16MHz clock
    // Prescaler = 1024
    // Timer frequency = 16,000,000 / 1024 = 15,625 Hz
    // Time per tick = 1 / 15,625 = 64 microseconds
    // compareMatch = (sampleTime * 1,000,000) / 64 (convert sampleTime to microseconds)
    uint16_t compareMatch = static_cast<uint16_t>((sampleTime * 1e6) / 64);

    // Configure Timer1
    TCCR1A = 0;
    TCCR1B = 0;
    TCNT1 = 0;
    OCR1A = compareMatch;
    TCCR1B |= (1 << WGM12); // CTC mode
    TCCR1B |= (1 << CS12) | (1 << CS10); // Prescaler = 1024
    TIMSK1 |= (1 << OCIE1A); // Enable timer compare interrupt
}

void TimerInterrupt::attachInterruptHandler(void (*isr)()) {
    userISR = isr;
}

void TimerInterrupt::handleInterrupt() {
    if (userISR) {
        userISR();
    }
}

#endif  // TIMER_INTERRUPT_CPP