#include <LCDRenderer.hpp>

LCDRenderer::LCDRenderer()
  : lcd(LCD_RS_PIN, LCD_EN_PIN, LCD_D4_PIN, LCD_D5_PIN, LCD_D6_PIN, LCD_D7_PIN) { 
  lastState = GameState::SPLASH_ART;
  lastMenuOption = MenuOption::MENU_OPTIONS_COUNT;
  lastScore = 0;
  lastLevel = 0;
  lastStars = 0;
  lastUpdateTime = 0;
}

void LCDRenderer::init() {
  lcd.begin(lcdCols, lcdRows);
}

void LCDRenderer::clear() {
  lcd.clear();
}

void LCDRenderer::render(const GameModel & model) {
  GameState currentState = model.getState();

  if(currentState != lastState) {
    clear();
    lastState = currentState;
  }

  switch(currentState) {
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
      renderGameOver(model);
    break;
    case GameState::VICTORY:
      renderVictory(model);
    break;
  }
}

void LCDRenderer::renderSplashScreen() {
  static bool rendered = false;
  if(!rendered) {
    lcd.setCursor(0, 0);
    lcd.print(" MAZE RUNNER ");
    lcd.setCursor(0, 1);
    lcd.print(" PRESS TO START");
    rendered = true;
  }
}

void LCDRenderer::renderMenu(const GameModel & model) {
  MenuOption current = model.getSelectedMenuOption();
  if(current != lastMenuOption) {
    clear();
    lastMenuOption = current;
    lcd.setCursor(0, 0);
    lcd.print("MENU:           ");
    lcd.setCursor(0, 1);
    switch(current) {
      case MenuOption::START_GAME:
        lcd.print(">Start Game     ");
      break;
      case MenuOption::HIGH_SCORES:
        lcd.print(">High Scores    ");
      break;
      case MenuOption::OPTIONS:
        lcd.print(">Options        ");
      break;
      case MenuOption::ABOUT:
        lcd.print(">About          ");
      break;
      default:
      break;
    }
  }
}

void LCDRenderer::renderGameplay(const GameModel & model) {
  uint32_t currentTime = millis();
  uint32_t score = model.getScore();
  uint8_t level = model.getCurrentLevelIndex() + 1;
  uint8_t stars = model.getCurrentLevel().starsCollected;
  uint8_t totalStars = model.getCurrentLevel().starsPerRoom;

  bool shouldUpdate = (currentTime - lastUpdateTime >= updateInterval) || (score != lastScore) || (level != lastLevel) || (stars != lastStars);

  if(shouldUpdate) {
    lcd.setCursor(0, 0);
    lcd.print("Score: ");
    displayScore(score);
    displayLevelInfo(level, stars, totalStars);
    lcd.setCursor(12, 1);
    lcd.print(model.getPlayer().isAlive ? "OK  " : "DEAD");
    lastUpdateTime = currentTime;
    lastScore = score;
    lastLevel = level;
    lastStars = stars;
  }
}

void LCDRenderer::renderPaused() {
  static bool rendered = false;

  if(!rendered) {
    lcd.setCursor(0, 0);
    lcd.print("    PAUSED      ");
    lcd.setCursor(0, 1);
    lcd.print("Press to Resume ");
    rendered = true;
  }
}

void LCDRenderer::renderGameOver(const GameModel & model) {
  static bool rendered = false;
  if(!rendered) {
    lcd.setCursor(0, 0);
    lcd.print(model.isNewHighscore(model.getScore()) ? "NEW HIGH SCORE! " : "  GAME OVER!    ");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(model.getScore());
    rendered = true;
  }
}

void LCDRenderer::renderVictory(const GameModel & model) {
  static bool rendered = false;
  if (!rendered) {
    lcd.setCursor(0, 0);
    lcd.print(model.isNewHighscore(model.getScore()) ? "NEW HIGH SCORE! " : "   YOU WON!!!   ");
    lcd.setCursor(0, 1);
    lcd.print("Score: ");
    lcd.print(model.getScore());
    rendered = true;
  }
}

void LCDRenderer::displayScore(uint32_t score) {
  lcd.print(score);
  lcd.print("    ");
}

void LCDRenderer::displayLevelInfo(uint8_t level, uint8_t starsCollected, uint8_t totalStars) {
  lcd.setCursor(0, 1);
  lcd.print("L:");
  lcd.print(level);
  lcd.print(" C:");
  lcd.print(starsCollected);
  lcd.print("/");
  lcd.print(totalStars);
  lcd.print("  ");
}