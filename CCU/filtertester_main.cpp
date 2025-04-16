#include "src/filter.h"
#include <iostream>

int main() {
    // FIR with 3-tap moving average
    FIRFilter fir({0.33, 0.33, 0.33});
    test_filter(fir, "FIR Filter (Moving Average)",30);

    // IIR single-pole (example: y[n] = 0.7*y[n-1] + 0.3*x[n])
    IIRFilter iir(0.3, 0.7);
    test_filter(iir, "IIR Filter",30);

    // Exponential Decay with alpha = 0.2
    ExponentialDecayFilter expDecay(0.2);
    test_filter(expDecay, "Exponential Decay Filter",30);

    return 0;
}