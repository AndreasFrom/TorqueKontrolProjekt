#ifndef MOTOR_PID_H
#define MOTOR_PID_H

#include <Arduino.h>

class MotorPID {
public:
    MotorPID(double kp, double ki, double kd, double setpoint = 300, double sampleTime = 0.01);
    void setGains(double kp, double ki, double kd);
    void setSetpoint(double setpoint);
    double compute(double current_value);
    void reset();

private:
    uint8_t mode_;
    double kp_, ki_, kd_; // PID gains
    double setpoint_;     // Desired setpoint
    double integral_;     // Integral term
    double prev_error_ = 1;   // Previous error for derivative term
    double sampleTime_;   // Sample time for PID computation
};

#endif  // MOTOR_PID_H