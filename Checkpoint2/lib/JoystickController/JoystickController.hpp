#ifndef JOYSTICK_CONTROLLER_HPP
#define JOYSTICK_CONTROLLER_HPP

#include <Defaults.hpp>

enum class JoystickDirection {
  NONE,
  UP,
  DOWN,
  LEFT,
  RIGHT
};

class JoystickController {
private:
  uint8_t xPin;
  uint8_t yPin;
  uint8_t buttonPin;

  uint16_t xValue;
  uint16_t yValue;
  bool buttonPressed;
  bool lastButtonState;

  static const uint16_t centerMin = 300;
  static const uint16_t centerMax = 700;
  static const uint32_t debounceDelay = 50;

  uint32_t lastDebounceTime = 0;
  JoystickDirection currentDirection;
  JoystickDirection lastDirection;

public:
  JoystickController();

  void begin();
  void update();

  JoystickDirection getDirection() const;
  bool hasNewDirection() const;
  void clearDirection();

  bool isButtonPressed() const; // level-sensitive logic
  bool isButtonJustPressed(); // edge-sensitive logic

  uint16_t getXValue() const { return xValue; }
  uint16_t getYValue() const { return yValue; }

};

#endif