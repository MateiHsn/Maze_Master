# Matrix Project
Repo for the Matrix project for the Introduction to Robotics course

  ## Task requirements

  In the case of this project, I implemented a maze-runner type of game on an Arduino dev board that controlled an 8x8 LED dot matrix display in order to display the gameplay. The game itself controls the game logic at a maximum dimension of 16x16, which can be modified by changing the logic slightly (primarily how the maps are stored and how entity positions and checks are done). This project was quite challenging, given the memory limitations of the ATMega328P.

  ## Components used

  1. Arduino UNO R3
  2. HD44780-compatible LCD
  3. A-1088BS 8x8 LED dot matrix
  4. MAX7219 led driver
  5. IMU (based on the MPU6050)
  6. Joystick
  7. Active buzzer
  8. 3 10K $\Omega$ resistors for pull-up/pull-down
  9. 47K $\Omega$ resistor
  10. 100K $\Omega$  potentiometer
  11. 330 $\Omega$ resistor for the LCD's backlight
  12. 100 nF ceramic capacitor
  13. 10 $\mu$ F electrolytic capacitor
  14. 1x IRFZ44N MOSFET

  A small detail that helps the circuit work a bit more reliably on other Arduino boards is the backlight control of the LCD's backlight, which is done by controlling a MOSFET using a pin compatible with PWM. When I first implemented the circuit, I used an Arduino Uno R4 Wi-Fi, which is unable to supply more than 8 mA per I/O pin, which could've made the backlight too dim. So, in order to allow the flow of the highest current possible, I switched to using a transistor with a pull-down resistor to control the backlight when using either the R4 or the R3.

  # Final look of the system

  After the change were made to the circuit, its diagram also changed into its final state, which is displayed below:

  ![Final KiCAD schematic](/Final/Matrix_schematic.png)

  ![Top-down View](/Final/top_down_view.jpeg)

  ![Side View](/Final/side_view.jpeg)

  The final, functioning circuit can be seen below:

  [Video showcasing functionality](https://youtu.be/UF_sIgvUU6U)
