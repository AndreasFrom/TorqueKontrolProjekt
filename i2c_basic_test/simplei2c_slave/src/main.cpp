#include <Arduino.h>
#include "i2c_slave.h"

I2CSlave i2cSlave(0x42); 

void setup() {
    Serial.begin(115200);
    i2cSlave.begin();
}

void loop() {
    delay(1000);
}
