#ifndef MOTOR_PID_CPP
#define MOTOR_PID_CPP

#include "motor_pid.h"

MotorPID::MotorPID(double kp, double ki, double kd, double setpoint, double sampleTime)
    : kp_(kp), ki_(ki), kd_(kd), setpoint_(setpoint),
      integral_(0), prev_error_(0), sampleTime_(sampleTime) {
    // Validate sampleTime
    if (sampleTime_ <= 0) {
        Serial.println("Error: Sample time must be positive.");
        while (true); // Halt execution
    }
}

void MotorPID::setGains(double kp, double ki, double kd) {
    kp_ = kp;
    ki_ = ki;
    kd_ = kd;
}

void MotorPID::setSetpoint(double setpoint) {
    setpoint_ = constrain(setpoint, 0, 1000);  
}

double MotorPID::compute(double current_value) {
    double error = setpoint_ - current_value;
    integral_ += error * (sampleTime_);
    integral_ = constrain(integral_, -50, 50);  // Prevent integral windup
    double derivative = (error - prev_error_) / (sampleTime_);
    prev_error_ = error;

    // Calculate PID output
    return (kp_ * error) + (ki_ * integral_) + (kd_ * derivative);
}

void MotorPID::reset() {
    integral_ = 0;
    prev_error_ = 0;
}

#endif  // MOTOR_PID_CPP