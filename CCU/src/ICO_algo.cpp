#ifndef ICO_ALGO_CPP
#define ICO_ALGO_CPP

#include "ICO_algo.h"

ICOAlgo::ICOAlgo(double eta, double sampleTime, Reflex& reflex, std::vector<Predictive>& predictive)
    : eta_(eta), sampleTime_(sampleTime), reflex_(reflex), predictive_(predictive)
{
    reflex_.setSampleTime(sampleTime);
}



double ICOAlgo::getError() {
    return reflex_.getError();
}

double ICOAlgo::getEta()
{
    return eta_;
}

double ICOAlgo::getOmega1() {
    return predictive_.at(0).getOmega_n();
}

void ICOAlgo::setEta(double eta)
{
    eta_ = constrain(eta, 0, 1);
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        it->setEta(eta_);
    }
}

double ICOAlgo::computeChange(double input_reflex, double input_prediction, double setpoint)
{
    //Serial.println("computeChange");
    double derivatative_error = reflex_.computeDerivativeError(input_reflex, setpoint); // Calculate derivative error
    double reflex_out = reflex_.getFilteredError() * reflex_.getOmega0(); // Calculate reflex output
    double predictive_sum = 0; // Initialize predictive sum
    //Serial.println("Beforeloop");
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        //Serial.println("computeOutput");
        predictive_sum += it->computeOutput(input_prediction, derivatative_error); // Calculate predictive output
    }
    return (reflex_out + predictive_sum);
}


void ICOAlgo::updateOmegaValues(double omega0, double omega_predictive_start)
{
    reflex_.setOmega0(omega0); // Update reflex omega0 value
    //Serial.println("Update omega0:");
    //Serial.println(omega0);
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        //Serial.println("Update omega_predictive_start:");
        //Serial.println(omega_predictive_start);
        it->setOmega_n_start(omega_predictive_start); // Update predictive omega value
    }
}

void ICOAlgo::resetICO() {
    reflex_.resetICO(); // Reset reflex object
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        it->resetICO(); // Reset each predictive object
    }
}


Reflex::Reflex(double omega0, double sampleTime, Filter *h0)
: omega0_(omega0), sampleTime_(sampleTime), h0_(h0) {}

double ICOAlgo::getomega_n()
{
    double sum = 0;
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        sum += it->getOmega_n(); // Sum omega_n values from all predictive objects
    }
    return sum;
}

double Reflex::getOmega0()
{
    return omega0_;
}

double Reflex::getError() {
    return error_;
}

double Reflex::getFilteredError()
{
    return filtered_error_;
}

void Reflex::setOmega0(double omega0)
{
    omega0_ = omega0;
}

void Reflex::setSampleTime(double sampleTime)
{
    sampleTime_ = sampleTime;
}

void Reflex::resetICO() {
    prev_error_ = 0;
}

double Reflex::computeDerivativeError(double input_reflex, double setpoint) {
    //Serial.println("computeDerivativeError");
    // Calculate S0 from time delay of one sample. (Reflex)
    S0_current_ = S0_next_; // Store previous S0 value
    S0_next_ = input_reflex; // Update S0 with current input value

    // Calculate X0 which is the error
    prev_error_ = error_; // Store previous error value
    error_ = setpoint - S0_current_; // Calculate current error
    
    if (h0_ != nullptr) {
        //Serial.println("Filter applied to error");
        filtered_prev_error_ = filtered_error_; // Store previous filtered error value
        filtered_error_ = h0_->filter(error_); // Apply filter to error
        // Calculate derivative of error
        return (filtered_error_ - filtered_prev_error_) / sampleTime_; // Calculate derivative of error
    }

    return (error_ - prev_error_) / sampleTime_; // Calculate derivative of error
}


Predictive::Predictive(double eta, double omega_predictive_start, Filter *hn)
: eta_(eta), omega_n_(omega_predictive_start), hn_(hn) {
}

double Predictive::getOmega_n()
{
    return omega_n_;
}

void Predictive::setOmega_n_start(double omega_n_start)
{
    omega_n_start_ = omega_n_start;
    omega_n_ = omega_n_start_; // Initialize omega_n_ with omega_n_start_
}

double Predictive::getEta()
{
    return eta_;
}

void Predictive::setEta(double eta)
{
    eta_ = constrain(eta, 0, 1);
}

void Predictive::updateOmegaValue(double input_prediction, double derivativeError)
{
    // Calculate change of omega_predictive_start
    omega_n_ += (input_prediction * eta_ * derivativeError);
    omega_n_ = constrain(omega_n_, -3, 3); // Constrain omega_predictive_start to be between -1 and 1
    //Serial.print("Omega_n: ");
    //Serial.println(omega_n_);
}

void Predictive::resetICO() {
    omega_n_ = omega_n_start_;
}

double Predictive::computeOutput(double input_prediction, double derivativeError) {
    // Calculate prediction with filterbank
    if (hn_ != nullptr) {
        // Apply filter to input prediction
        //Serial.println("Filter applied to input prediction");
        input_prediction = hn_->filter(input_prediction); // Apply filter to prediction
    }

    updateOmegaValue(input_prediction, derivativeError); 

    //Serial.println("computeOutput");
    //Serial.println(input_prediction);
    double output = omega_n_ * input_prediction;
    //Serial.print("Output: ");
    //Serial.println(output);
    return output; // Calculate output
}

#endif  // ICO_ALGRO_H