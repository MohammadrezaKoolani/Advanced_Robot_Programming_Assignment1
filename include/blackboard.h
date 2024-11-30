#ifndef BLACKBOARD_H
#define BLACKBOARD_H

#include <stdlib.h>
#include <unistd.h>

// Define the structure for the drone
typedef struct {
    int x, y;
    double velocity_x, velocity_y;
} Drone;

// Define the structure for shared memory (blackboard)
typedef struct {
    int drone_x;
    int drone_y;
    int obstacles[10][2];  // 10 obstacles, each with x and y coordinates
    int targets[5][2];     // 5 targets, each with x and y coordinates
    int target_ids[5];     // IDs for each target
} Blackboard;

// Function declarations

// Create or open shared memory for the blackboard
Blackboard* create_shared_memory();

// Generate obstacles and store their positions in shared memory
void generate_obstacles(Blackboard* blackboard, int num_obstacles, int width, int height);

// Generate targets and store their positions in shared memory
void generate_targets(Blackboard* blackboard, int num_targets, int width, int height);

// Calculate repulsive force from an obstacle (based on distance)
double calculate_repulsion(int drone_x, int drone_y, int obstacle_x, int obstacle_y);

// Calculate attractive force toward a target (based on distance)
double calculate_attraction(int drone_x, int drone_y, int target_x, int target_y);

// Update the drone's position based on calculated forces (called in main.c)
void update_drone_position(Drone* drone, double force_x, double force_y);

#endif  // BLACKBOARD_H
