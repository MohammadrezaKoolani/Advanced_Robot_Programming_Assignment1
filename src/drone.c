#include <math.h>
#include "blackboard.h"  // Include blackboard.h for the drone structure and functions



#include "blackboard.h"  // Include blackboard.h for the drone structure and functions

// Function to update the drone's position based on forces
void update_drone_position(Drone* drone, double force_x, double force_y) {
    double mass = 1.0;  // Drone mass (kg)
    double viscous_friction = 1.0;  // Drone viscous friction coefficient
    
    // Using Euler's method for position update
    drone->velocity_x += force_x / mass;  // Update velocity in X direction
    drone->velocity_y += force_y / mass;  // Update velocity in Y direction
    
    // Update position based on velocity
    drone->x += (int)drone->velocity_x;
    drone->y += (int)drone->velocity_y;
    
    // Apply viscous friction (air resistance)
    drone->velocity_x *= (1 - viscous_friction);
    drone->velocity_y *= (1 - viscous_friction);
}
