#include "motor_sensor.h"

MotorSensor* MotorSensor::instance = nullptr;

MotorSensor::MotorSensor(int pin, int filterSize)
    : pin_(pin), filterSize_(filterSize), lastTime(0), timeBetweenSensors(0), rpmIndex(0) {
    if (filterSize_ > MAX_FILTER_SIZE) {
        filterSize_ = MAX_FILTER_SIZE;
    }

    for (int i = 0; i < filterSize_; i++) {
        rpmReadings[i] = 0;
    }

    instance = this; // Set the singleton instance
}

void MotorSensor::begin() {
    pinMode(pin_, INPUT);
}

void MotorSensor::MotorSensorISR() {
    if (instance) {
        unsigned long currentMicros = micros();
        if (instance->lastTime != 0) {
            instance->timeBetweenSensors = currentMicros - instance->lastTime;
            
            
            if (instance->timeBetweenSensors == 0) {
                instance->timeBetweenSensors = 1; // Smallest nonzero value
            }
        }
        instance->lastTime = currentMicros;
        instance->sensorTriggered = true;
    }
}


unsigned long MotorSensor::getTimeBetweenSensors() {
    return timeBetweenSensors;
}

double MotorSensor::getFilteredRPM(double newRPM) {
    rpmReadings[rpmIndex] = newRPM;
    rpmIndex = (rpmIndex + 1) % filterSize_;

    double sum = 0;
    for (int i = 0; i < filterSize_; i++) {
        sum += rpmReadings[i];
    }
    return sum / filterSize_;
}