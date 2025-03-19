#include "motor_sensor.h"

MotorSensor* MotorSensor::instance = nullptr;

MotorSensor::MotorSensor(int pin, int filterSize, int currentSensePin)
    : pin_(pin), filterSize_(filterSize), currentSensePin_(currentSensePin), lastTime(0), timeBetweenSensors(0), rpmIndex(0) {
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

double MotorSensor::getMotorCurrent() {
    //Sensor resolution: 1.1V/A
    int sensorVal = analogRead(currentSensePin_);
    current_ = (((sensorVal * 5.0) / 1023.0) / 1.1);
    Serial.print("Current: "); 
    Serial.println(current_);
    return current_;
}
