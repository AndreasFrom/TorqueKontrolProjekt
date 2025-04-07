#ifndef ICO_ALGRO_H
#define ICO_ALGRO_H

#include <Arduino.h>

class ICOAlgro {
public:
    ICOAlgro(double IMU_T0, double IMU_T_1, double eta, double omega2 ,double sampleTime);
    double computeICOError(double IMU_T0, double IMU_T_1);
    double computeOmega1 (double IMU_T0, double IMU_T_1);
    double computeDeltaTorque_Speed(double IMU_T0, double IMU_T_1);
    void resetICO();

private:
    double IMU_T0_;  // Current data from IMU
    double IMU_T_1_; // Previous data from IMU
    double eta_;     // Learning rate
    double omega2_ = 1;   // Scale for error
    double error_; // Error used in ICO
    double prev_error_ = 1;   // Previous error for derivative term
    double sampleTime_;   // Sample time for ICO computation
    double omega1_ = 0; // dynamically updated weight
};

#endif  // ICO_ALGRO_H