#ifndef ICO_ALRO_H
#define ICO_ALRO_H

#include <Arduino.h>

class ICOAlgo {
public:
    ICOAlgo(double eta, double omega2 ,double sampleTime);
    double computeChange(double input, double setpoint);

    double getOmega1();
    double getError();

    void resetICO();

private:
    double input_;      // Current data from IMU
    double setpoint_;   // Previous data from IMU

    double eta_;        // Learning rate
    double sampleTime_; // Sample time for ICO computation

    double omega0_ = 1; // Scale for error
    double omega1_ = 0; // dynamically updated weight

    double error_ = 0; // Stores current error
    double prev_error_ = 0;   // Previous error for derivative term

    double S0_current_ = 0; // Stores S0 value at t = 0
    double S0_next_ = 0; // Stores S0 value at t = 1
};

#endif  // ICO_ALGO_H