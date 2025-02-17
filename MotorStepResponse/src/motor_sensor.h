#ifndef MOTOR_SENSOR_H
#define MOTOR_SENSOR_H

#include <Arduino.h>

#define MAX_FILTER_SIZE 10

class MotorSensor {
public:
    MotorSensor(int pin, int filterSize);
    void begin();
    static void MotorSensorISR();
    unsigned long getTimeBetweenSensors();
    double getFilteredRPM(double newRPM);
    bool isSensorTriggered() { return sensorTriggered; }
    void resetSensorTriggered() { sensorTriggered = false; }

private:
    int pin_;
    int filterSize_;
    volatile unsigned long lastTime = 100000;
    volatile unsigned long timeBetweenSensors = 100000;
    double rpmReadings[MAX_FILTER_SIZE] = {0};
    int rpmIndex;
    volatile bool sensorTriggered = false;

    static MotorSensor* instance; // Singleton instance for ISR
};

#endif