#ifndef ICO_ALRO_H
#define ICO_ALRO_H

#include <Arduino.h>

class ICOAlgo {
public:
    ICOAlgo(double eta, double omega2 ,double sampleTime);
    double computeICOError(double input, double setpoint);
    double computeOmega1 (double input, double setpoint);
    double computeChange(double input, double setpoint);
    void resetICO();

private:
    double input_;  // Current data from IMU
    double setpoint_; // Previous data from IMU
    double eta_;     // Learning rate
    double omega2_ = 1;   // Scale for error
    double error_; // Error used in ICO
    double prev_error_ = 1;   // Previous error for derivative term
    double sampleTime_;   // Sample time for ICO computation
    double omega1_ = 0; // dynamically updated weight
};

#endif  // ICO_ALGO_H