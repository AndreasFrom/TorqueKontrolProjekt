#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

class Sensor {
public:
    Sensor(int pin, int filterSize);  // Constructor now accepts filter size
    void begin();
    static void sensorISR();  // Static function for ISR

    unsigned long getTimeBetweenSensors();
    double getFilteredRPM(double newRPM);  // Make this method public

private:
    int pin_;
    int filterSize_;  // Store the filter size
    volatile unsigned long lastTime;
    volatile unsigned long timeBetweenSensors;
    static const int MAX_FILTER_SIZE = 50;  // Max size of the filter (you can adjust this as needed)
    double rpmReadings[MAX_FILTER_SIZE];
    int rpmIndex;

    double getFilteredRPM(); // Keep a private version that doesn't take an argument
};

#endif  // SENSOR_H
