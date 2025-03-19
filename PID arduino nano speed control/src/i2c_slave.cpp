#include "i2c_slave.h"

I2CSlave* I2CSlave::instance = nullptr;

I2CSlave::I2CSlave() : _mode(0), _setpoint(0), _kp(0), _ki(0), _kd(0) {
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
    _mode = (0 <= mode && mode <= 1 ? mode : 0); // Validate input
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
    switch(Wire.read()){
        case CMD_SetPIDParam :
            if (bytes == 5){
                instance->_mode = Wire.read();
                instance->_kp = Wire.read() / 10.0;
                instance->_ki = Wire.read() / 10.0;
                instance->_kd = Wire.read() / 10.0;

                instance->newPIDGainsAvailable = true; // Set the flag to indicate new gains are available
                
                Serial.print("Received: Mode = ");
                Serial.print(instance->_mode);
                Serial.print(", Kp = ");
                Serial.print(instance->_kp);
                Serial.print(", Ki = ");
                Serial.print(instance->_ki);
                Serial.print(", Kd = ");
                Serial.println(instance->_kd);
            }
            Serial.print("Missing I2C data, A");
            break;

        case CMD_SetPIDSetpoint :
            if (bytes == 2){
                instance->_setpoint = Wire.read() * 10;  // Scale back

                Serial.print("Received: Setpoint = ");
                Serial.print(instance->_setpoint);
            }
            break;

        default :
            Serial.print("Invalid CMD byte received");
            break;

    }
}


void I2CSlave::requestEvent() { // Send data to master
    if (instance) {
        Wire.write((byte)(instance->_setpoint / 10));
        Wire.write((byte)(instance->_kp * 10));
        Wire.write((byte)(instance->_ki * 10));
        Wire.write((byte)(instance->_kd * 10));
    }
}