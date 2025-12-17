# Matrix Project
Repo for the Matrix project for the Introduction to Robotics course


# Checkpoint 1

## Task requirements

  The main task is to implement a system comprised of a working 8x8 LED matrix, an LCD and at least a sensor meant for interfacing with the environment. The main task itself is split into two checkpoints. The first is meant to verify the actual physical form of the circuit alongside some minimal software implementation while the latter is meant to develop the software side of the project.

## Components used

  For creating the circuit, I used the following components:

  1. Arduino Uno R3 (based on the ATMega328P)
  2. HD44780-compatible LCD
  3. A-1088BS 8x8 LED matrix
  4. MAX7219 LED driver (connected for an 8x8 LED matrix)
  5. IMU (MPU6050-based)
  6. Joystick
  7. Active buzzer
  8. Resistors, potentiometers and wires as needed
  9. 10uF electrolytic capacitor
  10. 100nF ceramic capacitor

## Schematics and photos of the setup

  Initially, I designed the following schematic in KiCAD for easier implementation on a breadboard and for keeping tracking of which pins have which function when writing the code for the system:

  ![KiCAD schematic](/Checkpoint1/Matrix_schematic.png)

  The schematic was then used to implement the following circuit on multiple breadboards:

  ![Circuit](/Checkpoint1/Circuit.jpeg)

## Video showcasing functionality

  This video presents the way the system works in its current state: [Video link](https://youtu.be/X9oOibPy6bo)

## Plans for the second checkpoint

  After completing the first checkpoint, I plan to implement a game where the player has to "balance" a virtual ball on a plate (a dot on the LED matrix) by angling the IMU so that they collect as many points as possible without letting the ball fall off the plate.


# Final presentation

  Since the first checkpoint, the system has suffered some small modifications, like being able to control the brightness of the LCD's backlight by using PWM and a MOSFET to control the duty cycle of the signal applied. Adding pull-up resistors to the I2C lines of the IMU was another minor change that will improve the stability of the system.

  After the minor changes, the circuit's list of components has changed slightly:

  1. Arduino UNO R3
  2. HD44780-compatible LCD
  3. A-1088BS 8x8 LED dot matrix
  4. MAX7219 led driver
  5. IMU (based on the MPU6050)
  6. Joystick
  7. Active buzzer
  8. 3 10K $\Omega$ resistors for pull-up/pull-down
  9. 47K $\Omega$ resistor
  10. 100K $/Omega$  potentiometer
  11. 330 $\Omega$ resistor for the LCD's backlight
  12. 100 nF ceramic capacitor
  13. 10 $\mu$ F electrolytic capacitor
  14. 1x IRFZ44N MOSFET

  After the change were made to the circuit, its diagram also changed into its final state, which is displayed below:

  ![Final KiCAD schematic](/Final/Matrix_schematic.png)

  A new view of the circuit can be seen below:
  
  [Video showcasing functionality]()
