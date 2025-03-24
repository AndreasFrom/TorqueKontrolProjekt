#include "kinematic.h"

Kinematic::Kinematic() {
}

void Kinematic::getVelocities_diff(float v_set, float r_set, Velocities_diff &velocities) {
    float distance_ICR_left  = sqrt( pow( r_set - (d_track / 2), 2) + pow( (d_wheelbase / 2),2));
    float distance_ICR_right = sqrt( pow( r_set + (d_track / 2), 2) + pow( (d_wheelbase / 2),2));
    velocities.v_left  = ( v_set * ( distance_ICR_left  / r_set ) / ( cos( asin( (d_wheelbase * 0.5) / distance_ICR_left ) ) ) );
    velocities.v_right = ( v_set * ( distance_ICR_right / r_set ) / ( cos( asin( (d_wheelbase * 0.5) / distance_ICR_right) ) ) );
}


void Kinematic::getVelocities_acker(float v_set, float r_set, Velocities_acker &velocities) {
    float distance_ICR_left_rear   = ( r_set - ( d_track / 2 ) );
    float distance_ICR_right_rear  = ( r_set + ( d_track / 2 ) );
    float distance_ICR_left_front  = sqrt( pow( distance_ICR_left_rear, 2 ) + pow( d_wheelbase,2 ) );
    float distance_ICR_right_front = sqrt( pow( distance_ICR_right_rear,2 ) + pow( d_wheelbase,2 ) );

    velocities.v_left_front  = ( ( v_set * (distance_ICR_left_front  / r_set) ) / cos( asin ( d_wheelbase / distance_ICR_left_front  ) ) );
    velocities.v_right_front = ( ( v_set * (distance_ICR_right_front / r_set) ) / cos( asin ( d_wheelbase / distance_ICR_right_front ) ) );
    velocities.v_left_rear   = ( v_set * ( distance_ICR_left_rear  / r_set ) );
    velocities.v_right_rear  = ( v_set * ( distance_ICR_right_rear / r_set ) );
}