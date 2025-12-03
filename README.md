# Matrix Project
Repo for the Matrix project for the Introduction to Robotics course

## Task requirements

  The main task is to implement a system comprised of a working 8x8 LED matrix, an LCD and at least a sensor meant for interfacing with the environment. The main task itself is split into two checkpoints. The first is meant to verify the actual physical form of the circuit alongside some minimal software implementation while the latter is meant to develop the software side of the project.

## Components used

  For creating the circuit, I used the following components:

  1. Arduino Uno R4 (Renesas RA4M1-based)
  2. HD44780-compatible LCD
  3. A-1088BS 8x8 LED matrix
  4. MAX7219 LED driver (connected for an 8x8 LED matrix)
  5. IMU (MPU6050-based)
  6. Joystick
  7. Active buzzer
  8. Resistors, potentiometers and wires as needed

## Schematics and photos of the setup

  Initially, I designed the following schematic in KiCAD for easier implementation on a breadboard and for keeping tracking of which pins have which function when writing the code for the system:

  ![KiCAD schematic](/Checkpoint1/Matrix_schematic.png)

  The schematic was then used to implement the following circuit on multiple breadboards:

  ![Circuit](/Checkpoint1/Circuit.jpeg)

## Video showcasing functionality

  This video presents the way the system works in its current state: [Video link]()

## Plans for the second checkpoint

  After completing the first checkpoint, I plan to implement a game where the player has to "balance" a virtual ball on a plate (a dot on the LED matrix) by angling the IMU so that they collect as many points as possible without letting the ball fall off the plate.
