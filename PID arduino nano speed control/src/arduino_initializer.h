#ifndef ARDUINO_INITIALIZER_H
#define ARDUINO_INITIALIZER_H

#include <Arduino.h>
#include "motor_sensor.h"
#include "timer_interrupt.h"

class ArduinoInitializer {
public:
    // Constructor with dependency injection
    ArduinoInitializer(int sensorPin, int pwmPin, int enablePin, int dirPin,
                       Sensor* sensor, TimerInterrupt* timer);

    // Destructor to clean up dynamically allocated memory
    ~ArduinoInitializer();

    // Initialize all components
    void begin();

private:
    // Initialize the motor control pins
    void initializeMotor();

    // Initialize the sensor and attach the ISR
    void initializeSensor();

    // Initialize the timer interrupt
    void initializeTimer();

    // Pin configurations
    const int sensorPin_;
    const int pwmPin_;
    const int enablePin_;
    const int dirPin_;

    // Dependencies
    Sensor* sensor_;
    TimerInterrupt* timer_;
};

#endif // ARDUINO_INITIALIZER_H