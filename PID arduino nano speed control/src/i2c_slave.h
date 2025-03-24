#ifndef I2CSLAVE_H
#define I2CSLAVE_H

#define DIPSWITCH_1 7
#define DIPSWITCH_2 8

// I2C Command bytes
#define CMD_SetPIDParam 0x10
#define CMD_SetPIDSetpoint 0x20

#include <Wire.h>
#include <Arduino.h>

class I2CSlave {
public:
    I2CSlave(double& currentVelocity, double& currentTorque, double& currentRPM, double& motorCurrent);
    void begin();
    void setSetpoint(double setpoint);
    void setPIDGains(double kp, double ki, double kd);
    void setCtrlMode(char mode);
    double getSetpoint();
    double getKp();
    double getKi();
    double getKd();
    char getCtrlMode();

    bool newPIDGainsAvailable = false;

private:
    uint8_t _address;
    const double& _currentVelocity;
    const double& _currentTorque;
    const double& _currentRPM;
    const double& _motorCurrent;
    uint8_t _mode;
    double _setpoint;
    double _kp;
    double _ki;
    double _kd;
    

    static void receiveEvent(int bytes);
    static void requestEvent();
    static I2CSlave* instance;
};

#endif