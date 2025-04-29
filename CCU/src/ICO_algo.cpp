#ifndef ICO_ALGO_CPP
#define ICO_ALGO_CPP

#include "ICO_algo.h"

ICOAlgo::ICOAlgo(double eta, double omega0, double omega1, double sampleTime, Filter *h0, Filter *h1)
: eta_(eta), omega0_(omega0), omega1_(omega1), sampleTime_(sampleTime),h0_(h0), h1_(h1), error_(0), prev_error_(0) { 
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

void ICOAlgo::setEta(double eta)
{
    eta_ = constrain(eta, 0, 1);
}

double ICOAlgo::computeChange(double input_reflex, double input_prediction, double setpoint) {
    // Calculate S0 from time delay of one sample. (Reflex)
    S0_current_ = S0_next_; // Store previous S0 value
    S0_next_ = input_reflex; // Update S0 with current input value

    // Calculate X0 which is the error
    prev_error_ = error_; // Store previous error value
    error_ = setpoint - S0_current_; // Calculate current error
    
    if (h0_ != nullptr) {
        error_ = h0_->filter(error_); // Apply filter to error
    }

    // Calculate derivative of error
    double derivativeError = (error_ - prev_error_) / sampleTime_; // Calculate derivative of error

    // Calculate change of omega1
    omega1_ += (input_reflex * eta_ * derivativeError);
    omega1_ = constrain(omega1_, -1, 1); // Constrain omega1 to be between -1 and 1

    // Calculate prediction with filterbank
    if (h1_ != nullptr) {
        input_prediction = h1_->filter(input_prediction); // Apply filter to prediction
    }

    // Calculate output
    return (input_prediction * omega1_) + (error_ * omega0_); //Reflex + Prediction
}

void ICOAlgo::updateOmegaValues(double omega0, double omega1)
{
    omega0_ = omega0;
    omega1_ = omega1;
}

void ICOAlgo::resetICO() {
    prev_error_ = 0;
}


#endif  // ICO_ALGRO_H