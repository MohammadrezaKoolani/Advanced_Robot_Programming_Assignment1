#define _POSIX_C_SOURCE 200809L  // For POSIX shared memory functions
#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/mman.h>   // For mmap(), shm_open()
#include <fcntl.h>      // For shm_open()
#include <sys/stat.h>   // For S_IRUSR, S_IWUSR, and file permissions
#include <unistd.h>     // For ftruncate(), close(), and other system calls
#include "blackboard.h"  // Include the header file that defines the structures and function prototypes

// Create or open shared memory object for blackboard
Blackboard* create_shared_memory() {
    // Create and open shared memory object
    int shm_fd = shm_open("/blackboard", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (shm_fd == -1) {
        perror("Failed to open shared memory object");
        exit(1);
    }
    
    // Set the size of shared memory object
    if (ftruncate(shm_fd, sizeof(Blackboard)) == -1) {
        perror("Failed to set size of shared memory object");
        exit(1);
    }
    
    // Map the shared memory object into memory
    Blackboard* blackboard = mmap(NULL, sizeof(Blackboard), PROT_READ | PROT_WRITE,
                                  MAP_SHARED, shm_fd, 0);
    if (blackboard == MAP_FAILED) {
        perror("Failed to map shared memory");
        exit(1);
    }
    
    return blackboard;
}

// Function to generate obstacles and store them in shared memory
void generate_obstacles(Blackboard* blackboard, int num_obstacles, int width, int height) {
    for (int i = 0; i < num_obstacles; i++) {
        // Generate random positions for obstacles
        blackboard->obstacles[i][0] = rand() % width;  // Random X position
        blackboard->obstacles[i][1] = rand() % height; // Random Y position
        
        // Log the obstacle generation
        FILE *logfile = fopen("log/obstacle_log.txt", "a");
        fprintf(logfile, "Obstacle %d generated at (%d, %d)\n", i+1, blackboard->obstacles[i][0], blackboard->obstacles[i][1]);
        fclose(logfile);
        
        // Represent the obstacle with 'O' on the screen
        mvaddch(blackboard->obstacles[i][1], blackboard->obstacles[i][0], 'O');
    }
}

// Function to generate targets and store them in shared memory
void generate_targets(Blackboard* blackboard, int num_targets, int width, int height) {
    for (int i = 0; i < num_targets; i++) {
        // Generate random positions for targets
        blackboard->targets[i][0] = rand() % width;
        blackboard->targets[i][1] = rand() % height;
        blackboard->target_ids[i] = i + 1;  // Assign an ID to each target
        
        // Log the target generation
        FILE *logfile = fopen("log/target_log.txt", "a");
        fprintf(logfile, "Target %d generated at (%d, %d)\n", blackboard->target_ids[i], blackboard->targets[i][0], blackboard->targets[i][1]);
        fclose(logfile);
        
        // Represent the target with its ID
        mvprintw(blackboard->targets[i][1], blackboard->targets[i][0], "%d", blackboard->target_ids[i]);
    }
}

// Function to calculate repulsive force from an obstacle
double calculate_repulsion(int drone_x, int drone_y, int obstacle_x, int obstacle_y) {
    double distance = sqrt(pow(drone_x - obstacle_x, 2) + pow(drone_y - obstacle_y, 2));
    
    // Apply a more gradual repulsion force and limit the force strength
    double force = 1.0 / (pow(distance, 1.5) + 0.1);  // Adjust exponent and add small constant to avoid division by zero
    
    // Optionally, apply a maximum force limit
    double max_force = 0.5;  // Max repulsion force
    if (force > max_force) {
        force = max_force;
    }

    // Log repulsion force calculation
    FILE *logfile = fopen("log/repulsion_log.txt", "a");
    fprintf(logfile, "Calculating repulsion for drone at (%d, %d) and obstacle at (%d, %d), force: %f\n", 
            drone_x, drone_y, obstacle_x, obstacle_y, force);
    fclose(logfile);
    
    return force;  // Return the calculated force
}


// Function to calculate attractive force toward a target
double calculate_attraction(int drone_x, int drone_y, int target_x, int target_y) {
    double distance = sqrt(pow(drone_x - target_x, 2) + pow(drone_y - target_y, 2));
    
    // Log attraction force calculation
    FILE *logfile = fopen("log/attraction_log.txt", "a");
    fprintf(logfile, "Calculating attraction for drone at (%d, %d) and target at (%d, %d)\n", drone_x, drone_y, target_x, target_y);
    fclose(logfile);
    
    return 1.0 / distance;  // Attractive force: inverse distance
}

