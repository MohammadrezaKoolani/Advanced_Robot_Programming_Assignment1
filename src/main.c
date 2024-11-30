#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time(NULL)
#include "blackboard.h"  // Correctly include the header file
#include <time.h>  // Include time functions for watchdog

// Watchdog variables
int last_movement_time = 0;  // Timestamp of last drone movement
int movement_flag = 0;       // Flag to track if the drone has moved
const int WATCHDOG_TIMEOUT = 5;  // Timeout in seconds before triggering the watchdog

int main() {
    srand(time(NULL));  // Initialize random number generator

    initscr();               // Initialize ncurses
    cbreak();                // Disable line buffering
    keypad(stdscr, TRUE);    // Enable special keys
    noecho();                // Don't echo input characters

    mvprintw(0, 0, "Drone Simulator - Press 'q' to quit");
    refresh();

    // Create or open shared memory for the blackboard
    Blackboard* blackboard = create_shared_memory();

    // Generate obstacles and targets in shared memory
    generate_obstacles(blackboard, 10, COLS, LINES);  // 10 obstacles
    generate_targets(blackboard, 5, COLS, LINES);      // 5 targets

    // Initialize the drone in shared memory at position (10, 10)
    Drone drone = {blackboard->drone_x, blackboard->drone_y, 0.0, 0.0};  // Create a Drone instance

    int ch;
    while ((ch = getch()) != 'q') {
        mvaddch(drone.y, drone.x, ' ');  // Clear old drone position

        // Handle movement (update drone position based on keyboard input)
        if (ch == KEY_UP) drone.y--;
        if (ch == KEY_DOWN) drone.y++;
        if (ch == KEY_LEFT) drone.x--;
        if (ch == KEY_RIGHT) drone.x++;

        // Set flag and reset the movement timer when drone moves
        if (ch == KEY_UP || ch == KEY_DOWN || ch == KEY_LEFT || ch == KEY_RIGHT) {
            movement_flag = 1;  // Set flag if the drone moves
            last_movement_time = time(NULL);  // Reset the movement timer
        }

        // Apply forces from obstacles and targets (repulsion and attraction)
        double total_force_x = 0.0;
        double total_force_y = 0.0;

        // Calculate repulsive forces from obstacles (using shared memory data)
        for (int i = 0; i < 10; i++) {
            double force = calculate_repulsion(drone.x, drone.y,
                                            blackboard->obstacles[i][0], blackboard->obstacles[i][1]);
            total_force_x += force * (drone.x - blackboard->obstacles[i][0]);  // Repulsion force in X direction
            total_force_y += force * (drone.y - blackboard->obstacles[i][1]);  // Repulsion force in Y direction

            // If the drone collides with an obstacle, regenerate it
            if (drone.x == blackboard->obstacles[i][0] && drone.y == blackboard->obstacles[i][1]) {
                // Clear the old obstacle position
                mvaddch(blackboard->obstacles[i][1], blackboard->obstacles[i][0], ' ');  // Set old position to space

                // Regenerate the obstacle at a new random position
                blackboard->obstacles[i][0] = rand() % COLS;  // New random X position
                blackboard->obstacles[i][1] = rand() % LINES; // New random Y position
                mvaddch(blackboard->obstacles[i][1], blackboard->obstacles[i][0], 'O');  // Place new obstacle at random position
            }
        }


        // Calculate attractive forces from targets (using shared memory data)
        for (int i = 0; i < 5; i++) {
            double force = calculate_attraction(drone.x, drone.y,
                                                 blackboard->targets[i][0], blackboard->targets[i][1]);
            total_force_x -= force * (drone.x - blackboard->targets[i][0]);  // Attractive force in X
            total_force_y -= force * (drone.y - blackboard->targets[i][1]);  // Attractive force in Y

            // If the drone reaches a target, remove and regenerate it
            if (drone.x == blackboard->targets[i][0] && drone.y == blackboard->targets[i][1]) {
                // Clear the old target position
                mvaddch(blackboard->targets[i][1], blackboard->targets[i][0], ' ');  
                
                // Log target removal
                FILE *logfile = fopen("log/target_log.txt", "a");
                fprintf(logfile, "Target %d reached and removed at (%d, %d)\n", blackboard->target_ids[i],
                        blackboard->targets[i][0], blackboard->targets[i][1]);
                fclose(logfile);

                // Regenerate target at a new random position
                blackboard->targets[i][0] = rand() % COLS;
                blackboard->targets[i][1] = rand() % LINES;
                blackboard->target_ids[i] = i + 1;  // Reassign ID for the target
                mvprintw(blackboard->targets[i][1], blackboard->targets[i][0], "%d", blackboard->target_ids[i]);  // Place new target at random position
            }
        }

        // Update drone position based on the forces
        update_drone_position(&drone, total_force_x, total_force_y);  // Pass pointer to drone structure

        // Log drone's position
        FILE *logfile = fopen("log/drone_log.txt", "a");
        fprintf(logfile, "Drone moved to position (%d, %d)\n", drone.x, drone.y);
        fclose(logfile);

        // Check for inactivity (watchdog)
        if (time(NULL) - last_movement_time > WATCHDOG_TIMEOUT && movement_flag == 0) {
            // Log the inactivity warning
            logfile = fopen("log/watchdog_log.txt", "a");
            fprintf(logfile, "Watchdog: No movement detected for %d seconds. Exiting...\n", WATCHDOG_TIMEOUT);
            fclose(logfile);

            mvprintw(LINES - 1, 0, "Watchdog timeout! Exiting simulation.");
            refresh();
            sleep(2);  // Delay to show the message
            endwin();  // Exit ncurses
            exit(0);   // Stop the simulation
        }

        // Display drone at new position
        mvaddch(drone.y, drone.x, 'X');
        refresh();
    }

    endwin();  // End ncurses mode
    return 0;
}
