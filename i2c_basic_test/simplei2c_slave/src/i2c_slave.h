#ifndef I2CSLAVE_H
#define I2CSLAVE_H

#include <Wire.h>
#include <Arduino.h>

class I2CSlave {
public:
    I2CSlave(uint8_t address);
    void begin();

private:
    uint8_t _address;
    uint8_t _receivedValue;
    uint8_t _receivedData[4]; 


    static void receiveEvent(int bytes);
    static void requestEvent();
    static I2CSlave* instance;
};

#endif
