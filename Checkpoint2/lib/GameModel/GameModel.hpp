#ifndef GAME_MODEL_HPP
#define GAME_MODEL_HPP

#include <Arduino.h>

enum class GameState {
  SPLASH_ART,
  MENU,
  HIGH_SCORES,
  SETTINGS,
  ABOUT,
  PLAYING,
  PAUSED,
  GAME_OVER,
  VICTORY
};

enum MenuOption {
  START_GAME,
  HIGH_SCORES,
  OPTIONS,
  ABOUT,
  MENU_OPTIONS_COUNT
};

enum class EntityType {
  EMPTY = ' ',
  PLAYER = 2,
  ENEMY,
  EXIT,
  STAR,
  WALL,
  ENTITY_TYPE_COUNT
};

struct Player {
  uint16_t column;
  uint16_t row;
  bool isAlive;
};

struct HighScoreData {
  uint32_t scores[3];
  byte checksum;
};

struct Level {
  uint16_t * rows;
  uint8_t starsPerRoom;
  uint8_t starsCollected;
  uint8_t dimension;

  bool isInBounds(int16_t col, int16_t row) const {
    return col >= 0 && col < dimension && row >= 0 && row < dimension;
  }

  uint8_t getMaxBitPosition() const {
    return 16 - dimension;
  }
};

class GameModel {
private:
  GameState currentState;
  MenuOption selectedMenuOption;

  Player player;

  static const uint8_t maxEntityPosition = 15;
  static const uint8_t minEntityPosition = 0;
  static const uint8_t level1Dimension = 8;
  static const uint8_t level2Dimension = 12;
  static const uint8_t maxDimension = 16;
  

  static const uint8_t totalLevels = 3;
  Level levels[totalLevels];
  uint8_t currentLevelIndex;

  EntityType levelState[totalLevels][maxDimension][maxDimension];

  uint32_t score;
  uint32_t levelStartTime;
  static const uint8_t pointsPerStar = 10;
  static const uint8_t baseLevelClearPoints = 100;

  static const uint8_t highscoreCount = 3;
  uint32_t highscores[highscoreCount];

  void initializeLevels();

public:
  GameModel();
  
  GameState getState() const;
  void setState(GameState newState);

  MenuOption getSelectedMenuOption() const;
  void selectNextMenuOption();
  void selectPreviousMenuOption();
  void confirmMenuSelection();

  void startNewGame();
  void resetGame();

  const Player& getPlayer() const;
  bool movePlayer(uint16_t deltaColumn, uint16_t deltaRow);
  void killPlayer();
  
  byte getCurrentLevelIndex() const;
  const Level & getCurrentLevel() const;
  bool isCurrentLevelCleared() const;
  void advanceToNextLevel();
  bool isGameCompleted() const;

  bool collectStarAt(uint16_t column, uint16_t row);
  bool checkEnemyAt(uint16_t column, uint16_t row);
  bool checkExitAt(uint16_t column, uint16_t row);
  bool checkWallInWay(uint16_t column, uint16_t row);
  void resetPlayerToLevelStart();

  uint32_t getScore() const;
  void addScore(uint32_t points);
  void calculateLevelClearBonus();
  uint32_t getLevelStartTime() const;
  void startRoomTimer();

  const uint32_t * getHighScores() const;
  bool isNewHighscore(uint32_t newScore) const;
  void addHighScore(uint32_t newScore);
  uint8_t calculateChecksum(const uint32_t * scores) const;
  void loadHighScoreFromEEPROM(int eepromAddress);
  void saveHighScoresFromEEPROM(int eepromAddress) const;
  void resetHighScores();
  void displayHighscores();

  void displaySettings();
  void displayAbout();
  
  void setVictory();
  void setGameOver();
};

#endif