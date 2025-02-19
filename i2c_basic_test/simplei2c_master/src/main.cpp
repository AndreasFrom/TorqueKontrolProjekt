#include "i2c_master.h"

I2CMaster i2cMaster; // Define the master instance globally

void setup() {
    i2cMaster.begin();
}

void loop() {
    delay(1000); // Send data every second

    i2cMaster.sendData(0x42, 100, 10, 5, 1); 
    delay(50);
    i2cMaster.requestData(0x42);
}
