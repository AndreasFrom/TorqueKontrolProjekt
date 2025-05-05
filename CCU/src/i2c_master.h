#ifndef I2CMASTER_H
#define I2CMASTER_H

#define SEND_DATA_SERIAL false

#define SCALE_FACTOR_SPEED 71.0
#define SCALE_FACTOR_TORQUE 0.5
#define SCALE_FACTOR_RPM 0.25
#define SCALE_FACTOR_CURRENT 57.0

#include <Wire.h>
#include <Arduino.h>

// Command bytes
#define CMD_PARAM 0x10
#define CMD_SET 0x20

struct MUData
{
    double setpoint_recv;
    double value_recv;
    double current_recv;
};

class I2CMaster {
public:
    I2CMaster();
    void begin(); 
    bool sendParam(uint8_t slave_adress, uint8_t mode, float kp, float ki, float kd);
    bool sendSetpoint(uint8_t slave_adress, float setpoint);
    bool requestData(uint8_t slave_adress, MUData& data);

private:
    uint8_t _slaveAddress;
    uint8_t _mode = 0;
};

#endif
