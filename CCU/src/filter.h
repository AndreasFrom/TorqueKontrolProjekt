#ifndef FILTER_H
#define FILTER_H

#include <vector>
#include <string>


class Filter {
public:
    virtual double filter(double input) = 0;
    virtual void reset() = 0;
    virtual ~Filter() {}
};

class FIRFilter : public Filter {
private:
    std::vector<double> coefficients;
    std::vector<double> buffer;
    size_t index = 0;

public:
    FIRFilter(const std::vector<double>& coeffs);
    double filter(double input) override;
    void reset() override;
};

class IIRFilter : public Filter {
private:
    double a;  // feedback coefficient
    double b;  // feedforward coefficient
    double prev_output = 0.0;

public:
    IIRFilter(double b_coeff, double a_coeff);
    double filter(double input) override;
    void reset() override;
};

class ExponentialDecayFilter : public Filter {
private:
    double alpha;
    double state;

public:
    ExponentialDecayFilter(double alpha_value);
    double filter(double input) override;
    void reset() override;
};

class PassThroughFilter : public Filter {
public:
    double filter(double input) override { return input; }
    void reset() override {}
};

class PIDFilter : public Filter {
private:
    double kp, ki, kd;
    double prev_error;
    double integral;
    double dt; 
public:
    PIDFilter(double p, double i, double d, double time_step);

    double filter(double input) override;

    void reset() override;
};

// ===== Test Harness =====
void test_filter(Filter& filter, const std::string& name, int length = 20);

#endif // FILTER_H