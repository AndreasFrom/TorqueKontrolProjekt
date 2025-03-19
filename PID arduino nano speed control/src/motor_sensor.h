#ifndef MOTOR_SENSOR_H
#define MOTOR_SENSOR_H

#include <Arduino.h>

#define MAX_FILTER_SIZE 10

class MotorSensor {
public:
    MotorSensor(int pinSensor, int pinCurrent, int filterSize);
    void begin();
    static void MotorSensorISR();
    unsigned long getTimeBetweenSensors();
    double getCurrentReading();
    double getFilteredRPM(double newRPM);
    bool isSensorTriggered() { return sensorTriggered; }
    void resetSensorTriggered() { sensorTriggered = false; }

private:
    int _pinSensor;
    int _pinCurrent;
    int _filterSize;
    volatile unsigned long lastTime = 1;
    volatile unsigned long timeBetweenSensors = 1;
    double rpmReadings[MAX_FILTER_SIZE];
    int rpmIndex;
    volatile bool sensorTriggered = false;

    static MotorSensor* instance; // Singleton instance for ISR
};

#endif