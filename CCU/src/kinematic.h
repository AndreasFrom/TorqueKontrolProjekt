#ifndef KINEMATIC_H
#define KINEMATIC_H

#include <Arduino.h>
#include <math.h>

#define d_track 0.17        //170mm
#define d_wheelbase 0.29    //290mm
#define d_wheel 0.068       //68mm

struct Velocities_acker {
    float v_left_front;
    float v_right_front;
    float v_left_rear;
    float v_right_rear;
};

class Kinematic {
public:
    Kinematic();
    /**
     * @brief Calculate the velocities for a given set velocity and radius.
     * 
     * @param v_set The desired linear velocity of the vehicle (m/s).
     * @param r_set The desired turning radius of the vehicle (m).
     * @param velocities The structure to store the calculated wheel velocities (m/s).
     */
    void getVelocities_acker(float v_set, float r_set, Velocities_acker &velocities);

    /**
     * @brief Calculate the velocities for a given set velocity and radius.
     * 
     * @param v_set The desired linear velocity of the vehicle (m/s).
     * @param r_set The desired turning radius of the vehicle (m).
     * @param velocities The structure to store the calculated wheel velocities RPM.
     */
    void getRpms_acker(float v_set, float r_set, Velocities_acker & velocities);

    /**
     * @brief Calculate the Ackermann steering velocities for a given set velocity and angular velocity.
     * 
     * @param v_set The desired linear velocity of the vehicle (m/s).
     * @param omega_set The desired angular velocity of the vehicle (deg/s).
     * @param velocities The structure to store the calculated velocities for each wheel (m/s).
     */
    void getVelocities_acker_omega(float v_set, float omega_set, Velocities_acker & velocities);
    

private:
};

#endif