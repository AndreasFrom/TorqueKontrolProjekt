#ifndef ICO_ALRO_H
#define ICO_ALRO_H

#include <Arduino.h>
#include "filter.h"
#include <vector>

class Reflex {
    public:
        Reflex(double omega0, double sampleTime, Filter *h0 = nullptr);
        double computeDerivativeError(double input0, double setpoint);
        double getOmega0();
        double getError();
        double getFilteredError();
        void setOmega0(double omega0);
        void setSampleTime(double sampleTime);
        void clearFilter();

        void resetICO();
    private:
        double input_;      // Current data from IMU
        double setpoint_;   // Previous data from IMU

        double sampleTime_; // Sample time for ICO computation

        double omega0_ = 0.1; // Scale for error

        double error_ = 0; // Stores current error
        double prev_error_ = 0;   // Previous error for derivative term
        double filtered_error_ = 0; // Filtered error
        double filtered_prev_error_ = 0; // Previous filtered error

        double S0_current_ = 0; // Stores S0 value at t = 0
        double S0_next_ = 0; // Stores S0 value at t = 1

        Filter *h0_ = nullptr; // Pointer to filter object
};

class Predictive {
    public:
        Predictive(double eta, double omega_predictive_start, Filter *hn = nullptr);
        double computeOutput(double input_prediction, double input_reflex);
        void updateOmegaValue(double input_prediction, double derivativeError);
        double getOmega_n();
        void setOmega_n_start(double omega_n_start);
        double getEta();
        void setEta(double eta);
        void clearFilter();

        void resetICO();

    private:
        double eta_;        // Learning rate

        double omega_n_start_; // Scale for error
        double omega_n_;

        Filter *hn_ = nullptr; // Pointer to filter object
};


class ICOAlgo {
public:
    /// @brief 
    /// @param eta 
    /// @param omega0 
    /// @param omega_predictive_start 
    /// @param sampleTime 
    /// @param filter
    ICOAlgo(double eta, double sampleTime, Reflex& reflex, std::vector<Predictive>& predictive);
    double computeChange(double input0, double input1, double setpoint);

    void updateOmegaValues(double omega0, double omega_n);

    double getomega_n();
    double getOmega1();
    double getOmega0();
    double getError();
    double getEta();
    double getPredictiveSum() { return predictive_sum_; } 
    void  setEta(double eta);
    void clearFilters();

    void resetICO();

private:
    double input_;      // Current data from IMU
    double setpoint_;   // Previous data from IMU

    double eta_;        // Learning rate
    double sampleTime_; // Sample time for ICO computation
    double predictive_sum_ = 0; // Sum of predictive outputs

    Reflex& reflex_; // Reflex object for prediction

    std::vector<Predictive>& predictive_;
};


#endif  // ICO_ALGO_H