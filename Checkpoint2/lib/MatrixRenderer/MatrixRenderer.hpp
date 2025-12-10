#ifndef MATRIX_RENDERER_HPP
#define MATRIX_RENDERER_HPP

#include <Defaults.hpp>
#include <IRenderer.hpp>

const uint8_t animationFrameTime = 200;

class MatrixRenderer : public IRenderer {
private:
  LedControl lc;
  static const uint8_t matrixSize = 8;
  static const uint8_t deviceIndex = 0;

  uint8_t displayBuffer[matrixSize];
  GameState lastState;
  MenuOption lastMenuOption;
  uint32_t lastAnimationTime;
  uint8_t animationFrame;

  void updateDisplay();
  void drawBuffer(const uint8_t * buffer);
  void setPixel(uint8_t x, uint8_t y, bool state);

  void renderSplashScreen();
  void renderMenu(const GameModel & model);
  void renderGameplay(const GameModel & model);
  void renderPaused();
  void renderGameOver();
  void renderVictory();

  void drawStartGameSymbol();
  void drawHighScoreSymbol();
  void drawOptionSymbol();
  void drawAboutSymbol();
  void drawSplashAnimation();
  void renderViewport(const GameModel & model, uint8_t viewportSize);

public:
  MatrixRenderer();
  void init() override;
  void render(const GameModel & model) override;
  void clear() override;
  void setBrightness(uint8_t intensity);
};

#endif