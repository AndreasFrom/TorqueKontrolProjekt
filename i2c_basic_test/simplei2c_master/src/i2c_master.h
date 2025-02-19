#ifndef I2CMASTER_H
#define I2CMASTER_H

#include <Wire.h>
#include <Arduino.h>

class I2CMaster {
public:
    I2CMaster();
    void begin();
    void sendData(uint8_t slave_adress,uint8_t setpoint, uint8_t kp, uint8_t ki, uint8_t kd);
    void requestData(uint8_t slave_adress);

private:
    uint8_t _slaveAddress;
};

#endif
