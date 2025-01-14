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
Initialize random seed
Initialize ncurses UI
Create shared memory for Blackboard
Generate 10 random obstacles and 5 random targets
Initialize drone at (10, 10) with zero velocity

// Main Loop
While (user input != 'q'):
    Get user input
    Move drone based on arrow keys
    Apply repulsive forces from obstacles
    Apply attractive forces from targets
    Update drone position with total forces
    Check watchdog for inactivity
    Refresh UI display

End ncurses session
```
### Blackboard (blackboard.c and blackboard.h)
- **Shared Memory**: Manages data for the drone, obstacles, and targets.
- **Obstacle & Target Generation**:
	- generate_obstacles(): Randomly places obstacles.
	- generate_targets(): Randomly places targets.
- **Force Calculations**:
	- calculate_repulsion(): Computes repulsive forces from obstacles.
	- calculate_attraction(): Computes attractive forces from targets.

**blackboard.c and blackboard.h Pseudocode**:
```cpp 
// Shared Memory Setup
Create shared memory for Blackboard

// Obstacle and Target Generation
Generate random obstacles (10) and targets (5) in shared memory

// Force Calculations
For each obstacle: Calculate and apply repulsive force to drone
For each target: Calculate and apply attractive force to drone
```
### Drone Dynamics (drone.c)
- **Position Updates**: Calculates new positions based on forces and velocity.
- **Viscous Friction**: Simulates air resistance.

**drone.c pseudocode**:
```cpp
// Update Drone Position
Calculate total force (repulsion + attraction)
Update velocity using Euler's method
Update position based on velocity
Apply viscous friction to velocity
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
// Monitor Drone Activity
If drone idle time > TIMEOUT:
    Log timeout
    Terminate simulation
```
- **Logs**:

	- **target_log.txt**: Logs interactions with target objects, providing detailed records of every interaction with the simulation.
	- **repulsion_log.txt**: Captures the forces of repulsion, documenting the interactions between objects that generate repulsive forces.
	- **attraction_log.txt**: Tracks attractive forces, including object interactions that result in attraction.
	- **drone_log.txt**: Logs the drone's position throughout the simulation for tracking its movements over time.
	- **watchdog_log.txt**: Specifically logs any timeout events that occur, including 	timestamps of when the simulation is terminated due to inactivity.
## Flowchart of Drone Simulation Process
The flowchart below provides a visual representation of the drone simulator's operational flow. It outlines the system's initialization, user input handling, obstacle and target interactions, position updates, and watchdog monitoring for inactivity.



### Flowchart Description:

- **Initialization**: The system sets up the environment, creates shared memory, and generates obstacles and targets.

- **User Input**: The drone moves based on user input through arrow keys.

- **Obstacle and Target Interaction**:

	- If the drone is near obstacles, repulsive forces are applied.

	- If the drone is far from targets, attractive forces are applied.

- **Position Update**: The drone's position is updated based on the combined forces.

- **Display**: The updated position is shown on the screen.

- **Watchdog Mechanism**: If the drone remains inactive for more than 5 seconds, the simulation exits due to timeout.

- **Termination**: The simulation can be manually exited by pressing 'q'.
  
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


