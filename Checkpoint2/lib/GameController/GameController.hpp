#ifndef GAME_CONTROLLER_HPP
#define GAME_CONTROLLER_HPP

#include <Defaults.hpp>
#include <GameModel.hpp>
#include <HardwareController.hpp>
#include <IRenderer.hpp>

enum class SettingsOption {
  BRIGHTNESS,
  BACK_TO_MENU,
  SETTINGS_OPTION_COUNT
};

class GameController {
private:
  GameModel * model;
  HardwareController * hardware;
  IRenderer ** renderers;

  uint8_t rendererCount;
  uint32_t lastUpdateTime;
  static const uint32_t updateInterval = 50;

  uint8_t highscoreScrollIndex;
  uint8_t settingsSelectedOption;
  uint8_t matrixBrightness;

  void updateSplashScreen();
  void updateMenu();
  void updateHighscores();
  void updateSettings();
  void updateAbout();
  void updateGameplay();
  void updatePaused();
  void updateGameOver();
  void updateVictory();
  void processMenuInput();
  void processGameplayInput();
  void handlePlayerMovement();
  void checkGameEvents();

public:
  GameController(GameModel * gameModel, HardwareController * hw);

  void init();
  void update();
  void addRenderer(IRenderer * renderer);
  void renderAll();
  GameModel * getModel() const { return model; }
  HardwareController * getHardware() const { return hardware; }
};

#endif