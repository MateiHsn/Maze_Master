#include <HardwareController.hpp>

HardwareController::HardwareController(JoystickController * joy, IMUController * imuSensor, AudioController * audioCtrl) {
    joystick = joy;
    imu = imuSensor;
    audio = audioCtrl;
    useIMUForGameplay = false;
    lastMoveTime = 0;
}

void HardwareController::init() {
  joystick->begin();
  if(imu->begin()) {
    useIMUForGameplay = true;
  }
  audio->begin();
}

void HardwareController::update() {
  joystick->update();
  if(useIMUForGameplay && imu->isAvailable()) {
    imu->update();
  }

  audio->update();

  currentInput.upPressed = false;
  currentInput.downPressed = false;
  currentInput.leftPressed = false;
  currentInput.rightPressed = false;
  currentInput.buttonJustPressed = false;

  JoystickDirection joyDir = joystick->getDirection();
  currentInput.buttonPressed = joystick->isButtonPressed();
  currentInput.buttonJustPressed = joystick->isButtonJustPressed();

  if(joyDir == JoystickDirection::UP) currentInput.upPressed = true;
  if(joyDir == JoystickDirection::DOWN) currentInput.downPressed = true;
  if(joyDir == JoystickDirection::LEFT) currentInput.leftPressed = true;
  if(joyDir == JoystickDirection::RIGHT) currentInput.rightPressed = true;

  if(useIMUForGameplay && imu->isAvailable()) {
    TiltDirection tiltDir = imu->getDirection();
    if(tiltDir == TiltDirection::UP) currentInput.upPressed = true;
    if(tiltDir == TiltDirection::DOWN) currentInput.downPressed = true;
    if(tiltDir == TiltDirection::LEFT) currentInput.leftPressed = true;
    if(tiltDir == TiltDirection::RIGHT) currentInput.rightPressed = true;
  }
}

const InputState & HardwareController::getInput() const {
  return currentInput;
}

bool HardwareController::canMove() const {
  return (millis() - lastMoveTime) >= moveCooldown;
}

void HardwareController::resetMoveCooldown() {
  lastMoveTime = millis();
}

void HardwareController::playMenuMove()       { audio->playSound(SoundEffect::MENU_MOVE); }

void HardwareController::playMenuSelect()     { audio->playSound(SoundEffect::MENU_SELECT); }

void HardwareController::playCollectItem()    { audio->playSound(SoundEffect::COLLECT_CUP); }

void HardwareController::playPlayerDeath()    { audio->playSound(SoundEffect::PLAYER_DEATH); }

void HardwareController::playLevelComplete()  { audio->playSound(SoundEffect::LEVEL_COMPLETE); }

void HardwareController::playGameOver()       { audio->playSound(SoundEffect::GAME_OVER); }

void HardwareController::playVictory()        { audio->playSound(SoundEffect::VICTORY); }

void HardwareController::playStartupSound()   { audio->playSound(SoundEffect::STARTUP); }

void HardwareController::setIMUMode(bool enabled) {
  if(imu->isAvailable()) useIMUForGameplay = enabled;
}

bool HardwareController::isIMUMode() const {
  return useIMUForGameplay && imu->isAvailable();
}

bool HardwareController::isIMUAvailable() const {
  return imu->isAvailable();
}

