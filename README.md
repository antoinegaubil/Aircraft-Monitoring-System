# Aircraft Monitoring System

## Overview

This C++ program simulates a real-time aircraft monitoring system using threads, mutexes, scheduling and interrupts. The system monitors key sensors such as fuel level, temperature, and pressure. It also includes a smoke detector, warning lamps, and a command interface for the pilot.

## Features

### 1. Real-time Monitoring

- The program uses threads and scheduling to enable real-time monitoring of the aircraft's vital parameters.
- Threads are employed to concurrently monitor fuel level, temperature, and pressure.

### 2. Sensors

#### Fuel Sensor

- Monitors the fuel level in the aircraft.
- Generates warnings if the fuel level is critically low.

#### Engine Sensor

- Monitors temperature and pressure levels in the aircraft.
- Generates warnings if values are outside normal ranges.

#### Smoke Detector

- Simulates a smoke detector for the left and right engines.
- Generates warnings if smoke is detected.

### 3. User Interaction

- Pilot commands are processed in real-time through a dedicated thread.
- Allows the pilot to simulate smoke, clear warnings, and request sensor rates.

### 4. Warning Lamps

- The system uses warning lamps to indicate critical conditions for fuel, temperature, pressure, and smoke.

### 5. Dials

- Displays real-time sensor values such as fuel level, temperature, and pressure.
- Calculates and displays sensor rates based on initial and current values.

### 6. Concurrency and Mutex

- Utilizes mutexes to ensure thread safety during data access and printing.
- Mutexes are employed to control access to shared variables and resources.

### 7. Continuous Monitoring

- The program continuously monitors and displays information.
- Threads run concurrently, providing real-time updates on the aircraft's status.

### How to Use

- Compile and run the program using a C++ compiler.
  



