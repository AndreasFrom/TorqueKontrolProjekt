#ifndef DATABUFFER_H
#define DATABUFFER_H

#include <Arduino.h>
#include <queue>
#include <Wire.h>

struct dataBlock {
    // Timestamp
    //uint8_t setpoint;
    //uint8_t setpoint_radius;
    int16_t acc_x;
    int16_t acc_y;
    int16_t gyro_z;
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