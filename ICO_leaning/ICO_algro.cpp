#ifndef ICO_ALGRO_CPP
#define ICO_ALGRO_CPP

#include "ICO_algro.h"

ICOAlgro::ICOAlgro(double input, double setpoint, double eta, double omega2, double sampleTime)
: input_(input), setpoint_(setpoint), eta_(eta), omega2_(omega2), sampleTime_(sampleTime), error_(0), prev_error_(0), omega1_(0) { 
}


double ICOAlgro::computeICOError(double input, double setpoint)
{
    prev_error_ = error_;
    error_ = (input - setpoint);
    return error_;
}

double ICOAlgro::computeOmega1(double input, double setpoint)
{
    computeICOError(input, setpoint);
    double derivativeError = (error_ - prev_error_) / sampleTime_; 
    omega1_ = omega1_ + (derivativeError * input * eta_);
    return omega1_;
}


double ICOAlgro::computeChange(double input, double setpoint)
{
    return input * (computeOmega1(input,setpoint)) + (error_ * omega2_);
}

void ICOAlgro::resetICO()
{
    omega1_ = 0;
    prev_error_ = 0;
}

// IMU_0 -> input
// computeDeltaTorque_Speed -> compute_change
//