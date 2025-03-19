#include "motor_sensor.h"

MotorSensor* MotorSensor::instance = nullptr;

MotorSensor::MotorSensor(int pinSensor, int pinCurrent, int filterSize)
    : _pinSensor(pinSensor), _pinCurrent(pinCurrent), _filterSize(filterSize), lastTime(0), timeBetweenSensors(0), rpmIndex(0) {
    if (_filterSize > MAX_FILTER_SIZE) {
        _filterSize = MAX_FILTER_SIZE;
    }

    for (int i = 0; i < _filterSize; i++) {
        rpmReadings[i] = 0;
    }

    instance = this; // Set the singleton instance
}

void MotorSensor::begin() {
    pinMode(_pinSensor, INPUT);
    pinMode(_pinCurrent, INPUT);
}

void MotorSensor::MotorSensorISR() {
    if (instance) {
        unsigned long currentMicros = micros();
        if (instance->lastTime != 0) {
            instance->timeBetweenSensors = currentMicros - instance->lastTime;
        }
        instance->lastTime = currentMicros;
        instance->sensorTriggered = true;
    }
}

unsigned long MotorSensor::getTimeBetweenSensors() {
    return timeBetweenSensors;
}

double MotorSensor::getCurrentReading()
{
    return analogRead(_pinCurrent); 
}

double MotorSensor::getFilteredRPM(double newRPM) {
    rpmReadings[rpmIndex] = newRPM;
    rpmIndex = (rpmIndex + 1) % _filterSize;

    double sum = 0;
    for (int i = 0; i < _filterSize; i++) {
        sum += rpmReadings[i];
    }
    return sum / _filterSize;
}