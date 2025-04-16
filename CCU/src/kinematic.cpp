#include "kinematic.h"

Kinematic::Kinematic() {
}

void Kinematic::getVelocities_acker(float v_set, float r_set, Velocities_acker &velocities) {
    float distance_ICR_left_rear   = ( r_set - ( d_track / 2 ) );
    float distance_ICR_right_rear  = ( r_set + ( d_track / 2 ) );
    float distance_ICR_left_front  = sqrt( pow( distance_ICR_left_rear, 2 ) + pow( d_wheelbase,2 ) );
    float distance_ICR_right_front = sqrt( pow( distance_ICR_right_rear,2 ) + pow( d_wheelbase,2 ) );

    velocities.v_left_front  = ( v_set * ( distance_ICR_left_front  / r_set) );
    velocities.v_right_front = ( v_set * ( distance_ICR_right_front / r_set) );
    velocities.v_left_rear   = ( v_set * ( distance_ICR_left_rear  / r_set ) );
    velocities.v_right_rear  = ( v_set * ( distance_ICR_right_rear / r_set ) );
}

void Kinematic::getRpms_acker(float v_set, float r_set, Velocities_acker & velocities) {
    getVelocities_acker(v_set, r_set, velocities);
    velocities.v_left_front  = ( ( velocities.v_left_front * 60.0 )  / ( PI * d_wheel ) );
    velocities.v_right_front = ( ( velocities.v_right_front * 60.0 ) / ( PI * d_wheel ) );
    velocities.v_left_rear   = ( ( velocities.v_left_rear * 60.0 )   / ( PI * d_wheel ) );
    velocities.v_right_rear  = ( ( velocities.v_right_rear * 60.0 ) / ( PI * d_wheel ) );
}

void Kinematic::getVelocities_acker_omega(float v_set, float omega_set, Velocities_acker &velocities) {
    float r_cal = ( v_set / omega_set );
    getVelocities_acker(v_set, r_cal, velocities);
}
