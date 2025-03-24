#include "i2c_master.h"

I2CMaster::I2CMaster() {}

void I2CMaster::begin() {
    Wire.begin();
    Serial.println("I2C Master Ready!");
}

bool I2CMaster::sendParam(uint8_t slave_adress, uint8_t mode, float kp, float ki, float kd) {
    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_PARAM);
    Wire.write(mode);
    Wire.write(byte(kp*10));
    Wire.write(byte(ki*10));
    Wire.write(byte(kd*10));
    Wire.endTransmission();

    if(SEND_DATA_SERIAL){
        Serial.println("Parameters sent!");
    }
    return true;
}

bool I2CMaster::sendSetpoint(uint8_t slave_adress, float setpoint) {
    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_SET);
    Wire.write(byte(setpoint*10));
    Wire.endTransmission();
    
    if(SEND_DATA_SERIAL){
        Serial.println("Setpoint sent!");
    }
    return true;
}

bool I2CMaster::requestData(uint8_t slave_adress, MUData& data) {
    Wire.requestFrom(slave_adress, 3); // Request 3 bytes

    if (Wire.available() >= 3) {
        uint8_t raw_setpoint = Wire.read();
        uint8_t raw_value = Wire.read();
        uint8_t raw_current = Wire.read();

        data.setpoint_recv = static_cast<float>(raw_setpoint) / 10.0;
        data.value_recv = static_cast<float>(raw_value) / 10.0;
        data.current_recv = static_cast<float>(raw_current) / 10.0;
        

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
