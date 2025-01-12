#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include "blackboard.h"
#include <math.h>

int last_movement_time = 0;
int movement_flag = 0;
const int WATCHDOG_TIMEOUT = 10;
const double REPULSION_SCALE = 1.0;  // Repulsion strength
const double ATTRACTION_SCALE = 5.0;  // Attraction strength
const double OBSTACLE_AVOIDANCE_DISTANCE = 2.0;  // Distance to start repulsion
const double TARGET_ATTRACTION_DISTANCE = 5.0;  // Distance to start attraction

void ensure_log_directory_exists() {
    struct stat st = {0};
    if (stat("log", &st) == -1) {
        mkdir("log", 0700);
    }
}

void log_drone_position(int x, int y) {
    FILE *file = fopen("log/position_log.txt", "a");
    if (file) {
        fprintf(file, "Drone Position: (%d, %d)\n", x, y);
        fclose(file);
    }
}

void log_repulsion_force(int obstacle_id, double force_x, double force_y) {
    FILE *file = fopen("log/repulsion_log.txt", "a");
    if (file) {
        fprintf(file, "Repulsion from Obstacle %d: Force=(%f, %f)\n", obstacle_id, force_x, force_y);
        fclose(file);
    }
}

void log_attraction_force(int target_id, double force_x, double force_y) {
    FILE *file = fopen("log/attraction_log.txt", "a");
    if (file) {
        fprintf(file, "Attraction to Target %d: Force=(%f, %f)\n", target_id, force_x, force_y);
        fclose(file);
    }
}

int is_collision_with_obstacle(int next_x, int next_y, Blackboard *blackboard) {
    for (int i = 0; i < 10; i++) {
        if (next_x == blackboard->obstacles[i][0] && next_y == blackboard->obstacles[i][1]) {
            return 1;
        }
    }
    return 0;
}

int main() {
    srand(time(NULL));

    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();

    mvprintw(0, 0, "Drone Simulator - Press 'q' to quit");
    refresh();

    ensure_log_directory_exists();

    Blackboard *blackboard = create_shared_memory();
    blackboard->drone_x = 10;
    blackboard->drone_y = 10;

    generate_obstacles(blackboard, 6, COLS, LINES);
    generate_targets(blackboard, 6, COLS, LINES);

    Drone drone = {blackboard->drone_x, blackboard->drone_y, 0.0, 0.0};

    int ch;
    while ((ch = getch()) != 'q') {
        mvaddch((int)drone.y, (int)drone.x, ' ');

        int next_x = drone.x;
        int next_y = drone.y;

        // Allow free movement with arrow keys
        if (ch == KEY_UP) next_y--;
        if (ch == KEY_DOWN) next_y++;
        if (ch == KEY_LEFT) next_x--;
        if (ch == KEY_RIGHT) next_x++;

        // Prevent movement through obstacles
        if (!is_collision_with_obstacle(next_x, next_y, blackboard)) {
            drone.x = next_x;
            drone.y = next_y;
        }

        // Calculate and apply attraction force when close to targets
        for (int i = 0; i < 5; i++) {
            double dx = blackboard->targets[i][0] - drone.x;
            double dy = blackboard->targets[i][1] - drone.y;
            double distance = sqrt(dx * dx + dy * dy);
            if (distance < TARGET_ATTRACTION_DISTANCE) {
                double force = ATTRACTION_SCALE / distance;
                drone.x += (force * dx) / distance;
                drone.y += (force * dy) / distance;
                log_attraction_force(i, (force * dx) / distance, (force * dy) / distance);
            }
        }

        // Calculate and apply repulsion force when close to obstacles
        for (int i = 0; i < 10; i++) {
            double dx = drone.x - blackboard->obstacles[i][0];
            double dy = drone.y - blackboard->obstacles[i][1];
            double distance = sqrt(dx * dx + dy * dy);
            if (distance < OBSTACLE_AVOIDANCE_DISTANCE && distance != 0) {
                double force = REPULSION_SCALE / (distance * distance);
                drone.x += (force * dx) / distance;
                drone.y += (force * dy) / distance;
                log_repulsion_force(i, (force * dx) / distance, (force * dy) / distance);
            }
        }

        blackboard->drone_x = (int)drone.x;
        blackboard->drone_y = (int)drone.y;
        log_drone_position((int)drone.x, (int)drone.y);

        mvaddch((int)drone.y, (int)drone.x, 'X');
        refresh();
    }

    endwin();
    return 0;
}
