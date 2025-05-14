#include "torqueControl.h"

TorqueControl::TorqueControl() {}

void TorqueControl::calculateCurrents(float velocity, Currents &currents) {
    // Beregning af strøm baseret på lineære funktioner givet af brugeren
    currents.current_left_front  = (0.4482 * velocity + 0.8415)/4; //(0.038 * velocity + 0.219)/2; // 0.027 * velocity + 0.181;  // MU0
    currents.current_right_front = currents.current_left_front;
    currents.current_left_rear   = currents.current_left_front;
    currents.current_right_rear  = currents.current_left_front;
}
