#ifndef KINEMATIC_H
#define KINEMATIC_H

#include <Arduino.h>
#include <math.h>

#define d_track 0.17        //170mm
#define d_wheelbase 0.29    //290mm
#define d_wheel 0.068       //68mm

struct Velocities_diff {
    float v_left;
    float v_right;
};

struct Velocities_acker {
    float v_left_front;
    float v_right_front;
    float v_left_rear;
    float v_right_rear;
};

class Kinematic {
public:
    Kinematic();
    void getVelocities_diff(float v_set, float r_set, Velocities_diff &velocities);
    void getVelocities_acker(float v_set, float r_set, Velocities_acker &velocities);

private:
};

#endif