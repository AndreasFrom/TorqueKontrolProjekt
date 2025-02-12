#include "timer_interrupt.h"

// Initialize the static function pointer to nullptr
void (*TimerInterrupt::userISR)() = nullptr;

// Constructor
TimerInterrupt::TimerInterrupt() {}

// Begin Timer1 with the specified compare match value
void TimerInterrupt::begin(uint16_t compareMatch) {
    // Clear Timer1 control registers
    TCCR1A = 0;  
    TCCR1B = 0;  
    TCNT1  = 0;  // Initialize counter to 0
    
    OCR1A  = compareMatch;  // Set the compare match register
    
    // Set CTC mode (Clear Timer on Compare Match)
    TCCR1B |= (1 << WGM12);
    
    // Set prescaler to 1024 (adjust if needed)
    TCCR1B |= (1 << CS12) | (1 << CS10);
    
    // Enable Timer1 compare match interrupt
    TIMSK1 |= (1 << OCIE1A);
}

// Attach a user-defined ISR to the timer interrupt
void TimerInterrupt::attachInterruptHandler(void (*isr)()) {
    userISR = isr;
}

// Handle the interrupt by calling the user-defined ISR
void TimerInterrupt::handleInterrupt() {
    if (userISR) {
        userISR();  // Execute the user-defined ISR function
    }
}
