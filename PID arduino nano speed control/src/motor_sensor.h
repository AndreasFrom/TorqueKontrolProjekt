#ifndef MOTOR_SENSOR_H
#define MOTOR_SENSOR_H

#include <Arduino.h>

#define MAX_FILTER_SIZE 10

class MotorSensor {
public:
    MotorSensor(int RPMpin, int filterSizeRPM, int currentSensePin, int filterSizeCurrent);
    void begin();
    static void MotorSensorISR();
    unsigned long getTimeBetweenSensors();
    double getFilteredRPM(double newRPM);
    double getFilteredCurrent(double newCurrent);
    bool isSensorTriggered() { return sensorTriggered; }
    void resetSensorTriggered() { sensorTriggered = false; }
    double getMotorCurrent();

private:
    int RPMpin_;
    int filterSizeRPM_;
    int currentSensePin_;
    int filterSizeCurrent_;
    double current_;
    volatile unsigned long lastTime = 1;
    volatile unsigned long timeBetweenSensors = 1;
    double rpmReadings[MAX_FILTER_SIZE];
    int rpmIndex;
    double currentReadings[MAX_FILTER_SIZE];
    int currentIndex;
    volatile bool sensorTriggered = false;

    static MotorSensor* instance; // Singleton instance for ISR
};

#endif