#include <JoystickController.hpp>

JoystickController::JoystickController() {
  xPin = JOYSTICK_X_AXIS;
  yPin = JOYSTICK_Y_AXIS;
  buttonPin = JOYSTICK_BUTTON_PIN;

  xValue = joystickDefaultValue;
  yValue = joystickDefaultValue;
  buttonPressed = false;
  lastButtonState = false;
  lastDebounceTime = 0;

  currentDirection = JoystickDirection::NONE;
  lastDirection = JoystickDirection::NONE;
}

void JoystickController::begin() {
  pinMode(buttonPin, INPUT_PULLUP);
}

void JoystickController::update() {
  xValue = analogRead(xPin);
  yValue = analogRead(yPin);

  lastDirection = currentDirection;

  if(yValue < centerMin) {
    currentDirection = JoystickDirection::UP;
  } else if (yValue > centerMax) {
    currentDirection = JoystickDirection::DOWN;
  } else if (xValue < centerMin) {
    currentDirection = JoystickDirection::LEFT;
  } else if (xValue > centerMax) {
    currentDirection = JoystickDirection::RIGHT;
  } else {
    currentDirection = JoystickDirection::NONE;
  }

  bool currentButtonReading = digitalRead(buttonPin) == LOW;

  if(currentButtonReading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if((millis() - lastDebounceTime) > debounceDelay) {
    buttonPressed = currentButtonReading;
  }

  lastButtonState = currentButtonReading;

}

JoystickDirection JoystickController::getDirection() const {
  return currentDirection;
}

bool JoystickController::hasNewDirection() const {
  return currentDirection != lastDirection && currentDirection != JoystickDirection::NONE;
}

void JoystickController::clearDirection() {
  lastDirection = currentDirection;
}

bool JoystickController::isButtonPressed() const {
  return buttonPressed;
}

bool JoystickController::isButtonJustPressed() {
  static bool lastPressed = false;
  bool justPressed = buttonPressed && !lastPressed;
  lastPressed = buttonPressed;
  return justPressed;
}