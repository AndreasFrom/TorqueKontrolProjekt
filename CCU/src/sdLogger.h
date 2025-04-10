#ifndef SDLOGGER_H
#define SDLOGGER_H

#include <Arduino.h>
#include <SD.h>
#include "i2c_master.h"

// Define data structure for logging
struct dataBlock {
    unsigned long timestamp;    // Timestamp

    uint8_t mode;               // Control mode
    float setpoint;           // Setpoints:
    float setpoint_radius;    // Setpoint radius

    float acc_x;                // IMU Acceleration x
    float acc_y;                // IMU Acceleration y
    float gyro_z;               // IMU Gyro z

    float actual_velocity;      // Actual velocity

    float Kp;                   // PID Kp
    float Ki;                   // PID Ki
    float Kd;                   // PID Kd
    
    MUData MU0;                 // Data from MU
    MUData MU1;                 // Data from MU
    MUData MU2;                 // Data from MU
    MUData MU3;                 // Data from MU

    float error_yaw;
    float error_velocity;
    float updated_yaw;
    float updated_velocity;

    float omega_yaw;
    float omega_move;
};

class SDLogger {
public:
    SDLogger(void);
    void init(const int spi_cs, const char* filename);
    void addData(const dataBlock& data);
    void close();
private:
    File _dataFile;
    const char* _filename;
    bool _fileOpen = false;
    String _dataHeader = ("timestamp, mode, setpoint, setpoint_radius, acc_x, acc_y, gyro_z, actual_velocity, Kp, Ki, Kd, MU0setpoint, MU0value, MU0current, MU1setpoint, MU1value, MU1current, MU2setpoint, MU2value, MU2current, MU3setpoint, MU3value, MU3current, error_yaw, error_velocity, updated_yaw, updated_velocity, omega_yaw, omega_move");
};

#endif