# Drone Simulator with Shared Memory

## Contributors
- **Sarvenaz Ashoori - S6878764**
- **Mohammadreza Koolani - s5467640**

---

## Project Overview

This project simulates the movement of a drone in a 2D space, controlled via a text-based interface using the ncurses library. The simulation includes:

- **Drone Movement**: Control the drone using arrow keys; movement is reflected on the screen.
- **Obstacle and Target Interaction**: The drone experiences repulsion near obstacles and attraction towards targets. Positions are updated upon interaction.
- **Shared Memory**: Drone state, obstacles, and targets are stored in shared memory with processes communicating through it.
- **Watchdog Mechanism**: Ensures drone activity; the system terminates after a timeout if no movement is detected.

The system utilizes shared memory, semaphores, and pipes for inter-process communication. Users interact through the terminal.

---

## Tools Required

- **GCC Compiler**
- **Linux Environment** (Tested on Ubuntu 22.04)
- **Ncurses Library** for UI
- **Shared Memory & Semaphores** for inter-process communication
- **Make** for building the project

---

## How to Run

1. Clone or download the repository.
2. Navigate to the directory and build the project using:
   ```bash
   make
   ```

3. Run the simulator using:
   ```bash
    make run
   ```
## Drone Control
- **Arrow Keys**: Move the Drone (↑, ↓, ←,  →)
- **Quit**: press q
## Drone Behavior
- **Obstacle Interaction**: Repulsive forces are applied near obstacles.
- **Target Interaction**: Attraction forces guide the drone to targets. A target is removed upon contact, and a new one appears.
- **Free Movement**: The drone can freely traverse the screen unless obstructed.
## Components of the System
### Main Process (main.c):
- **Initialization**: Sets up ncurses for UI and initializes shared memory.
- **Drone Movement**: Updates drone position based on user input.
- **Obstacle & Target Handling**: Manages obstacles and targets stored in shared memory.
- **Watchdog**: Terminates the simulation if the drone remains idle for a defined period.

**main.c Pseudocode**:
```cpp
// Initialization
Initialize random number generator with current time
Initialize ncurses (initscr, cbreak, keypad, noecho)
Display "Drone Simulator - Press 'q' to quit"

// Create shared memory (Blackboard) for storing drone, obstacles, and targets
Create shared memory for Blackboard

// Generate 10 obstacles and 5 targets with random positions
Generate obstacles in shared memory (10)
Generate targets in shared memory (5)

// Initialize drone at position (10, 10)
Initialize drone with position (10, 10), velocity 0

// Main loop
While (user input is not 'q'):

    // Clear old drone position
    Clear previous drone position on screen

    // Get user input
    ch = Get user input (getch)

    // Handle movement based on user input
    If (ch == KEY_UP):
        Move drone up
    Else If (ch == KEY_DOWN):
        Move drone down
    Else If (ch == KEY_LEFT):
        Move drone left
    Else If (ch == KEY_RIGHT):
        Move drone right

    // Update movement flag and last movement time
    If (drone moved):
        Set movement flag = true
        Set last_movement_time = current time

    // Apply forces from obstacles (repulsion)
    Initialize total_force_x = 0.0, total_force_y = 0.0
    For each obstacle in shared memory (10 obstacles):
        Calculate repulsive force between drone and obstacle
        Apply repulsive force to total_force_x, total_force_y

        // Handle obstacle collision
        If (drone collides with obstacle):
            Clear obstacle's old position
            Regenerate obstacle at a new random position

    // Apply forces from targets (attraction)
    For each target in shared memory (5 targets):
        Calculate attractive force between drone and target
        Apply attractive force to total_force_x, total_force_y

        // Handle target reached
        If (drone reaches target):
            Log target removal
            Clear target's old position
            Regenerate target at a new random position

    // Update drone position based on total forces
    Update drone position using total_force_x and total_force_y

    // Log drone's position
    Log drone's new position in file

    // Check for inactivity (watchdog timeout)
    If (time since last movement > WATCHDOG_TIMEOUT):
        If (movement flag is false):
            Log watchdog timeout
            Display "Watchdog timeout! Exiting simulation."
            Exit the program

    // Display new drone position on screen
    Display drone at new position on screen
    Refresh screen

// End ncurses session
End ncurses (endwin)

```
### Blackboard (blackboard.c and blackboard.h)
- **Shared Memory**: Manages data for the drone, obstacles, and targets.
- **Obstacle & Target Generation**:
	- generate_obstacles(): Randomly places obstacles.
	- generate_targets(): Randomly places targets.
