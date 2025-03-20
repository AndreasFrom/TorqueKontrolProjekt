#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <Arduino.h>
#include <SD.h>

// Define data structure for logging
struct dataBlock {
    // Timestamp:
    unsigned long timestamp;
    // Setpoints:
    //uint8_t setpoint;
    //uint8_t setpoint_radius;
    // IMU data:
    float acc_x;
    float acc_y;
    float gyro_z;
    // Motor Unit data
};

class SDLogger {
public:
    SDLogger(const int spi_cs);
    void init(const char* filename);
    void addData(const dataBlock& data);
    bool getData(dataBlock& data);
private:
    File _dataFile;
    const char* _filename;
    bool _fileOpenForRead = false;
};

#endif