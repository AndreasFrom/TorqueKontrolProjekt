#ifndef ICO_ALGO_CPP
#define ICO_ALGO_CPP

#include "ICO_algo.h"

ICOAlgo::ICOAlgo(double eta, double omega0, double sampleTime)
: eta_(eta), omega0_(omega0), sampleTime_(sampleTime), error_(0), prev_error_(0), omega1_(0) { 
}

double ICOAlgo::getOmega1(void) {
    return omega1_;
}

double ICOAlgo::getOmega0()
{
    return omega0_;
}

double ICOAlgo::getError() {
    return error_;
}

double ICOAlgo::getEta()
{
    return eta_;
}

double ICOAlgo::computeChange(double input, double setpoint) {
    // Calculate S0 from time delay of one sample. (Reflex)
    S0_current_ = S0_next_; // Store previous S0 value
    S0_next_ = input; // Update S0 with current input value

    // Calculate X0 which is the error
    prev_error_ = error_; // Store previous error value
    error_ = setpoint - S0_current_; // Calculate current error

    // Calculate derivative of error
    double derivativeError = (error_ - prev_error_) / sampleTime_; // Calculate derivative of error

    // Calculate change of omega1
    omega1_ += (input * eta_ * derivativeError);

    // Calculate output
    return (input * omega1_) + (error_ * omega0_); //Reflex + Prediction
}

void ICOAlgo::resetICO() {
    omega1_ = 0;
    prev_error_ = 0;
}


#endif  // ICO_ALGRO_H