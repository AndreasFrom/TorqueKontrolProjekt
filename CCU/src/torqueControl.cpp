#include "torqueControl.h"

TorqueControl::TorqueControl() {}

void TorqueControl::calculateCurrents(float velocity, Currents &currents) {
    // Beregning af strøm baseret på lineære funktioner givet af brugeren
    currents.current_left_front  = 0.027 * velocity + 0.181;  // MU0
    currents.current_right_front = 0.032 * velocity + 0.358;  // MU1
    currents.current_left_rear   = 0.011 * velocity + 0.038;  // MU2
    currents.current_right_rear  = -0.025 * velocity + 0.399; // MU3
}
