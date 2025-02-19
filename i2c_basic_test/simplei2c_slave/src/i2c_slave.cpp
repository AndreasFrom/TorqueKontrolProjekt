#include "i2c_slave.h"

I2CSlave* I2CSlave::instance = nullptr; // Initialize instance pointer

I2CSlave::I2CSlave(uint8_t address) : _address(address), _receivedValue(0) {
    instance = this; // Store instance reference
}

void I2CSlave::begin() {
    Wire.begin(_address);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent);
}

void I2CSlave::receiveEvent(int bytes) {
    if (instance) {
        Serial.print("Received: ");
        int index = 0;
        while (Wire.available() && index < 4) { // Read all bytes
            instance->_receivedData[index] = Wire.read();
            Serial.print(instance->_receivedData[index]); // Print received value
            Serial.print(" ");
            index++;
        }
        Serial.println(); 
    }
}


void I2CSlave::requestEvent() {
    if (instance) {
        for (int i = 0; i < 4; i++) {
            Wire.write((instance->_receivedData[i]) + 5);
        }
    }
}

