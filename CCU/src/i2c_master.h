#ifndef I2CMASTER_H
#define I2CMASTER_H

#define SEND_DATA_SERIAL false

#include <Wire.h>
#include <Arduino.h>

// Command bytes
#define CMD_PARAM 0x10
#define CMD_SET 0x20

struct MUData
{
    float setpoint_recv;
    float value_recv;
    float current_recv;
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
};

#endif
