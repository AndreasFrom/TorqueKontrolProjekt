#include <Arduino.h>
#include "i2c_slave.h"

I2CSlave i2cSlave(0x08); 

void setup() {
    Serial.begin(115200);
    i2cSlave.begin();
    Serial.println("I2C Slave initialized");
}

void loop() {
    delay(1000);
    Serial.println("Looping...");
}
