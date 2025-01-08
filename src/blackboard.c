// blackboard.c
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
    double eta = 1.0; // Scaling factor for repulsive force
    double rho_min = 1.0; // Minimum distance to avoid singularities
    double rho_max = 5.0; // Maximum distance of influence for the obstacle

    // Calculate Euclidean distance between the drone and the obstacle
    double distance = sqrt(pow(drone_x - obstacle_x, 2) + pow(drone_y - obstacle_y, 2));

    if (distance > rho_max) {
        // No repulsive force if the drone is outside the influence radius
        return 0.0;
    }

    // Ensure the distance does not go below rho_min to avoid extreme forces
    double effective_distance = fmax(distance, rho_min);

    // Repulsive force magnitude based on Latombe/Khatib model
    double repulsive_force = eta * (1.0 / effective_distance - 1.0 / rho_max) / (effective_distance * effective_distance);

    // Log repulsion force calculation
    FILE *logfile = fopen("log/repulsion_log.txt", "a");
    fprintf(logfile, "Calculating repulsion for drone at (%d, %d) and obstacle at (%d, %d), force: %f\n", 
            drone_x, drone_y, obstacle_x, obstacle_y, repulsive_force);
    fclose(logfile);

    return repulsive_force;
}

// Function to calculate attractive force toward a target
double calculate_attraction(int drone_x, int drone_y, int target_x, int target_y) {
    double xi = 1.0; // Scaling factor for attractive force
    double rho_goal = 10.0; // Distance beyond which the attractive force switches to conic model

    // Calculate Euclidean distance between the drone and the target
    double distance = sqrt(pow(drone_x - target_x, 2) + pow(drone_y - target_y, 2));

    // Log attraction force calculation
    FILE *logfile = fopen("log/attraction_log.txt", "a");
    fprintf(logfile, "Calculating attraction for drone at (%d, %d) and target at (%d, %d), distance: %f\n", 
            drone_x, drone_y, target_x, target_y, distance);
    fclose(logfile);

    if (distance <= rho_goal) {
        // Parabolic attractive potential within the influence radius
        return 0.5 * xi * distance * distance;
    } else {
        // Conic attractive potential beyond the influence radius
        return xi * rho_goal * (distance - 0.5 * rho_goal);
    }
}
