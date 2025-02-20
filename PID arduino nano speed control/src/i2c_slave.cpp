#include "i2c_slave.h"

I2CSlave* I2CSlave::instance = nullptr;

I2CSlave::I2CSlave() : _setpointRPM(0), _kp(0), _ki(0), _kd(0) {
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

void I2CSlave::setSetpointRPM(double setpoint) {
    _setpointRPM = setpoint;
}

void I2CSlave::setPIDGains(double kp, double ki, double kd) {
    _kp = kp;
    _ki = ki;
    _kd = kd;
}

double I2CSlave::getSetpointRPM() {
    return _setpointRPM;
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


void I2CSlave::receiveEvent(int bytes) {
    if (instance && bytes >= 4) {  // Ensure enough data is received
        instance->_setpointRPM = Wire.read() * 10;  // Scale back
        instance->_kp = Wire.read() / 10.0;
        instance->_ki = Wire.read() / 10.0;
        instance->_kd = Wire.read() / 10.0;

        instance->newPIDGainsAvailable = true; // Set the flag to indicate new gains are available

        Serial.print("Received: Setpoint = ");
        Serial.print(instance->_setpointRPM);
        Serial.print(", Kp = ");
        Serial.print(instance->_kp);
        Serial.print(", Ki = ");
        Serial.print(instance->_ki);
        Serial.print(", Kd = ");
        Serial.println(instance->_kd);
    }
}


void I2CSlave::requestEvent() {
    if (instance) {
        Wire.write((byte)(instance->_setpointRPM / 10));
        Wire.write((byte)(instance->_kp * 10));
        Wire.write((byte)(instance->_ki * 10));
        Wire.write((byte)(instance->_kd * 10));
    }
}