- **Force Calculations**:
	- calculate_repulsion(): Computes repulsive forces from obstacles.
	- calculate_attraction(): Computes attractive forces from targets.

**blackboard.c Pseudocode**:
```cpp 
// Function to create or open shared memory for the blackboard
Create_shared_memory():
    // Create and open shared memory object
    shm_fd = shm_open("/blackboard", O_CREAT | O_RDWR, S_IRUSR | S_IWUSR)
    If shm_fd == -1:
        Print error message and exit

    // Set the size of shared memory object
    ftruncate(shm_fd, size of Blackboard)
    If ftruncate fails:
        Print error message and exit

    // Map the shared memory object into process memory
    blackboard = mmap(NULL, size of Blackboard, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)
    If mmap fails:
        Print error message and exit

    Return blackboard


// Function to generate obstacles in shared memory
Generate_obstacles(blackboard, num_obstacles, width, height):
    For i from 0 to num_obstacles:
        // Generate random x, y coordinates for the obstacle
        blackboard->obstacles[i][0] = Random number between 0 and width  // x position
        blackboard->obstacles[i][1] = Random number between 0 and height // y position

        // Log obstacle generation to file
        Open "log/obstacle_log.txt" for appending
        Write "Obstacle i generated at (x, y)" to the log
        Close the file

        // Display obstacle at position (x, y) on screen
        mvaddch(blackboard->obstacles[i][1], blackboard->obstacles[i][0], 'O')


    // Function to generate targets in shared memory
    Generate_targets(blackboard, num_targets, width, height):
        For i from 0 to num_targets:
            // Generate random x, y coordinates for the target
            blackboard->targets[i][0] = Random number between 0 and width
            blackboard->targets[i][1] = Random number between 0 and height
            blackboard->target_ids[i] = i + 1  // Assign target ID

        // Log target generation to file
        Open "log/target_log.txt" for appending
        Write "Target i generated at (x, y)" to the log
        Close the file

        // Display target ID at position (x, y) on screen
        mvprintw(blackboard->targets[i][1], blackboard->targets[i][0], target_ids[i])


    // Function to calculate repulsive force from an obstacle
    Calculate_repulsion(drone_x, drone_y, obstacle_x, obstacle_y):
        // Calculate distance between drone and obstacle
        distance = sqrt((drone_x - obstacle_x)^2 + (drone_y - obstacle_y)^2)

        // Calculate repulsion force
        force = 1.0 / (distance^1.5 + 0.1)  // Avoid division by zero by adding a small constant

        // Apply maximum force limit
        max_force = 0.5
        If force > max_force:
            force = max_force

        // Log the repulsion force calculation
        Open "log/repulsion_log.txt" for appending
        Write "Calculating repulsion for drone at (drone_x, drone_y) and obstacle at (obstacle_x, obstacle_y), force: force" to the log
        Close the file

        Return force


    // Function to calculate attractive force towards a target
    Calculate_attraction(drone_x, drone_y, target_x, target_y):
        // Calculate distance between drone and target
        distance = sqrt((drone_x - target_x)^2 + (drone_y - target_y)^2)

        // Log the attraction force calculation
        Open "log/attraction_log.txt" for appending
        Write "Calculating attraction for drone at (drone_x, drone_y) and target at (target_x, target_y)" to the log
        Close the file

        Return 1.0 / distance  // Attractive force is inverse of the distance
```
**blackboard.h Pseudocode**:
```cpp
// Define the structure for the drone
Drone Structure:
    int x         // Drone's x-coordinate
    int y         // Drone's y-coordinate
    double velocity_x  // Drone's velocity in the x direction
    double velocity_y  // Drone's velocity in the y direction


// Define the structure for shared memory (blackboard)
Blackboard Structure:
    int drone_x    // x-coordinate of the drone
    int drone_y    // y-coordinate of the drone
    int obstacles[10][2]  // List of 10 obstacles, each with x and y coordinates
    int targets[5][2]     // List of 5 targets, each with x and y coordinates
    int target_ids[5]     // List of target IDs


// Function declarations

// Create or open shared memory for the blackboard
Create_shared_memory():
    // Return a pointer to the blackboard structure


// Generate obstacles and store their positions in shared memory
Generate_obstacles(blackboard, num_obstacles, width, height):
    // For each obstacle, generate random positions and store in shared memory
    // Log each generated obstacle


// Generate targets and store their positions in shared memory
Generate_targets(blackboard, num_targets, width, height):
    // For each target, generate random positions and store in shared memory
    // Log each generated target


// Calculate repulsive force from an obstacle (based on distance)
Calculate_repulsion(drone_x, drone_y, obstacle_x, obstacle_y):
    // Calculate the repulsive force based on the distance from the drone to the obstacle
    // Return the calculated force


// Calculate attractive force toward a target (based on distance)
Calculate_attraction(drone_x, drone_y, target_x, target_y):
    // Calculate the attractive force based on the distance from the drone to the target
    // Return the calculated force


// Update the drone's position based on calculated forces (called in main.c)
Update_drone_position(drone, force_x, force_y):
    // Update the drone's position based on the forces calculated (not shown in blackboard.c)

```
### Drone Dynamics (drone.c)
- **Position Updates**: Calculates new positions based on forces and velocity.
- **Viscous Friction**: Simulates air resistance.

