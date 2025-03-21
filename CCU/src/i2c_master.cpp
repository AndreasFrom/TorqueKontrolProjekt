#include "i2c_master.h"

I2CMaster::I2CMaster() {}

void I2CMaster::begin() {
    Wire.begin();
    Serial.println("I2C Master Ready!");
}

bool I2CMaster::sendParam(uint8_t slave_adress, uint8_t mode, uint8_t kp, uint8_t ki, uint8_t kd) {
    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_PARAM);
    Wire.write(mode);
    Wire.write(kp);
    Wire.write(ki);
    Wire.write(kd);
    Wire.endTransmission();

    if(SEND_DATA_SERIAL){
        Serial.println("Parameters sent!");
    }
    return true;
}

bool I2CMaster::sendSetpoint(uint8_t slave_adress, uint8_t setpoint) {
    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_SET);
    Wire.write(setpoint);
    
    if(SEND_DATA_SERIAL){
        Serial.println("Setpoint sent!");
    }
    return true;
}

bool I2CMaster::requestData(uint8_t slave_adress, MUData& data) {
    Wire.requestFrom(slave_adress, 3); // Request 3 bytes

    if (Wire.available() >= 3) {
        data.setpoint_recv = Wire.read();
        data.value_recv = Wire.read();
        data.current_recv = Wire.read();

        if(SEND_DATA_SERIAL){
            Serial.print("Received back: Setpoint = ");
            Serial.print(data.setpoint_recv);
            Serial.print(", Measured value = ");
            Serial.print(data.value_recv);
            Serial.print(", Current = ");
            Serial.println(data.current_recv);
        }
    } else {
        Serial.println("Error: Did not receive expected data from slave!");
    }
}
