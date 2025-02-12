#include "motor_sensor.h"

Sensor* sensorPtr = nullptr;  // Pointer to the current Sensor instance

Sensor::Sensor(int pin, int filterSize) : pin_(pin), filterSize_(filterSize), lastTime(0), timeBetweenSensors(0), rpmIndex(0) {
    if (filterSize_ > MAX_FILTER_SIZE) {  // Prevent filter size from exceeding the maximum limit
        filterSize_ = MAX_FILTER_SIZE;
    }

    for (int i = 0; i < filterSize_; i++) {
        rpmReadings[i] = 0;
    }

    sensorPtr = this;  // Store the pointer to this object
}

void Sensor::begin() {
    pinMode(pin_, INPUT);
    attachInterrupt(digitalPinToInterrupt(pin_), sensorISR, RISING);
}

void Sensor::sensorISR() {
    if (sensorPtr) {  // Ensure the pointer is valid
        unsigned long currentMicros = micros();
        if (sensorPtr->lastTime != 0) {
            sensorPtr->timeBetweenSensors = currentMicros - sensorPtr->lastTime;
        }
        sensorPtr->lastTime = currentMicros;
    }
}

unsigned long Sensor::getTimeBetweenSensors() {
    return timeBetweenSensors;
}

double Sensor::getFilteredRPM(double newRPM) {
    rpmReadings[rpmIndex] = newRPM;
    rpmIndex = (rpmIndex + 1) % filterSize_;  // Wrap around the filter size

    double sum = 0;
    for (int i = 0; i < filterSize_; i++) {
        sum += rpmReadings[i];
    }
    return sum / filterSize_;
}