**drone.c pseudocode**:
```cpp
// Function to update the drone's position based on forces
Update_drone_position(drone, force_x, force_y):
    mass = 1.0  // Drone mass (kg)
    viscous_friction = 1.0  // Drone viscous friction coefficient
    
    // Using Euler's method for position update
    // Update velocity in X and Y directions based on applied forces
    drone.velocity_x = drone.velocity_x + (force_x / mass)  
    drone.velocity_y = drone.velocity_y + (force_y / mass)  
    
    // Update the drone's position based on velocity
    drone.x = drone.x + (int)drone.velocity_x
    drone.y = drone.y + (int)drone.velocity_y
    
    // Apply viscous friction (air resistance) to the velocity
    drone.velocity_x = drone.velocity_x * (1 - viscous_friction)
    drone.velocity_y = drone.velocity_y * (1 - viscous_friction)
```

##Build Process
The project uses a Makefile. Directories include:
- src/: Source files (main.c, blackboard.c, drone.c).
- include/: Header files (blackboard.h, common.h).
- build/: Compiled object files.
- bin/: Executables.
- log/: Log files.
## Example Commands
- **Build**: make
- **Run**: make run
- **Clean**: make clean

##Watchdog
A **watchdog** mechanism is implemented to monitor activity and ensure that the system does not remain idle for too long during the simulation. It helps in maintaining system responsiveness and preventing the simulation from running indefinitely without progress.
- **Timeout**: The watchdog will trigger a timeout if the system remains idle for more than 5 seconds. When triggered, it will end the simulation to avoid unintentional stalls.

- **Configuration**: The timeout period is set to 5 seconds by default, but this can be customized according to user preferences. The timeout duration can be adjusted in the system settings file or through the following function call:

```cpp
 set_watchdog_timeout(new_timeout)
```
In this function, new_timeout is the new timeout period in seconds. For example, to set the timeout to 10 seconds:
```cpp
set_watchdog_timeout(10)
```
- **Logs**:

	- **target_log.txt**: Logs interactions with target objects, providing detailed records of every interaction with the simulation.
	- **repulsion_log.txt**: Captures the forces of repulsion, documenting the interactions between objects that generate repulsive forces.
	- **attraction_log.txt**: Tracks attractive forces, including object interactions that result in attraction.
	- **drone_log.txt**: Logs the drone's position throughout the simulation for tracking its movements over time.
	- **watchdog_log.txt**: Specifically logs any timeout events that occur, including 	timestamps of when the simulation is terminated due to inactivity.

## Usage
Once the simulation begins, the user will observe the system's behavior through output in the terminal. The simulation environment will include the following visual representations:

 - **Drone**: Represented by an 'X' character.
 - **Obstacles**: Represented by 'O' characters.
 - **Targets**: Represented by numbers (1 to 6) corresponding to the target positions on the screen.

**Sample Output**:
When running the simulation, you will see something similar to the following in the terminal:
**add a video here**

**What to Expect**:
- Upon starting the simulation, the drone will be placed at an initial position (e.g., (10, 20)).
- You will see obstacles represented by 'O' at specific coordinates, which the drone may interact with.
- The simulation will also display targets (e.g., Target 1, Target 2, etc.) at specific positions that the drone may aim to reach.
- The Watchdog will monitor activity, and if no movement occurs for more than 5 seconds, the simulation will be terminated.
- The simulation environment will update continuously as the drone moves toward targets or interacts with obstacles.

## Future Enhancements
- **Improved Dynamics**: Add acceleration and deceleration.
- **Multi-Drone Simulation**: Enable interactions between multiple drones.
- **Enhanced Targets**: Complex scoring and interactions.
- **Graphical UI**: Replace the text interface with a GUI

##Conclusion
This project demonstrates how to use **shared memory** and **inter-process communication** in a multi-process drone simulation system. It showcases how the drone can interact with obstacles and targets and how **real-time user input** can control the drone's movement. The use of **watchdog** ensures system stability by monitoring the drone's activity.


