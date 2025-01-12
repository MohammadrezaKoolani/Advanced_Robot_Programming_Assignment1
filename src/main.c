#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>  // For rand() and srand()
#include <time.h>    // For time(NULL)
#include <sys/stat.h> // For mkdir
#include "blackboard.h"  // Correctly include the header file
#include <time.h>  // Include time functions for watchdog

// Watchdog variables
int last_movement_time = 0;  // Timestamp of last drone movement
int movement_flag = 0;       // Flag to track if the drone has moved
const int WATCHDOG_TIMEOUT = 10;  // Timeout in seconds before triggering the watchdog

void ensure_log_directory_exists() {
    struct stat st = {0};
    if (stat("log", &st) == -1) {
        mkdir("log", 0700);
    }
}

int main() {
    srand(time(NULL));  // Initialize random number generator

    initscr();               // Initialize ncurses
    cbreak();                // Disable line buffering
    keypad(stdscr, TRUE);    // Enable special keys
    noecho();                // Don't echo input characters

    mvprintw(0, 0, "Drone Simulator - Press 'q' to quit");
    refresh();

    // Ensure log directory exists
    ensure_log_directory_exists();

    // Create or open shared memory for the blackboard
    Blackboard* blackboard = create_shared_memory();

    // Initialize the drone's starting position
    blackboard->drone_x = 10;  // Set starting X position
    blackboard->drone_y = 10;  // Set starting Y position

    // Generate obstacles and targets in shared memory
    generate_obstacles(blackboard, 6, COLS, LINES);  // 6 obstacles
    generate_targets(blackboard, 6, COLS, LINES);    // 6 targets

    // Initialize the drone in shared memory at position (10, 10)
    Drone drone = {blackboard->drone_x, blackboard->drone_y, 0.0, 0.0};  // Create a Drone instance

    int ch;
    while ((ch = getch()) != 'q') {
        mvaddch(drone.y, drone.x, ' ');  // Clear old drone position

        int manual_control = 0;  // Flag to check if manual input is used

        // Handle movement (update drone position based on keyboard input)
        if (ch == KEY_UP) { drone.y--; manual_control = 1; }
        if (ch == KEY_DOWN) { drone.y++; manual_control = 1; }
        if (ch == KEY_LEFT) { drone.x--; manual_control = 1; }
        if (ch == KEY_RIGHT) { drone.x++; manual_control = 1; }

        // Set flag and reset the movement timer when drone moves
        if (manual_control) {
            movement_flag = 1;  // Set flag if the drone moves
            last_movement_time = time(NULL);  // Reset the movement timer
        }

        // Apply forces from obstacles and targets ONLY if no manual input
        double total_force_x = 0.0;
        double total_force_y = 0.0;

        if (!manual_control) {
            FILE *force_log = fopen("log/force_log.txt", "a");
            if (force_log == NULL) {
                mvprintw(LINES - 1, 0, "Error: Cannot open force_log.txt");
                refresh();
            } else {
                // Calculate repulsive forces from obstacles
                for (int i = 0; i < 10; i++) {
                    double force = calculate_repulsion(drone.x, drone.y,
                                                       blackboard->obstacles[i][0], blackboard->obstacles[i][1]);
                    total_force_x += force * (drone.x - blackboard->obstacles[i][0]);
                    total_force_y += force * (drone.y - blackboard->obstacles[i][1]);

                    fprintf(force_log, "Repulsion from Obstacle %d: Force=(%f, %f)\n", i, total_force_x, total_force_y);
                }

                // Calculate attractive forces from targets
                for (int i = 0; i < 5; i++) {
                    double force = calculate_attraction(drone.x, drone.y,
                                                        blackboard->targets[i][0], blackboard->targets[i][1]);
                    total_force_x -= force * (drone.x - blackboard->targets[i][0]);
                    total_force_y -= force * (drone.y - blackboard->targets[i][1]);

                    fprintf(force_log, "Attraction to Target %d: Force=(%f, %f)\n", i, total_force_x, total_force_y);
                }
                fclose(force_log);
            }

            // Limit the forces to prevent extreme movement
            if (total_force_x > 0.5) total_force_x = 0.5;
            if (total_force_x < -0.5) total_force_x = -0.5;
            if (total_force_y > 0.5) total_force_y = 0.5;
            if (total_force_y < -0.5) total_force_y = -0.5;

            // Apply calculated forces
            update_drone_position(&drone, total_force_x, total_force_y);
        }

        // Boundary check to prevent the drone from going off-screen
        if (drone.x < 0) drone.x = 0;
        if (drone.x >= COLS) drone.x = COLS - 1;
        if (drone.y < 0) drone.y = 0;
        if (drone.y >= LINES) drone.y = LINES - 1;

        // Sync drone's position with shared memory
        blackboard->drone_x = drone.x;
        blackboard->drone_y = drone.y;

        // Log drone's updated position
        FILE *position_log = fopen("log/drone_position_log.txt", "a");
        fprintf(position_log, "Drone Position: (%d, %d)\n", drone.x, drone.y);
        fclose(position_log);

        // Display drone at new position
        mvaddch(drone.y, drone.x, 'X');
        refresh();
    }

    endwin();  // End ncurses mode
    return 0;
}
