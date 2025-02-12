#ifndef MOTOR_PID_H
#define MOTOR_PID_H

#include <Arduino.h>

class MotorPID {
public:
    MotorPID(double kp, double ki, double kd, double setpoint = 300);
    void setGains(double kp, double ki, double kd);
    void setSetpoint(double setpoint);
    double compute(double current_value);
    void reset();

private:
    double calculateDt();

    double kp_, ki_, kd_;
    double setpoint_;
    double integral_, prev_error_;
    double dt_, last_pid_update_;
};

#endif  // MOTOR_PID_H
