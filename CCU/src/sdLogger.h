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
    SDLogger(void);
    void init(const int spi_cs, const char* filename);
    void addData(const dataBlock& data);
    bool getData(dataBlock& data);
    void close();
private:
    File _dataFile;
    const char* _filename;
    bool _fileOpen = false;
    String _dataHeader = ("timestamp, acc_x, acc_y, gyro_z");
};

#endif