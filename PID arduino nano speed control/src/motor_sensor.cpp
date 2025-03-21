#include "motor_sensor.h"

MotorSensor* MotorSensor::instance = nullptr;

MotorSensor::MotorSensor(int RPMpin, int filterSizeRPM, int currentSensePin, int filterSizeCurrent)
    : RPMpin_(RPMpin), filterSizeRPM_(filterSizeRPM), currentSensePin_(currentSensePin), filterSizeCurrent_(filterSizeCurrent) , lastTime(0), timeBetweenSensors(0), rpmIndex(0) {
    if (filterSizeRPM_ > MAX_FILTER_SIZE) {
        filterSizeRPM_ = MAX_FILTER_SIZE;
    }
    if (filterSizeCurrent_ > MAX_FILTER_SIZE) {
        filterSizeCurrent_ = MAX_FILTER_SIZE;
    }

    for (int i = 0; i < filterSizeRPM_; i++) {
        rpmReadings[i] = 0;
    }
    for (int i = 0; i < filterSizeCurrent_; i++) {
        currentReadings[i] = 0;
    }

    instance = this; // Set the singleton instance
}

void MotorSensor::begin() {
    pinMode(RPMpin_, INPUT);
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
    rpmIndex = (rpmIndex + 1) % filterSizeRPM_;

    double sum = 0;
    for (int i = 0; i < filterSizeRPM_; i++) {
        sum += rpmReadings[i];
    }
    return sum / filterSizeRPM_;
}

double MotorSensor::getFilteredCurrent(double newCurrent){
    currentReadings[currentIndex] = newCurrent;
    currentIndex = (currentIndex + 1) % filterSizeCurrent_;

    double sum = 0;
    for (int i = 0; i < filterSizeCurrent_; i++) {
        sum += currentReadings[i];
    }
    return sum / filterSizeCurrent_;
}

double MotorSensor::getMotorCurrent() {
    //Sensor resolution: 1.1V/A
    int sensorVal = analogRead(currentSensePin_);
    current_ = (((sensorVal * 5.0) / 1023.0) / 1.1);
    //Serial.print("Current: "); 
    //Serial.println(current_);
    return current_;
}
