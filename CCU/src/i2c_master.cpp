#include "i2c_master.h"

// +--------------------+---------------------+----------+
// |        MODE        |                     |          |
// +--------------------+---------------------+----------+
// |         1          |          2          |    3     |
// | SPEED CONTROL MODE | TORQUE CONTROL MODE | RPM MODE |
// +--------------------+---------------------+----------+


I2CMaster::I2CMaster() {}

void I2CMaster::begin() {
    Wire.begin();
    Serial.println("I2C Master Ready!");
}

bool I2CMaster::sendParam(uint8_t slave_adress, uint8_t mode, float kp, float ki, float kd) {
    _mode = mode;

    uint16_t scaled_kp = kp*1000;   //Max 65.535 (Can contain three decimals)
    uint16_t scaled_ki = ki*800;    //Max 81.918 (Can contain three decimals)
    uint16_t scaled_kd = kd*10000;  //Max 6.5535 (Can contain four decimals)

    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_PARAM);
    Wire.write(mode);
    Wire.write(byte(scaled_kd & 0xFF));
    Wire.write(byte((scaled_kd >> 8) & 0xFF));
    Wire.write(byte(scaled_ki & 0xFF));
    Wire.write(byte((scaled_ki >> 8) & 0xFF));
    Wire.write(byte(scaled_kp & 0xFF));
    Wire.write(byte((scaled_kp >> 8) & 0xFF));
    Wire.endTransmission();

    if(SEND_DATA_SERIAL){
        Serial.println("Parameters sent!");
    }
    return true;
}

bool I2CMaster::sendSetpoint(uint8_t slave_adress, float setpoint) {
    Wire.beginTransmission(slave_adress);
    Wire.write(CMD_SET);
    switch (_mode)
    {
    case 0:
        Wire.write(byte(setpoint * SCALE_FACTOR_SPEED));
        break;
    case 1:
        Wire.write(byte(setpoint * SCALE_FACTOR_TORQUE));
        break;
    default:
        Wire.write(byte(setpoint * SCALE_FACTOR_RPM));
        break;
    }
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

        switch (_mode)
        {
            data.setpoint_recv = static_cast<double>(raw_setpoint) / SCALE_FACTOR_SPEED;
            data.value_recv = static_cast<double>(raw_value) / SCALE_FACTOR_SPEED;
            break;
        case 1:
            data.setpoint_recv = static_cast<double>(raw_setpoint) / SCALE_FACTOR_TORQUE;
            data.value_recv = static_cast<double>(raw_value) / SCALE_FACTOR_TORQUE;
            break;
        default:
            data.setpoint_recv = static_cast<double>(raw_setpoint) / SCALE_FACTOR_RPM;
            data.value_recv = static_cast<double>(raw_value) / SCALE_FACTOR_RPM;
            break;
        }
        
        data.current_recv = static_cast<double>((raw_current) / SCALE_FACTOR_CURRENT);
        

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
