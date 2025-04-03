#ifndef ICO_ALGRO_CPP
#define ICO_ALGRO_CPP

#include "ICO_algro.h"

ICOAlgro::ICOAlgro(double IMU_T0, double IMU_T_1, double eta, double omega2, double sampleTime)
: IMU_T0_(IMU_T0), IMU_T_1_(IMU_T_1), eta_(eta), omega2_(omega2), sampleTime_(sampleTime), error_(0), prev_error_(0), omega1_(0) { 
}


double ICOAlgro::computeICOError(double IMU_T0, double IMU_T_1)
{
    prev_error_ = error_;
    error_ += (IMU_T0 - IMU_T_1);
    return error_;
}

double ICOAlgro::computeOmega1(double IMU_T0, double IMU_T_1, double eta)
{
    computeICOError(IMU_T0, IMU_T_1);
    double derivativeError = (error_ - prev_error_) / sampleTime_; 
    omega1_ = omega1_ + (derivativeError * IMU_T0 * eta);
    return omega1_;
}


double ICOAlgro::computeDeltaTorque_Speed(double IMU_T0, double IMU_T_1, double eta, double omega2)
{
    // delta T = IMU_T0 * omega1 + error * omega2
    double deltaOutput = (IMU_T0 * (computeOmega1(IMU_T0,IMU_T_1,eta))) + (error_ * omega2); 
    return deltaOutput;
}

void ICOAlgro::resetICO()
{
    omega1_ = 0;
    omega2_ = 1;
    prev_error_ = 0;
}
