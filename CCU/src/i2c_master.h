#ifndef I2CMASTER_H
#define I2CMASTER_H

#include <Wire.h>
#include <Arduino.h>

// Command bytes
#define CMD_PARAM 0x10
#define CMD_SET 0x20

class I2CMaster {
public:
    I2CMaster();
    void begin(); 
    bool sendParam(uint8_t slave_adress, uint8_t mode, uint8_t kp, uint8_t ki, uint8_t kd);
    bool sendSetpoint(uint8_t slave_adress, uint8_t setpoint);
    bool requestData(uint8_t slave_adress);

private:
    uint8_t _slaveAddress;
};

#endif
