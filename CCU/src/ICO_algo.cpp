#ifndef ICO_ALGO_CPP
#define ICO_ALGO_CPP

#include "ICO_algo.h"

ICOAlgo::ICOAlgo(double eta, double sampleTime, Reflex& reflex, std::vector<Predictive>& predictive)
    : eta_(eta), sampleTime_(sampleTime), reflex_(reflex), predictive_(predictive)
{
    reflex_.setSampleTime(sampleTime);
#ifdef DEBUG_ICO
    Serial.println("INIT: ICOAlgo initialized with sample time and eta");
#endif
}

double ICOAlgo::getError() {
    return reflex_.getError();
}

double ICOAlgo::getEta() {
    return eta_;
}

double ICOAlgo::getOmega1() {
    return predictive_.at(0).getOmega_n();
}

void ICOAlgo::setEta(double eta) {
    eta_ = constrain(eta, 0, 1);
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        it->setEta(eta_);
    }
#ifdef DEBUG_ICO
    Serial.print("SET: Eta set to ");
    Serial.println(eta_);
#endif
}

double ICOAlgo::computeChange(double input_reflex, double input_prediction, double setpoint) {
#ifdef DEBUG_ICO
    Serial.println("CC: Starting computeChange");
#endif
    double derivatative_error = reflex_.computeDerivativeError(input_reflex, setpoint);
    double reflex_out = reflex_.getFilteredError() * reflex_.getOmega0();
    double predictive_sum = 0;
#ifdef DEBUG_ICO
    Serial.print("CC: Reflex output = ");
    Serial.println(reflex_out);
#endif
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        double pred_output = it->computeOutput(input_prediction, derivatative_error);
#ifdef DEBUG_ICO
        Serial.print("CC: Predictive output += ");
        Serial.println(pred_output);
#endif
        predictive_sum += pred_output;
    }
    double result = reflex_out + predictive_sum;
#ifdef DEBUG_ICO
    Serial.print("CC: Total output = ");
    Serial.println(result);
#endif
    return result;
}

void ICOAlgo::updateOmegaValues(double omega0, double omega_predictive_start) {
    reflex_.setOmega0(omega0);
#ifdef DEBUG_ICO
    Serial.print("UPDATE: Omega0 set to ");
    Serial.println(omega0);
#endif
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        it->setOmega_n_start(omega_predictive_start);
#ifdef DEBUG_ICO
        Serial.print("UPDATE: Predictive omega_n_start set to ");
        Serial.println(omega_predictive_start);
#endif
    }
}

void ICOAlgo::resetICO() {
    reflex_.resetICO();
#ifdef DEBUG_ICO
    Serial.println("RESET: Reflex ICO reset");
#endif
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        it->resetICO();
#ifdef DEBUG_ICO
        Serial.println("RESET: Predictive ICO reset");
#endif
    }
}

Reflex::Reflex(double omega0, double sampleTime, Filter *h0)
: omega0_(omega0), sampleTime_(sampleTime), h0_(h0) {
#ifdef DEBUG_ICO
    Serial.println("INIT: Reflex initialized");
#endif
}

double ICOAlgo::getomega_n() {
    double sum = 0;
    for (auto it = predictive_.begin(); it != predictive_.end(); ++it) {
        sum += it->getOmega_n();
    }
#ifdef DEBUG_ICO
    Serial.print("GET: Total omega_n = ");
    Serial.println(sum);
#endif
    return sum;
}

double Reflex::getOmega0() {
    return omega0_;
}

double Reflex::getError() {
    return error_;
}

double Reflex::getFilteredError() {
    return filtered_error_;
}

void Reflex::setOmega0(double omega0) {
    omega0_ = omega0;
#ifdef DEBUG_ICO
    Serial.print("SET: Reflex omega0 = ");
    Serial.println(omega0);
#endif
}

void Reflex::setSampleTime(double sampleTime) {
    sampleTime_ = sampleTime;
#ifdef DEBUG_ICO
    Serial.print("SET: Reflex sampleTime = ");
    Serial.println(sampleTime);
#endif
}

void Reflex::resetICO() {
    prev_error_ = 0;
    error_ = 0; // Stores current error
    prev_error_ = 0;   // Previous error for derivative term
    filtered_error_ = 0; // Filtered error
    filtered_prev_error_ = 0; // Previous filtered error

    S0_current_ = 0; // Stores S0 value at t = 0
    S0_next_ = 0; // Stores S0 value at t = 1

    h0_->reset(); // Reset the filter
    
#ifdef DEBUG_ICO
    Serial.println("RESET: Reflex prev_error reset to 0");
#endif
}

double Reflex::computeDerivativeError(double input_reflex, double setpoint) {
#ifdef DEBUG_ICO
    Serial.println("CDE: Starting computeDerivativeError");
#endif
    S0_current_ = S0_next_;
    S0_next_ = input_reflex;
    prev_error_ = error_;
    error_ = setpoint - S0_current_;
#ifdef DEBUG_ICO
    Serial.print("CDE: Error = ");
    Serial.println(error_);
#endif
    if (h0_ != nullptr) {
        filtered_prev_error_ = filtered_error_;
        filtered_error_ = h0_->filter(error_);
#ifdef DEBUG_ICO
        Serial.print("CDE: Filtered error = ");
        Serial.println(filtered_error_);
#endif
        return (filtered_error_ - filtered_prev_error_) / sampleTime_;
    }
    return (error_ - prev_error_) / sampleTime_;
}

Predictive::Predictive(double eta, double omega_predictive_start, Filter *hn)
: eta_(eta), omega_n_(omega_predictive_start), hn_(hn) {
#ifdef DEBUG_ICO
    Serial.println("INIT: Predictive initialized");
#endif
}

double Predictive::getOmega_n() {
    return omega_n_;
}

void Predictive::setOmega_n_start(double omega_n_start) {
    omega_n_start_ = omega_n_start;
    omega_n_ = omega_n_start_;
#ifdef DEBUG_ICO
    Serial.print("SET: Predictive omega_n_start = ");
    Serial.println(omega_n_);
#endif
}

double Predictive::getEta() {
    return eta_;
}

void Predictive::setEta(double eta) {
    eta_ = constrain(eta, 0, 1);
#ifdef DEBUG_ICO
    Serial.print("SET: Predictive eta = ");
    Serial.println(eta_);
#endif
}

void Predictive::updateOmegaValue(double input_prediction, double derivativeError) {
    omega_n_ += (input_prediction * eta_ * derivativeError);
    omega_n_ = constrain(omega_n_, -3, 3);
#ifdef DEBUG_ICO
    Serial.print("UPDATE: omega_n = ");
    Serial.println(omega_n_);
#endif
}

void Predictive::resetICO() {
    omega_n_ = omega_n_start_;

    hn_->reset(); // Reset the filter if it exists
#ifdef DEBUG_ICO
    Serial.println("RESET: Predictive omega_n reset");
#endif
}

double Predictive::computeOutput(double input_prediction, double derivativeError) {
#ifdef DEBUG_ICO
    Serial.println("CO: Starting computeOutput");
#endif
    if (hn_ != nullptr) {
        input_prediction = hn_->filter(input_prediction);
#ifdef DEBUG_ICO
        Serial.print("CO: Filtered prediction = ");
        Serial.println(input_prediction);
#endif
    }
    updateOmegaValue(input_prediction, derivativeError);
    double output = omega_n_ * input_prediction;
#ifdef DEBUG_ICO
    Serial.print("CO: Output = ");
    Serial.println(output);
#endif
    return output;
}

#endif  // ICO_ALGRO_H
