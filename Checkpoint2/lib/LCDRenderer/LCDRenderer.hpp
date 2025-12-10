#ifndef LCD_RENDERER_HPP
#define LCD_RENDERER_HPP

#include <Defaults.hpp>
#include <IRenderer.hpp>

class LCDRenderer : public IRenderer{ 
private:
  LiquidCrystal lcd;
  static const uint8_t lcdCols = 16;
  static const uint8_t lcdRows = 2;

  GameState lastState;
  MenuOption lastMenuOption;
  
  uint32_t lastScore;
  uint8_t lastLevel;
  uint8_t lastStars;
  uint32_t lastUpdateTime;
  static const uint32_t updateInterval = 100;

  void renderSplashScreen();
  void renderMenu(const GameModel & model);
  void renderGameplay(const GameModel & model);
  void renderPaused();
  void renderGameOver(const GameModel & model);
  void renderVictory(const GameModel & model);
  void displayScore(uint32_t score);
  void displayLevelInfo(uint8_t level, uint8_t cupsCollected, uint8_t totalCups);

public:
  LCDRenderer();
  void init() override;
  void render(const GameModel & model) override;
  void clear() override;
};

#endif