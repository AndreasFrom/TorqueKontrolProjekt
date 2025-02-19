#include "i2c_master.h"

I2CMaster::I2CMaster() {}

void I2CMaster::begin() {
    Wire.begin();
    Serial.begin(115200);
    Serial.println("I2C Master Ready!");
}

void I2CMaster::sendData(uint8_t slave_adress, uint8_t setpoint, uint8_t kp, uint8_t ki, uint8_t kd) {
    Wire.beginTransmission(slave_adress);
    Wire.write(setpoint);
    Wire.write(kp);
    Wire.write(ki);
    Wire.write(kd);
    Wire.endTransmission();

    Serial.println("Data sent!");
}

void I2CMaster::requestData(uint8_t slave_adress) {
    Wire.requestFrom(slave_adress, 4); // Request 4 bytes

    if (Wire.available() >= 4) {
        uint8_t setpoint_recv = Wire.read();
        float kp_recv = Wire.read();
        float ki_recv = Wire.read();
        float kd_recv = Wire.read();

        Serial.print("Received back: Setpoint = ");
        Serial.print(setpoint_recv);
        Serial.print(", Kp = ");
        Serial.print(kp_recv);
        Serial.print(", Ki = ");
        Serial.print(ki_recv);
        Serial.print(", Kd = ");
        Serial.println(kd_recv);
    } else {
        Serial.println("Error: Did not receive expected data from slave!");
    }
}
