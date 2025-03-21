#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <Arduino.h>
#include <queue>
#include <Wire.h>

// Define data structure for logging
struct dataBlock {
    // Timestamp:
    unsigned long timestamp;
    //  Setpoints:
    //uint8_t setpoint;
    //uint8_t setpoint_radius;
    // IMU data:
    float acc_x;
    float acc_y;
    float gyro_z;
    // Motor Unit data
};

class DataBuffer {
public:
    DataBuffer();
    void addData(const dataBlock& data); // Add data to buffer
    bool getData(dataBlock& data); // Get data from buffer

private:
    std::queue<dataBlock> _dataBuffer;
    const size_t _maxBufferSize = 500; // 5 seconds of data @ 100Hz
};

#endif