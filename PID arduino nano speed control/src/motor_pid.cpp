#ifndef MOTOR_PID_CPP
#define MOTOR_PID_CPP

#include "motor_pid.h"


MotorPID::MotorPID(double kp, double ki, double kd,double setpoint)
    : kp_(kp), ki_(ki), kd_(kd), setpoint_(setpoint), integral_(0), prev_error_(0) {}


void MotorPID::setGains(double kp, double ki, double kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}


void MotorPID::setSetpoint(double setpoint) {
    setpoint_ = constrain(setpoint, 0, 1000);  
}


double MotorPID::compute(double current_value) {
    double dt = calculateDt();

    double error = setpoint_ - current_value;
    integral_ += error * dt;
    integral_ = constrain(integral_, -50, 50);  // Prevent integral windup
    double derivative = (error - prev_error_) / dt;
    prev_error_ = error;
    return (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);
}


void MotorPID::reset() {
    integral_ = 0;
    prev_error_ = 0;
}

double MotorPID::calculateDt() {
    unsigned long now = millis();
    double dt = (now - last_pid_update_) / 1000.0; // Convert to seconds
    last_pid_update_ = now;
    return dt;
}

#endif  // MOTOR_PID_CPP
