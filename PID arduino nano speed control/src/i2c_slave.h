#ifndef I2CSLAVE_H
#define I2CSLAVE_H

#define DIPSWITCH_1 7
#define DIPSWITCH_2 8

// I2C Command bytes
#define CMD_SetPIDParam 0x10
#define CMD_SetPIDSetpoint 0x20

#define SCALE_FACTOR_SPEED 71.0
#define SCALE_FACTOR_TORQUE 510.0
#define SCALE_FACTOR_RPM 0.25
#define SCALE_FACTOR_CURRENT 57.0
#define SCALE_FACTOR_KP 1000
#define SCALE_FACTOR_KI 800
#define SCALE_FACTOR_KD 10000

// 60 / (PI * diameter_wheel)
#define VtoRPM 280.862
// (PI * diameter_wheel) / 60
#define RPMtoV 0.00356

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
    uint8_t getCtrlMode();

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