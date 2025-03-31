#include "i2c_slave.h"


// +--------------------+---------------------+----------+
// |        MODE        |                     |          |
// +--------------------+---------------------+----------+
// |         1          |          2          |    3     |
// | SPEED CONTROL MODE | TORQUE CONTROL MODE | RPM MODE |
// +--------------------+---------------------+----------+


I2CSlave* I2CSlave::instance = nullptr;

I2CSlave::I2CSlave(double& currentVelocity, double& currentTorque, double& currentRPM, double& motorCurrent) : 
    _currentVelocity(currentVelocity), 
    _currentTorque(currentTorque), 
    _currentRPM(currentRPM), 
    _motorCurrent(motorCurrent),
    _mode(0), 
    _setpoint(0), 
    _kp(0), 
    _ki(0), 
    _kd(0) {
    instance = this;
}

void I2CSlave::begin() {
    pinMode(DIPSWITCH_1, INPUT);
    pinMode(DIPSWITCH_2, INPUT);
    delay(1); 

    _address = 0x08 + ((!digitalRead(DIPSWITCH_1)) << 1) + (!digitalRead(DIPSWITCH_2));

    Wire.begin(_address);
    Wire.onReceive(receiveEvent);
    Wire.onRequest(requestEvent); 
}

void I2CSlave::setSetpoint(double setpoint) {
    _setpoint = setpoint;
}

void I2CSlave::setPIDGains(double kp, double ki, double kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

void I2CSlave::setCtrlMode(char mode) {
    _mode = mode;
}

double I2CSlave::getSetpoint() {
    return _setpoint;
}

double I2CSlave::getKp() {
    return _kp;
}

double I2CSlave::getKi() {
    return _ki;
}

double I2CSlave::getKd() {
    return _kd;
}

char I2CSlave::getCtrlMode() {
    return _mode;
}

void I2CSlave::receiveEvent(int bytes) { // Read data from master
    Serial.print("Received I2C data bytes: ");
    Serial.println(bytes);
    switch(Wire.read()){
        case CMD_SetPIDParam :
            Serial.println("CMD_SetPIDParam");
            if (bytes == 11){
                instance->_mode = Wire.read();
                uint32_t msb_kp = Wire.read();
                uint16_t mmb_kp = Wire.read();
                uint8_t lsb_kp = Wire.read();
                uint32_t msb_ki = Wire.read();
                uint16_t mmb_ki = Wire.read();
                uint8_t lsb_ki = Wire.read();
                uint32_t msb_kd = Wire.read();
                uint16_t mmb_kd = Wire.read();
                uint8_t lsb_kd = Wire.read();                
                
                instance->_kp = static_cast<double>(static_cast<uint32_t>((msb_kp << 16) | (mmb_kp << 8) | lsb_kp)) / SCALE_FACTOR_KP;
                instance->_ki = static_cast<double>(static_cast<uint32_t>((msb_ki << 16) | (mmb_ki << 8) | lsb_ki)) / SCALE_FACTOR_KI;
                instance->_kd = static_cast<double>(static_cast<uint32_t>((msb_kd << 16) | (mmb_kd << 8) | lsb_kd)) / SCALE_FACTOR_KD;

                instance->newPIDGainsAvailable = true; // Set the flag to indicate new gains are available
                
                Serial.print("Received: Mode = ");
                Serial.print(instance->_mode);
                Serial.print(", Kp = ");
                Serial.print(instance->_kp,3);
                Serial.print(", Ki = ");
                Serial.print(instance->_ki,3);
                Serial.print(", Kd = ");
                Serial.println(instance->_kd,4);
                break;
            }
            Serial.print("Missing I2C data");
            break;

        case CMD_SetPIDSetpoint :
            Serial.println("CMD_SetPIDSetpoint");
            if (bytes == 2){
                switch (instance->_mode)    
                {
                case 0:
                    instance->_setpoint = (Wire.read() / SCALE_FACTOR_SPEED) * (60 / (WHEEL_DIA * PI));  // Scale for Velocity
                    break;
                case 1:
                    instance->_setpoint = Wire.read() / SCALE_FACTOR_TORQUE;  // Scale for Torque
                    break;
                case 2:
                    instance->_setpoint = Wire.read() / SCALE_FACTOR_RPM;  // Scale for RPM
                    break;
                default:
                    break;
                }

                Serial.print("Received: Setpoint = ");
                Serial.print(instance->_setpoint);
                break;
            }
            Serial.print("Missing I2C data");
            break;

        default :
            Serial.print("Invalid CMD byte received");
            break;

    }
}



void I2CSlave::requestEvent() { // Send data to master
    if (instance) {
        // Return Setpoint and measured Speed/Torque/RPM
        switch (instance->_mode){
            case 0 : // Speed
                Wire.write((byte)((instance->_setpoint * SCALE_FACTOR_SPEED) * ((PI * WHEEL_DIA)/60))); // Scale to velocity again
                Wire.write((byte)(instance->_currentVelocity * SCALE_FACTOR_SPEED));
                break;

            case 1 : // Torque
                Wire.write((byte)(instance->_setpoint * SCALE_FACTOR_TORQUE));
                Wire.write((byte)(instance->_currentTorque * SCALE_FACTOR_TORQUE));
                break;

            case 2 : // RPM (Secret mode)
                Wire.write((byte)(instance->_setpoint * SCALE_FACTOR_RPM));
                Wire.write((byte)(instance->_currentRPM * SCALE_FACTOR_RPM));
                break;

            default :
                Wire.write(0xFF);
                break;
        }

        // Return current
        Wire.write((byte)(instance->_motorCurrent * SCALE_FACTOR_CURRENT));
    }
}