#ifndef TORQUE_CONTROL_H
#define TORQUE_CONTROL_H

#include <Arduino.h>

struct Currents {
    float current_left_front;
    float current_right_front;
    float current_left_rear;
    float current_right_rear;
};

class TorqueControl {
public:
    TorqueControl();
    void calculateCurrents(float velocity, Currents &currents);
};

#endif // TORQUE_CONTROL_H
