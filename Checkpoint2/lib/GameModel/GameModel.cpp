#include <EEPROM.h>
#include <Defaults.hpp>
#include <GameModel.hpp>

GameModel::GameModel() {
  setState(GameState::SPLASH_ART);
  selectedMenuOption = MenuOption::START_GAME;
  score = 0;
  levelStartTime = 0;
  currentLevelIndex = 0;

  for(uint8_t i = 0; i < highscoreCount; i ++) { 
    highscores[i] = 0;
  }

  player.column = 1;
  player.row = 1;
  player.isAlive = true;

  initializeLevels();
  loadHighScoreFromEEPROM(eepromAddress);
}

void GameModel::initializeLevels() {
  // Level 1 (8x8 tutorial level)
  static uint16_t level1Data[16] = {
    0b1111111100000000,
    0b1000000100000000,
    0b1000000100000000,
    0b1000000100000000,
    0b1000000100000000,
    0b1000000100000000,
    0b1000000100000000,
    0b1111111100000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
  };
  levels[0].rows = level1Data;
  levels[0].starsPerRoom = 3;
  levels[0].starsCollected = 0;
  levels[0].dimension = 8;

  // Level 2 (12x12 with enemies)
  static uint16_t level2Data[16] = {
    0b1111111111110000,
    0b1000000001110000,
    0b1111000001110000,
    0b1000000111110000,
    0b1001100000010000,
    0b1001100011110000,
    0b1000000000010000,
    0b1001111100010000,
    0b1001111100010000,
    0b1001111100010000,
    0b1000000000010000,
    0b1111111111110000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
  };
  levels[1].rows = level2Data;
  levels[1].starsPerRoom = 6;
  levels[1].starsCollected = 0;
  levels[1].dimension = 12;

  // Level 3 (16x16 full map)
  static uint16_t level3Data[16] = {
    0b1111111111111111,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1000000000000001,
    0b1111111111111111
  };
  levels[2].rows = level3Data;
  levels[2].starsPerRoom = 10;
  levels[2].starsCollected = 0;
  levels[2].dimension = 16;

  for(uint8_t level = 0; level < totalLevels; level++) {
    for(uint8_t row = 0; row < maxDimension; row++) {
      for(uint8_t col = 0; col < maxDimension; col++) {
        if(levels[level].rows != nullptr && row < 16) {
          bool isWall = (levels[level].rows[row] & (1 << (15 - col))) != 0;
          levelState[level][row][col] = isWall ? EntityType::WALL : EntityType::EMPTY;
        } else {
          levelState[level][row][col] = EntityType::EMPTY;
        }
      }
    }
  }
}

void GameModel::resetPlayerToLevelStart() {
  bool found = false;

  for(uint8_t row = 1; row < maxDimension && !found; row ++) {
    for(uint8_t col = 1; col < maxDimension && !found; col ++) {
      if(levelState[currentLevelIndex][row][col] == EntityType::PLAYER) {
        player.column = col;
        player.row = row;
        levelState[currentLevelIndex][row][col] = EntityType::EMPTY;
        found = true;
      }
    }
  }

  if(!found) {
    player.column = 1;
    player.row = 1;
  }
  player.isAlive = true;
}

GameState GameModel::getState() const {
  return currentState;
}

void GameModel::setState(GameState newState) {
  currentState = newState;
}

MenuOption GameModel::getSelectedMenuOption() const {
  return selectedMenuOption;
}

void GameModel::selectNextMenuOption() {
  selectedMenuOption = (MenuOption)((selectedMenuOption + 1) % MenuOption::MENU_OPTIONS_COUNT);
}

void GameModel::selectPreviousMenuOption() {
  if(selectedMenuOption == MenuOption::START_GAME) {
    selectedMenuOption = (MenuOption)(MenuOption::MENU_OPTIONS_COUNT - 1);
  } else {
    selectedMenuOption = (MenuOption)(selectedMenuOption - 1);
  }
}

void GameModel::confirmMenuSelection() {
  switch(selectedMenuOption) {
    case MenuOption::START_GAME:
      startNewGame();
    break;
    case MenuOption::HIGH_SCORES:
      displayHighscores();
    break;
    case MenuOption::OPTIONS:
      displaySettings();
    break;
    case MenuOption::ABOUT:
      displayAbout();
    break;
    default:
    break;
  }
}

void GameModel::startNewGame() {
  setState(GameState::PLAYING);
  currentLevelIndex = 0;
  score = 0;

  for(uint8_t i = 0; i < totalLevels; i ++) {
    levels[i].starsCollected = 0;
  }

  resetPlayerToLevelStart();
  startRoomTimer();
}

void GameModel::resetGame() {
  setState(GameState::MENU);
  selectedMenuOption = MenuOption::START_GAME;
  currentLevelIndex = 0;
  score = 0;
  levelStartTime = 0;
}

const Player& GameModel::getPlayer() const {
  return player;
}

