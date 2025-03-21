#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <Arduino.h>
#include <SD.h>
#include "i2c_master.h"

// Define data structure for logging
struct dataBlock {
    unsigned long timestamp;    // Timestamp

    uint8_t mode;               // Control mode
    uint8_t setpoint;           // Setpoints:
    uint8_t setpoint_radius;    // Setpoint radius

    float acc_x;                // IMU Acceleration x
    float acc_y;                // IMU Acceleration y
    float gyro_z;               // IMU Gyro z

    float Kp;                   // PID Kp
    float Ki;                   // PID Ki
    float Kd;                   // PID Kd
    
    MUData MU0;                 // Data from MU
    MUData MU1;                 // Data from MU
    MUData MU2;                 // Data from MU
    MUData MU3;                 // Data from MU
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
    String _dataHeader = 
        ("timestamp, mode, setpoint, setpoint_radius, acc_x, acc_y, gyro_z, Kp, Ki, Kd, MU0setpoint, MU0value, MU0current, MU1setpoint, MU1value, MU1current, MU2setpoint, MU2value, MU2current, MU3setpoint, MU3value, MU3current");
};

#endif