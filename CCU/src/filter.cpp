#include "filter.h"
#include <iostream>

// FIRFilter implementation
FIRFilter::FIRFilter(const std::vector<double>& coeffs) 
    : coefficients(coeffs), buffer(coeffs.size(), 0.0) {}

double FIRFilter::filter(double input) {
    buffer[index] = input;
    double output = 0.0;
    size_t idx = index;
    
    for (size_t i = 0; i < coefficients.size(); i++) {
        output += coefficients[i] * buffer[idx];
        idx = (idx == 0) ? buffer.size() - 1 : idx - 1;
    }
    
    index = (index + 1) % buffer.size();
    return output;
}

void FIRFilter::reset() {
    std::fill(buffer.begin(), buffer.end(), 0.0);
    index = 0;
}

// IIRFilter implementation
IIRFilter::IIRFilter(double b_coeff, double a_coeff) 
    : a(a_coeff), b(b_coeff), prev_output(0.0) {}

double IIRFilter::filter(double input) {
    double output = b * input - a * prev_output;
    prev_output = output;
    return output;
}

void IIRFilter::reset() {
    prev_output = 0.0;
}

// ExponentialDecayFilter implementation
ExponentialDecayFilter::ExponentialDecayFilter(double alpha_value) 
    : alpha(alpha_value), state(0.0) {}

double ExponentialDecayFilter::filter(double input) {
    state = alpha * input + (1 - alpha) * state;
    return state;
}

void ExponentialDecayFilter::reset() {
    state = 0.0;
}

// Test harness implementation
void test_filter(Filter& filter, const std::string& name, int length) {
    std::cout << "Testing " << name << ":\n";
    std::cout << "Step response:\n";
    
    // Step input test
    filter.reset();
    for (int i = 0; i < length; i++) {
        double input = (i >= length/2) ? 1.0 : 0.0;
        double output = filter.filter(input);
        printf("%2d: in=%.1f out=%.4f\n", i, input, output);
    }
    
    // Impulse test
    std::cout << "\nImpulse response:\n";
    filter.reset();
    for (int i = 0; i < length; i++) {
        double input = (i == length/2) ? 1.0 : 0.0;
        double output = filter.filter(input);
        printf("%2d: in=%.1f out=%.4f\n", i, input, output);
    }
    std::cout << "------------------------\n\n";
}

PIDFilterbank::PIDFilterbank(double kp_value, double ki_value, double kd_value, double dt_value)
{
    kp = kp_value;
    ki = ki_value;
    kd = kd_value;
    dt = dt_value;
    prev_error = 0.0;
    integral = 0.0;
}

double PIDFilterbank::filter(double input)
{
    double error = input; // Assuming input is the setpoint
    integral += error * dt;
    double derivative = (error - prev_error) / dt;
    double output = kp * error + ki * integral + kd * derivative;
    
    prev_error = error;
    return output;
}

void PIDFilterbank::reset()
{
    prev_error = 0.0;
    integral = 0.0;
}