bool GameModel::movePlayer(uint16_t deltaColumn, uint16_t deltaRow) {
  if(!player.isAlive) return false;

  int newCol = player.column + deltaColumn;
  int newRow = player.row + deltaRow;

  const Level & currentLevel = levels[currentLevelIndex];

  if(!currentLevel.isInBounds(newCol, newRow)) {
    return false;
  }

  if(checkWallInWay(newCol, newRow)) {
    
    return false;
  
  }

  player.column = newCol;
  player.row = newRow;

  if(checkEnemyAt(player.column, player.row)) {
    killPlayer();
    return true;
  }

  collectStarAt(player.column, player.row);

  if(checkExitAt(player.column, player.row) && isCurrentLevelCleared()) {
    advanceToNextLevel();
  }

  return true;
}

void GameModel::killPlayer() {
  player.isAlive = false;
  setGameOver();
}

byte GameModel::getCurrentLevelIndex() const {
  return currentLevelIndex;
}

const Level& GameModel::getCurrentLevel() const {
  return levels[currentLevelIndex];
}

bool GameModel::isCurrentLevelCleared() const {
  return levels[currentLevelIndex].starsCollected >= levels[currentLevelIndex].starsPerRoom;
}

void GameModel::advanceToNextLevel() {
  calculateLevelClearBonus();
  currentLevelIndex++;

  if(isGameCompleted()) {
    setVictory();
  } else {
    resetPlayerToLevelStart();
    startRoomTimer();
  }
}

bool GameModel::isGameCompleted() const {
  return currentLevelIndex >= totalLevels;
}

bool GameModel::collectStarAt(uint16_t column, uint16_t row) {
  if(levelState[currentLevelIndex][row][column] == EntityType::STAR) {
    levelState[currentLevelIndex][row][column] = EntityType::EMPTY;
    levels[currentLevelIndex].starsCollected++;
    addScore(pointsPerStar);
    return true;
  }
  return false;
}

bool GameModel::checkEnemyAt(uint16_t column, uint16_t row) {
  return levelState[currentLevelIndex][row][column] == EntityType::ENEMY;
}

bool GameModel::checkExitAt(uint16_t column, uint16_t row) {
  return levelState[currentLevelIndex][row][column] == EntityType::EXIT;
}

bool GameModel::checkWallInWay(uint16_t column, uint16_t row) {
  return levelState[currentLevelIndex][row][column] == EntityType::WALL;
}

uint32_t GameModel::getScore() const {
  return score;
}

void GameModel::addScore(uint32_t points) {
  score += points;
}

void GameModel::calculateLevelClearBonus() {
  uint32_t timeBonus = 0;
  uint32_t elapsedTime = (millis() - levelStartTime) / 1000;
  
  if(elapsedTime < 30) {
    timeBonus = 50;
  } else if(elapsedTime < 60) {
    timeBonus = 25;
  }

  addScore(baseLevelClearPoints + timeBonus);
}

uint32_t GameModel::getLevelStartTime() const {
  return levelStartTime;
}

void GameModel::startRoomTimer() {
  levelStartTime = millis();
}

const uint32_t* GameModel::getHighScores() const {
  return highscores;
}

bool GameModel::isNewHighscore(uint32_t newScore) const {
  return newScore > highscores[highscoreCount - 1];
}

void GameModel::addHighScore(uint32_t newScore) {
  if(!isNewHighscore(newScore)) return;

  for(uint8_t i = 0; i < highscoreCount; i++) {
    if(newScore > highscores[i]) {
      for(uint8_t j = highscoreCount - 1; j > i; j--) {
        highscores[j] = highscores[j - 1];
      }
      highscores[i] = newScore;
      break;
    }
  }

  saveHighScoresFromEEPROM(0);
}

uint8_t GameModel::calculateChecksum(const uint32_t* scores) const {
  uint8_t checksum = 0;
  for(uint8_t i = 0; i < highscoreCount; i++) {
    checksum ^= (scores[i] & 0xFF);
    checksum ^= ((scores[i] >> 8) & 0xFF);
    checksum ^= ((scores[i] >> 16) & 0xFF);
    checksum ^= ((scores[i] >> 24) & 0xFF);
  }
  return checksum;
}

void GameModel::loadHighScoreFromEEPROM(int eepromAddress) {
  HighScoreData data;
  EEPROM.get(eepromAddress, data);

  uint8_t checksum = calculateChecksum(data.scores);
  
  if(checksum == data.checksum) {
    for(uint8_t i = 0; i < highscoreCount; i++) {
      highscores[i] = data.scores[i];
    }
  } else {
    resetHighScores();
  }
}

void GameModel::saveHighScoresFromEEPROM(int eepromAddress) const {
  HighScoreData data;
  for(uint8_t i = 0; i < highscoreCount; i++) {
    data.scores[i] = highscores[i];
  }
  data.checksum = calculateChecksum(data.scores);
  
  EEPROM.put(eepromAddress, data);
}

void GameModel::resetHighScores() {
  for(uint8_t i = 0; i < highscoreCount; i++) {
    highscores[i] = 0;
  }
  saveHighScoresFromEEPROM(0);
}

void GameModel::displayHighscores() {
  currentState = GameState::HIGH_SCORES;
}

void GameModel::displaySettings() {
  currentState = GameState::SETTINGS;
}

void GameModel::displayAbout() {
  currentState = GameState::ABOUT;
}

void GameModel::setVictory() {
  currentState = GameState::VICTORY;
  addHighScore(score);
}

void GameModel::setGameOver() {
  currentState = GameState::GAME_OVER;
  addHighScore(score);
}