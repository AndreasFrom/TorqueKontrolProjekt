#ifndef I2CSLAVE_H
#define I2CSLAVE_H

#define DIPSWITCH_1 7
#define DIPSWITCH_2 8


#include <Wire.h>
#include <Arduino.h>

class I2CSlave {
public:
    I2CSlave();
    void begin();
    void setSetpointRPM(double setpoint);
    void setMode(int mode);
    void setMessuredData(double output, double current);
    void setPIDGains(double kp, double ki, double kd);
    void setTimestamp(unsigned long timestamp);
    double getSetpointRPM();
    double getKp();
    double getKi();
    double getKd();
    double getTimestamp();

    bool newPIDGainsAvailable = false;

private:
    uint8_t _address;
    double _setpointRPM;
    double _kp;
    double _ki;
    double _kd;
    double _output;
    double _current;
    unsigned long _timestamp;

    static void receiveEvent(int bytes);
    static void requestEvent();
    static I2CSlave* instance;
};

#endif