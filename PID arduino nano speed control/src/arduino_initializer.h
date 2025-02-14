#ifndef ARDUINO_INITIALIZER_H
#define ARDUINO_INITIALIZER_H

#include "motor_sensor.h"
#include "timer_interrupt.h"

class ArduinoInitializer {
public:
    ArduinoInitializer(int sensorPin, int pwmPin, int enablePin, int dirPin,
                       MotorSensor* sensor, TimerInterrupt* timer);
    ~ArduinoInitializer();
    void begin();

private:
    int sensorPin_;
    int pwmPin_;
    int enablePin_;
    int dirPin_;
    MotorSensor* sensor_;
    TimerInterrupt* timer_;

    void initializeMotor();
    void initializeSensor();
    void initializeTimer();
};

#endif