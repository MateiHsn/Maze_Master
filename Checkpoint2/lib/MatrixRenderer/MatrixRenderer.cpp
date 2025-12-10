#include <MatrixRenderer.hpp>

MatrixRenderer::MatrixRenderer()
  : lc(MAX7219_DIN_PIN, MAX7219_CLK_PIN, MAX7219_LOAD_PIN, 1) {
  lastState = GameState::SPLASH_ART;
  lastMenuOption = MenuOption::MENU_OPTIONS_COUNT;
  lastAnimationTime = 0;
  animationFrame = 0;
  for(uint8_t i = 0; i < matrixSize; i ++) displayBuffer[i] = 0;
}

void MatrixRenderer::init() {
  lc.shutdown(deviceIndex, false);
  lc.setIntensity(deviceIndex, 15);
  clear();
}

void MatrixRenderer::updateDisplay() {
  for(uint8_t row = 0; row < matrixSize; row++) {
    lc.setRow(deviceIndex, row, displayBuffer[row]);
  }
}

void MatrixRenderer::clear() {
  lc.clearDisplay(deviceIndex);
  for(uint8_t i = 0; i < matrixSize; i ++) displayBuffer[i] = 0;
}

void MatrixRenderer::setPixel(uint8_t x, uint8_t y, bool state) {
  if( x >= matrixSize || x < 0 || y >= matrixSize || y < 0) return;

  if(state) {
    displayBuffer[y] |= (1 << (7 - x));
  } else {
    displayBuffer[y] &= !(1 << (7 - x));
  }
}

void MatrixRenderer::drawBuffer(const uint8_t * buffer) {
  for(uint8_t i = 0; i < matrixSize; i ++) {
    displayBuffer[i] = buffer[i];
  }
  updateDisplay();
}

void MatrixRenderer::setBrightness(uint8_t intensity) {
  if(intensity > 15) intensity = 15;
  if(intensity < 0) intensity = 0;
  
  lc.setIntensity(deviceIndex, intensity);
}

void MatrixRenderer::render(const GameModel & model) {
  GameState currentState = model.getState();

  if(currentState != lastState) {
    clear();
    lastState = currentState;
    animationFrame = 0;
  }

  switch (currentState) {
    case GameState::SPLASH_ART: 
      renderSplashScreen();
    break;
    case GameState::MENU:
      renderMenu(model);
    break;
    case GameState::PLAYING:
      renderGameplay(model);
    break;
    case GameState::PAUSED:
      renderPaused();
    break;
    case GameState::GAME_OVER:
      renderGameOver();
    break;
    case GameState::VICTORY:
      renderVictory();
    break;
  }
}

void MatrixRenderer::renderSplashScreen() {
  drawSplashAnimation();
}

void MatrixRenderer::drawSplashAnimation() {
  uint32_t currentTime = millis();
  if(currentTime - lastAnimationTime > animationFrameTime) {
    animationFrame = (animationFrame + 1) % 4;
    lastAnimationTime = currentTime;
    switch(animationFrame){
      case 0: break;
      case 1: break;
      case 2: break;
      case 3: break;
    }
  }
}

void MatrixRenderer::renderMenu(const GameModel & model) {
  MenuOption current = model.getSelectedMenuOption();
  if(current != lastMenuOption) {
    clear();
    lastMenuOption = current;
  }

  switch(current){
    default: break;
  }
}

void MatrixRenderer::drawStartGameSymbol() {

}

void MatrixRenderer::drawHighScoreSymbol() {

}

void MatrixRenderer::drawOptionSymbol() {

}

void MatrixRenderer::drawAboutSymbol() {

}

void MatrixRenderer::renderGameplay(const GameModel & model) {
  renderViewport(model, matrixSize);
}

void MatrixRenderer::renderViewport(const GameModel & model, uint8_t viewportSize) {
  clear();

  const Player & player = model.getPlayer();
  const Level & currentLevel = model.getCurrentLevel();
  uint8_t levelDim = currentLevel.dimension;

  int16_t offsetX = player.column - (viewportSize / 2);
  int16_t offsetY = player.row - (viewportSize / 2);

  if(offsetX < 0) {
    offsetX = 0;
  }
  
  if(offsetY < 0) {
    offsetY = 0;
  }

  if(offsetX + viewportSize > levelDim) {
    offsetX = levelDim - viewportSize;
  } 

  if(offsetY + viewportSize > levelDim) {
    offsetY = levelDim - viewportSize;
  }

  for(uint8_t y = 0; y < viewportSize; y ++){
    for(uint8_t x = 0; x < viewportSize; x ++) {
      uint8_t worldX = offsetX + x;
      uint8_t worldY = offsetY + y;
    
      if(worldX >= levelDim || worldY >= levelDim) return;

      if(worldX == player.column && worldY == player.row) {
        setPixel(x, y, true);
        continue;
      }
      
      if(currentLevel.rows != nullptr && worldY < levelDim) {
        bool isWall = (currentLevel.rows[worldY] & (1 << (15 - worldX))) != 0;
        setPixel(x, y, isWall);
      }
    }
  }

  updateDisplay();

}

void MatrixRenderer::renderPaused() {

}

void MatrixRenderer::renderGameOver() {

}

void MatrixRenderer::renderVictory() {

}