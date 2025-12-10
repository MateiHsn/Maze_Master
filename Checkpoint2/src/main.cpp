#include <Defaults.hpp>
#include <GameController.hpp>
#include <LCDRenderer.hpp>
#include <MatrixRenderer.hpp>

/*
de facut:
tot ce e animatie sau simbol pe matrice
*/

GameModel gameModel;
JoystickController joystick;
IMUController imu;
AudioController audio;
HardwareController hardwareController(& joystick, & imu, & audio);
GameController gameController (& gameModel, & hardwareController);
LCDRenderer lcdRenderer;
MatrixRenderer matrixRenderer;

void setup() {
  gameController.init();
  gameController.addRenderer(& lcdRenderer);
  gameController.addRenderer(& matrixRenderer);
  matrixRenderer.setBrightness(8);
}

void loop() {
  gameController.update();
}