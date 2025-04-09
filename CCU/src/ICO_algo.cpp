#ifndef ICO_ALGO_CPP
#define ICO_ALGO_CPP

#include "ICO_algo.h"

ICOAlgo::ICOAlgo(double eta, double omega2, double sampleTime)
: eta_(eta), omega2_(omega2), sampleTime_(sampleTime), error_(0), prev_error_(0), omega1_(0) { 
}


double ICOAlgo::computeICOError(double input, double setpoint)
{
    prev_error_ = error_;
    error_ = (input - setpoint);
    return error_;
}

double ICOAlgo::computeOmega1(double input, double setpoint)
{
    computeICOError(input, setpoint);
    double derivativeError = (error_ - prev_error_) / sampleTime_; 
    omega1_ = omega1_ + (derivativeError * input * eta_);
    return omega1_;
}


double ICOAlgo::computeChange(double input, double setpoint)
{
    return input * (computeOmega1(input,setpoint)) + (error_ * omega2_);
}

void ICOAlgo::resetICO()
{
    omega1_ = 0;
    prev_error_ = 0;
}
#endif  // ICO_ALGRO_H