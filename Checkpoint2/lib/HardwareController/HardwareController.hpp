#ifndef HARDWARE_CONTROLLER_HPP
#define HARDWARE_CONTROLLER_HPP

#include <JoystickController.hpp>
#include <IMUController.hpp>
#include <AudioController.hpp>

struct InputState {
  bool upPressed;
  bool downPressed;
  bool leftPressed;
  bool rightPressed;
  bool buttonPressed;
  bool buttonJustPressed;

  InputState() : upPressed(false), downPressed(false), leftPressed(false), rightPressed(false), buttonPressed(false), buttonJustPressed(false) {}

};

class HardwareController {
private:
  JoystickController * joystick;
  IMUController * imu;
  AudioController * audio;

  InputState currentInput;
  bool useIMUForGameplay;

  uint32_t lastMoveTime;
  static const uint32_t moveCooldown = 200;

public:
  HardwareController(JoystickController * joy, IMUController * imuSensor, AudioController * audioCtrl);

  void init();
  void update();

  const InputState & getInput() const;
  bool canMove() const;
  void resetMoveCooldown();

  void playMenuMove();
  void playMenuSelect();
  void playCollectItem();
  void playPlayerDeath();
  void playLevelComplete();
  void playGameOver();
  void playVictory();
  void playStartupSound();

  void setIMUMode(bool enabled);
  bool isIMUMode() const;
  bool isIMUAvailable() const;
};

#endif