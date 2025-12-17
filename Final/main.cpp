#include <Arduino.h>
#include <LiquidCrystal.h>
#include <LedControl.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <avr/pgmspace.h>


// Pins
const uint8_t PIN_JOY_BTN = 2;
const uint8_t PIN_BUZZER = 3;
const uint8_t PIN_LCD_D4 = 4;
const uint8_t PIN_LCD_D5 = 5;
const uint8_t PIN_LCD_D6 = 6;
const uint8_t PIN_LCD_D7 = 7;
const uint8_t PIN_LCD_RS = 8;
const uint8_t PIN_LCD_EN = 9;
const uint8_t PIN_LCD_BACKLIGHT = 10;
const uint8_t PIN_MATRIX_LOAD = 11;
const uint8_t PIN_MATRIX_CLK = 12;
const uint8_t PIN_MATRIX_DIN = 13;
const uint8_t PIN_RANDOM_SEED = A0;
const uint8_t PIN_JOY_X = A1;
const uint8_t PIN_JOY_Y = A2;

// EEPROM Addresses
const uint16_t eepromAddressSettingsStart = 0;
const uint16_t eepromOffsetLCDBrightness = 0;
const uint16_t eepromOffsetMatrixBrightness = 1;
const uint16_t eepromOffsetSound = 2;
const uint16_t eepromOffsetIMU = 3;
const uint16_t eepromAddressHighscores = 20; // Start high scores later

// Game Constants
const uint8_t maxNameLength = 3;
const uint8_t highScoreCount = 3;
const uint8_t totalLevels = 3;
const uint8_t matrixSize = 8;
const uint8_t maxLevelEntities = 10;
const uint16_t pointsPerStar = 10;
const uint16_t baseLevelClearPoints = 6000;
const uint16_t timeBonusDeduction = 100; // Points lost per second
const uint8_t minStartDist = 3;
const uint8_t minExitDist = 2;

// Input Constants
const uint16_t joyCenterMin = 400;
const uint16_t joyCenterMax = 600;
const uint32_t debounceDelay = 50;
const uint32_t moveCooldown = 200;
const uint32_t menuMoveCooldown = 250;
const int16_t imuTiltThreshold = 3;
const uint32_t imuReadInterval = 100;
// const uint32_t imuReinitializeInterval = 20000;
// uint32_t imuRunningTime = 0;

// Brightness Constants (User scale 1-10)
const uint8_t brightnessMinUser = 1;
const uint8_t brightnessMaxUser = 10;

// Entity Types
const uint8_t ENTITY_EMPTY = 0;
const uint8_t ENTITY_PLAYER = 1;
const uint8_t ENTITY_EXIT = 2;
const uint8_t ENTITY_STAR = 3;
const uint8_t ENTITY_WALL = 4;

// Directions
const uint8_t DIR_NONE = 0;
const uint8_t DIR_UP = 1;
const uint8_t DIR_DOWN = 2;
const uint8_t DIR_LEFT = 3;
const uint8_t DIR_RIGHT = 4;


enum GameState {
  STATE_INTRO,
  STATE_MENU_MAIN,
  STATE_MENU_HIGHSCORES,
  STATE_MENU_SETTINGS,
  STATE_MENU_SETTINGS_LCD,
  STATE_MENU_SETTINGS_MATRIX,
  STATE_MENU_SETTINGS_SOUND,
  STATE_MENU_SETTINGS_IMU,
  STATE_MENU_SETTINGS_RESET_SCORES,
  STATE_MENU_SETTINGS_BACK,
  STATE_MENU_ABOUT,
  STATE_MENU_HOWTO,
  STATE_GAME_PLAYING,
  STATE_GAME_PAUSED,
  STATE_GAME_LEVEL_TRANSITION,
  STATE_GAME_VICTORY,
  STATE_NAME_ENTRY
};

// Menu Options
enum MainMenuOption {
  OPT_START = 0,
  OPT_HIGHSCORES,
  OPT_SETTINGS,
  OPT_ABOUT,
  OPT_HOWTO,
  MAIN_MENU_COUNT
};

enum SettingsOption {
  SET_LCD_BRIGHT = 0,
  SET_MATRIX_BRIGHT,
  SET_SOUND,
  SET_IMU,
  SET_RESET,
  SET_BACK,
  SETTINGS_COUNT
};

// Data Structs
struct HighScoreEntry {
  char name[maxNameLength + 1]; // +1 for null terminator
  uint16_t score;
};

struct ToneSequence {
  uint16_t frequency;
  uint16_t duration;
};

struct Entity {
  uint8_t col;
  uint8_t row;
  uint8_t type;
};


// Hardware Objects
LiquidCrystal lcd(PIN_LCD_RS, PIN_LCD_EN, PIN_LCD_D4, PIN_LCD_D5, PIN_LCD_D6, PIN_LCD_D7);
LedControl lc = LedControl(PIN_MATRIX_DIN, PIN_MATRIX_CLK, PIN_MATRIX_LOAD, 1);
Adafruit_MPU6050 mpu;

// Settings
uint8_t settingLCDBrightnessUser = 10; // 1-10 scale
uint8_t settingMatrixBrightnessUser = 5; // 1-10 scale
uint8_t lcdPWMOutputMin = 25;
uint8_t lcdPWMOutputMax = 255;
uint8_t matrixBrightnessMin = 0;
uint8_t matrixBrightnessMax = 15;
bool settingSoundEnabled = true;
bool settingIMUEnabled = false;
bool imuHardwareAvailable = false;

// Input State
uint16_t joyXVal = 512;
uint16_t joyYVal = 512;
bool btnPressed = false;
bool btnJustPressed = false;
bool lastBtnState = HIGH;
uint32_t lastDebounceTime = 0;
uint32_t lastInputMoveTime = 0; // For menu navigation rate limiting
uint32_t backToMenuDelay = 100;
uint32_t backToMenuIssuedTime = 0;
uint32_t LCDupdateInteval = 500;
// Game State
GameState currentState = STATE_INTRO;
MainMenuOption selectedMainMenu = OPT_START;
SettingsOption selectedSetting = SET_LCD_BRIGHT;
uint8_t pausedSelectedOption = 0; // 0=Continue, 1=Exit

// Gameplay Variables
uint16_t playerCol = 0;
uint16_t playerRow = 0;
// bool playerAlive = true;
uint16_t currentScore = 0;
uint8_t currentLevelIndex = 0;
uint32_t levelStartTime = 0;
uint32_t lastGameMoveTime = 0; // For player movement cooldown
uint8_t maxAttempts = 100;

// Level Data (Loaded from PROGMEM to RAM for current level)
const uint16_t * currentLevelRows;
uint8_t currentLevelDim = 8;
uint8_t currentLevelStarsTotal = 0;
uint8_t currentLevelStarsCollected = 0;
uint8_t currentLevelStartCol = 0;
uint8_t currentLevelStartRow = 0;
uint8_t currentLevelExitCol = 0;
uint8_t currentLevelExitRow = 0;
Entity currentEntities[maxLevelEntities];
uint8_t currentEntityCount = 0;

// High Scores
HighScoreEntry highScores[highScoreCount];
char inputNameBuffer[maxNameLength + 1] = "AAA"; // For name entry

// Audio State
bool audioPlaying = false;
const ToneSequence* audioSequence = nullptr;
uint8_t audioTotalTones = 0;
uint8_t audioCurrentToneIndex = 0;
uint32_t audioToneStartTime = 0;
uint32_t audioToneDuration = 0;

// Display Buffers & Timers
uint8_t matrixBuffer[matrixSize];
uint32_t lastStarDisplayBlinkTime = 0;
uint32_t starBlinkPeriod = 300;
uint32_t playerBlinkPeriod = 150;
uint32_t lastPlayerDisplayBlinkTime = 0;
bool blinkStateStar = false;
bool blinkStatePlayer = false;

const uint16_t level1Data[16] PROGMEM = {
  0b1111111100000000,
  0b1000000100000000,
  0b1110000100000000,
  0b1000111100000000,
  0b1110000100000000,
  0b1000111100000000,
  0b1000000100000000,
  0b1111111100000000,
  0,
  0,
  0,
  0,
  0,
  0,
  0,
  0
};

const uint16_t level2Data[16] PROGMEM = {
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
  0,
  0,
  0,
  0
};

const uint16_t level3Data[16] PROGMEM = {
  0b1111111111111111,
  0b1000001111100001,
  0b1000000000000001,
  0b1011111000110001,
  0b1000000000110001,
  0b1000001111110001,
  0b1011111111110001,
  0b1011111111110001,
  0b1000001111000001,
  0b1001111100000001,
  0b1001111101111001,
  0b1001111101110001,
  0b1000000001110001,
  0b1011110001110001,
  0b1000000000000001,
  0b1111111111111111
};

const ToneSequence seqMenuMove[] PROGMEM = { {800, 50} };
const ToneSequence seqMenuSelect[] PROGMEM = { {1200, 100}, {1500, 150} };
const ToneSequence seqCollectStar[] PROGMEM = { {1000, 80}, {1200, 80} };
const ToneSequence seqLevelComplete[] PROGMEM = { {1000, 100}, {1200, 100}, {1500, 100}, {2000, 200} };
const ToneSequence seqVictory[] PROGMEM = { {1500, 100}, {1800, 100}, {2100, 100}, {2500, 300} };
const ToneSequence seqStartup[] PROGMEM = { {1000, 200}, {1500, 200}, {2000, 200} };


const uint8_t iconPlay[8] = {
  0b00100000,
  0b00110000,
  0b00111000,
  0b00111100,
  0b00111100,
  0b00111000,
  0b00110000,
  0b00100000
};
const uint8_t iconTrophy[8] = {
  0b01100110,
  0b10111101,
  0b10111101,
  0b01111110,
  0b00111100,
  0b00011000,
  0b00011000,
  0b00111100
};
const uint8_t iconSettings[8] = {
  0b00011100,
  0b00011000,
  0b00010001,
  0b00011011,
  0b00111111,
  0b01110000,
  0b11100000,
  0b11000000 };
const uint8_t iconInfo[8] = {
  0b00011000,
  0b00011000,
  0b00000000,
  0b00011000,
  0b00111000,
  0b00011000,
  0b00011000,
  0b00111100
};
const uint8_t iconQuestion[8] = {
  0b00011000,
  0b00111100,
  0b01100110,
  0b00001100,
  0b00011000,
  0b00000000,
  0b00011000,
  0b00011000
};

void applyLCDBrightness() {
  // Map 1-10 to PWM (approx 25 to 255)
  uint8_t pwmVal = map(settingLCDBrightnessUser, brightnessMinUser, brightnessMaxUser, lcdPWMOutputMin, lcdPWMOutputMax);
  analogWrite(PIN_LCD_BACKLIGHT, pwmVal);
}

void applyMatrixBrightness() {
  // Map 1-10 to 0-15
  uint8_t hwVal = map(settingMatrixBrightnessUser, brightnessMinUser, brightnessMaxUser, matrixBrightnessMin, matrixBrightnessMax);
  lc.setIntensity(0, hwVal);
}

void playSoundSequence(const ToneSequence* seq, uint8_t count) {
  if (!settingSoundEnabled) return;
  // Stop current
  noTone(PIN_BUZZER);
  audioPlaying = true;
  audioSequence = seq;
  audioTotalTones = count;
  audioCurrentToneIndex = 0;
  
  // Play first
  uint16_t freq = pgm_read_word(&(seq[0].frequency));
  uint16_t dur = pgm_read_word(&(seq[0].duration));
  tone(PIN_BUZZER, freq, dur);
  audioToneStartTime = millis();
  audioToneDuration = dur;
}

void updateAudio() {
  if (!audioPlaying || !settingSoundEnabled) return;
  
  if (millis() - audioToneStartTime >= audioToneDuration) {
    audioCurrentToneIndex++;
    if (audioCurrentToneIndex >= audioTotalTones) {
      audioPlaying = false;
      noTone(PIN_BUZZER);
    } else {
      uint16_t freq = pgm_read_word(&(audioSequence[audioCurrentToneIndex].frequency));
      uint16_t dur = pgm_read_word(&(audioSequence[audioCurrentToneIndex].duration));
      tone(PIN_BUZZER, freq, dur);
      audioToneStartTime = millis();
      audioToneDuration = dur;
    }
  }
}

void loadSettings() {
  uint8_t val = EEPROM.read(eepromAddressSettingsStart + eepromOffsetLCDBrightness);
  settingLCDBrightnessUser = constrain(val, brightnessMinUser, brightnessMaxUser);
  
  val = EEPROM.read(eepromAddressSettingsStart + eepromOffsetMatrixBrightness);
  settingMatrixBrightnessUser = constrain(val, brightnessMinUser, brightnessMaxUser);
  
  val = EEPROM.read(eepromAddressSettingsStart + eepromOffsetSound);
  settingSoundEnabled = (val == 1);

  val = EEPROM.read(eepromAddressSettingsStart + eepromOffsetIMU);
  settingIMUEnabled = (val == 1);
}

void saveSettings() {
  EEPROM.update(eepromAddressSettingsStart + eepromOffsetLCDBrightness, settingLCDBrightnessUser);
  EEPROM.update(eepromAddressSettingsStart + eepromOffsetMatrixBrightness, settingMatrixBrightnessUser);
  EEPROM.update(eepromAddressSettingsStart + eepromOffsetSound, settingSoundEnabled ? 1 : 0);
  EEPROM.update(eepromAddressSettingsStart + eepromOffsetIMU, settingIMUEnabled ? 1 : 0);
}

void loadHighScores() {
  uint16_t addr = eepromAddressHighscores;
  for (uint8_t i = 0; i < highScoreCount; i++) {
    EEPROM.get(addr, highScores[i]);
    // Validate characters
    for(uint8_t c = 0; c < maxNameLength; c++) {
      if(highScores[i].name[c] < 'A' || highScores[i].name[c] > 'Z' && highScores[i].score != 0) highScores[i].name[c] = '-';
    }
    highScores[i].name[maxNameLength] = '\0';
    addr += sizeof(HighScoreEntry);
  }
}

void saveHighScores() {
  uint16_t addr = eepromAddressHighscores;
  for (uint8_t i = 0; i < highScoreCount; i++) {
    EEPROM.put(addr, highScores[i]);
    addr += sizeof(HighScoreEntry);
  }
}

void resetHighScores() {
  for (uint8_t i = 0; i < highScoreCount; i++) {
    strcpy(highScores[i].name, "---");
    highScores[i].score = 0;
  }
  saveHighScores();
}

void readInputs() {
  joyXVal = analogRead(PIN_JOY_X);
  joyYVal = analogRead(PIN_JOY_Y);
  
  bool reading = (digitalRead(PIN_JOY_BTN) == LOW);
  if (reading != lastBtnState) {
    lastDebounceTime = millis();
  }
  
  btnJustPressed = false;
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != btnPressed) {
      btnPressed = reading;
      if (btnPressed) {
        btnJustPressed = true;
      }
    }
  }
  lastBtnState = reading;
}

bool isWall(uint8_t c, uint8_t r) {
  if (c >= currentLevelDim || r >= currentLevelDim) return true;
  
  // Read row word from PROGMEM
  uint16_t rowData = pgm_read_word(&(currentLevelRows[r]));
  // Check bit (MSB is column 0)
  return (rowData & (1 << (15 - c)));
}

// Randomly place stars ensuring no collision with walls, start, or exit
void placeEntities(uint8_t count) {
  currentEntityCount = 0;
  uint16_t attempts = 0;
  while (currentEntityCount < count && attempts < maxAttempts) {
    attempts++;
    uint8_t c = random(currentLevelDim);
    uint8_t r = random(currentLevelDim);
    
    // Check Walls
    if (isWall(c, r)) continue;
    
    // Check distances
    int8_t distStart = abs((int8_t)c - (int8_t)currentLevelStartCol) + abs((int8_t)r - (int8_t)currentLevelStartRow);
    int8_t distExit = abs((int8_t)c - (int8_t)currentLevelExitCol) + abs((int8_t)r - (int8_t)currentLevelExitRow);
    
    if (distStart < minStartDist || distExit < minExitDist) continue;
    
    // Check duplicates
    bool overlap = false;
    for(uint8_t i=0; i<currentEntityCount; i++) {
      if(currentEntities[i].col == c && currentEntities[i].row == r) {
        overlap = true; 
        break;
      }
    }
    if (overlap) continue;
    
    currentEntities[currentEntityCount].col = c;
    currentEntities[currentEntityCount].row = r;
    currentEntities[currentEntityCount].type = ENTITY_STAR;
    currentEntityCount++;
  }
}

void initLevels(uint8_t levelIdx) {
  currentLevelIndex = levelIdx;
  currentLevelStarsCollected = 0;
  
  if (levelIdx == 0) {
    currentLevelRows = level1Data;
    currentLevelDim = 8;
    currentLevelStarsTotal = 2;
    currentLevelStartCol = 1;
    currentLevelStartRow = 1;
    currentLevelExitCol = 6;
    currentLevelExitRow = currentLevelDim - 2;
  } else if (levelIdx == 1) {
    currentLevelRows = level2Data;
    currentLevelDim = 12;
    currentLevelStarsTotal = 6;
    currentLevelStartCol = 1;
    currentLevelStartRow = 1;
    currentLevelExitCol = 10;
    currentLevelExitRow = currentLevelDim - 2;
  } else {
    currentLevelRows = level3Data;
    currentLevelDim = 16;
    currentLevelStarsTotal = 10;
    currentLevelStartCol = 1;
    currentLevelStartRow = 1;
    currentLevelExitCol = 14;
    currentLevelExitRow = currentLevelDim - 2;
  }
  
  playerCol = currentLevelStartCol;
  playerRow = currentLevelStartRow;
  placeEntities(currentLevelStarsTotal);
  levelStartTime = millis();
}

void startGame() {
  currentScore = 0;
  initLevels(0);
  currentState = STATE_GAME_PLAYING;
  lcd.clear();
}

void updateMatrixViewport() {
  // Clear local buffer
  memset(matrixBuffer, 0, matrixSize);
  
  // Calculate viewport offset to center player
  uint8_t colOffset = 0;
  uint8_t rowOffset = 0;
  
  if (currentLevelDim > matrixSize) {
    colOffset = constrain((uint8_t)playerCol - matrixSize/2, 0, currentLevelDim - matrixSize);
    rowOffset = constrain((uint8_t)playerRow - matrixSize/2, 0, currentLevelDim - matrixSize);
  }
  
  for(uint8_t r = 0; r < matrixSize; r++) {
    uint8_t levelR = r + rowOffset;
    if (levelR < currentLevelDim) {
      uint16_t rowData = pgm_read_word(&(currentLevelRows[levelR]));
      for(uint8_t c = 0; c < matrixSize; c++) {
        uint8_t levelC = c + colOffset;
        if (levelC < currentLevelDim) {
           if (rowData & (1 << (15 - levelC))) {
             matrixBuffer[r] |= (1 << (7 - c));
           }
        }
      }
    }
  }
  
  if (blinkStateStar) {
    for(uint8_t i=0; i<currentEntityCount; i++) {
      if (currentEntities[i].type == ENTITY_STAR) {
        int8_t starCol = currentEntities[i].col - colOffset;
        int8_t starRow = currentEntities[i].row - rowOffset;
        if (starCol >= 0 && starCol < matrixSize && starRow >= 0 && starRow < matrixSize) {
          matrixBuffer[starRow] |= (1 << (7 - starCol));
        }
      }
    }
  }
  
  int8_t exitCol = currentLevelExitCol - colOffset;
  int8_t exitRow = currentLevelExitRow - rowOffset;
  if (exitCol >= 0 && exitCol < matrixSize && exitRow >= 0 && exitRow < matrixSize) {
    bool drawExit = (currentLevelStarsCollected >= currentLevelStarsTotal) ? blinkStateStar : true;
    if (drawExit) matrixBuffer[exitRow] |= (1 << (7 - exitCol));
  }
  
  if(blinkStatePlayer) { 
    int8_t playerC = playerCol - colOffset;
    int8_t playerR = playerRow - rowOffset;
    if (playerC >= 0 && playerC < matrixSize && playerR >= 0 && playerR < matrixSize) {
      matrixBuffer[playerR] |= (1 << (7 - playerC));
    }
  }  

  // Push to hardware
  for(uint8_t i = 0; i < matrixSize; i ++) {
    lc.setColumn(0, i, matrixBuffer[i]);
  }
}


void handleIntro() {
  static bool drawn = false;
  if (!drawn) {
    lcd.clear();
    lcd.setCursor(0, 0); lcd.print(F("  MAZE MASTER"));
    lcd.setCursor(0, 1); lcd.print(F(" Press Button "));
    
    // Draw Play Icon on Matrix
    lc.clearDisplay(0);
    for(uint8_t i = 0; i < matrixSize; i++) lc.setColumn(0, i, iconPlay[i]);
    
    playSoundSequence(seqStartup, 3);
    drawn = true;
  }
  
  if (btnJustPressed) {
    playSoundSequence(seqMenuSelect, 2);
    currentState = STATE_MENU_MAIN;
    drawn = false;
  }
}

void handleMainMenu() {
  static bool drawn = false;
  static MainMenuOption lastOpt = MAIN_MENU_COUNT; // force redraw
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyYVal < joyCenterMin) {
      selectedMainMenu = (MainMenuOption)((selectedMainMenu + 1) % MAIN_MENU_COUNT);
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    } else if (joyYVal > joyCenterMax) {
      selectedMainMenu = (MainMenuOption)((selectedMainMenu - 1 + MAIN_MENU_COUNT) % MAIN_MENU_COUNT);
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    }
  }
  
  if (lastOpt != selectedMainMenu || !drawn) {
    lcd.clear();
    lcd.print(F(">"));
    
    const uint8_t* icon = iconPlay;
    
    switch(selectedMainMenu) {
      case OPT_START: 
        lcd.print(F("Start Game")); 
        icon = iconPlay; 
        break;
      case OPT_HIGHSCORES: 
        lcd.print(F("High Scores")); 
        icon = iconTrophy; 
        break;
      case OPT_SETTINGS: 
        lcd.print(F("Settings")); 
        icon = iconSettings; 
        break;
      case OPT_ABOUT: 
        lcd.print(F("About")); 
        icon = iconInfo; 
        break;
      case OPT_HOWTO: 
        lcd.print(F("How to Play")); 
        icon = iconQuestion; 
        break;
    }
    
    lcd.setCursor(0, 1);
    lcd.print(F("Select: Button"));
    
    lc.clearDisplay(0);
    for(uint8_t i=0; i < matrixSize; i++) lc.setColumn(0, i, icon[i]);
    
    lastOpt = selectedMainMenu;
    drawn = true;
  }
  
  if (btnJustPressed) {
    playSoundSequence(seqMenuSelect, 2);
    drawn = false; // Reset for next state
    lastOpt = MAIN_MENU_COUNT;
    
    switch(selectedMainMenu) {
      case OPT_START: startGame(); break;
      case OPT_HIGHSCORES: currentState = STATE_MENU_HIGHSCORES; break;
      case OPT_SETTINGS: currentState = STATE_MENU_SETTINGS; break;
      case OPT_ABOUT: currentState = STATE_MENU_ABOUT; break;
      case OPT_HOWTO: currentState = STATE_MENU_HOWTO; break;
    }
  }
}

void handleSettingsMenu() {
  static bool drawn = false;
  static SettingsOption lastOpt = SETTINGS_COUNT;
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyYVal < joyCenterMin) {
      selectedSetting = (SettingsOption)((selectedSetting + 1) % SETTINGS_COUNT);
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    } else if (joyYVal > joyCenterMax) {
      selectedSetting = (SettingsOption)((selectedSetting - 1 + SETTINGS_COUNT) % SETTINGS_COUNT);
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    }
  }
  
  if (lastOpt != selectedSetting || !drawn) {
    lcd.clear();
    lcd.print(F(">"));
    switch(selectedSetting) {
      case SET_LCD_BRIGHT:
        lcd.print(F("LCD Brightness"));
        break;
      case SET_MATRIX_BRIGHT:
        lcd.print(F("Mat Brightness"));
        break;
      case SET_SOUND:
        lcd.print(F("Sound: "));
        lcd.print(settingSoundEnabled ? F("ON") : F("OFF"));
        break;
      case SET_IMU:
        lcd.print(F("IMU Ctr: "));
        lcd.print(settingIMUEnabled ? F("ON") : F("OFF"));
        break;
      case SET_RESET: 
        lcd.print(F("Reset Scores"));
        break;
      case SET_BACK:
        lcd.print(F("Back to Menu"));
        break;
    }
    lcd.setCursor(0, 1);
    lcd.print(F("Back: Hold Btn"));
    
    lc.clearDisplay(0);
    for(uint8_t i = 0; i< matrixSize; i ++) {
      lc.setColumn(0, i, iconSettings[i]);
    }
    lastOpt = selectedSetting;
    drawn = true;
  }
  
  if (btnJustPressed) {
     playSoundSequence(seqMenuSelect, 2);
     drawn = false;
     lastOpt = selectedSetting;
     
     switch(selectedSetting) {
      case SET_LCD_BRIGHT:
        currentState = STATE_MENU_SETTINGS_LCD;
        break;
      case SET_MATRIX_BRIGHT:
        currentState = STATE_MENU_SETTINGS_MATRIX;
        break;
      case SET_SOUND:
        currentState = STATE_MENU_SETTINGS_SOUND;
        break;
      case SET_IMU:
        currentState = STATE_MENU_SETTINGS_IMU;
        break;
      case SET_RESET:
        currentState = STATE_MENU_SETTINGS_RESET_SCORES;
        break;
      case SET_BACK:
        currentState = STATE_MENU_SETTINGS;
        break;
     }
  }
}

void handleSettingsValue(uint8_t type) {
  // Generic handler for LCD/Matrix slider
  // type 0 = LCD, 1 = Matrix
  static bool drawn = false;
  static int8_t lastVal = -1;
  
  uint8_t* targetVal = (type == SET_LCD_BRIGHT) ? &settingLCDBrightnessUser : &settingMatrixBrightnessUser;
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    bool changed = false;
    if (joyXVal < joyCenterMin && *targetVal > brightnessMinUser) {
      (*targetVal)--;
      changed = true;
    } else if (joyXVal > joyCenterMax && *targetVal < brightnessMaxUser) {
      (*targetVal)++;
      changed = true;
    }
    
    if (changed) {
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
      if (type == 0) applyLCDBrightness();
      else applyMatrixBrightness();
      saveSettings();
    }
  }
  
  if (!drawn || lastVal != *targetVal) {
    lcd.clear();
    lcd.print(type == 0 ? F("LCD Bright: ") : F("Mat Bright: "));
    lcd.print(*targetVal);
    
    // Draw bar
    lcd.setCursor(0, 1);
    uint8_t bars = map(*targetVal, 1, 10, 1, 16);
    for(uint8_t i=0; i < bars; i++) lcd.print(F("#"));
    
    lastVal = *targetVal;
    drawn = true;
  }
  
  if (btnJustPressed) {
    playSoundSequence(seqMenuSelect, 2);
    currentState = STATE_MENU_SETTINGS_BACK;
    drawn = false;
  }
}

void handleSettingsToggle(uint8_t type) {
  // type 0 = Sound, 1 = IMU
  static bool drawn = false;
  bool * target = (type == SET_SOUND) ? &settingSoundEnabled : &settingIMUEnabled;
  
  if (!drawn) {
    lcd.clear();
    lcd.print(type == SET_SOUND ? F("Sound: ") : F("IMU Control: "));
    lcd.print(*target ? F("ON") : F("OFF"));
    lcd.setCursor(0, 1);
    lcd.print(F("Move Joy to Flip"));
    drawn = true;
  }
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyXVal < joyCenterMin || joyXVal > joyCenterMax) {
       *target = !(*target);
       saveSettings();
       playSoundSequence(seqMenuMove, 1);
       lastInputMoveTime = millis();
       drawn = false; // redraw
    }
  }
  
  if (btnJustPressed) {
    playSoundSequence(seqMenuSelect, 2);
    currentState = STATE_MENU_SETTINGS;
    drawn = false;
  }
}

void handleSettingsReset() {
  static bool drawn = false;
  static bool confirm = false;
  
  if (!drawn) {
    lcd.clear();
    lcd.print(F("Reset Scores?"));
    lcd.setCursor(0, 1);
    lcd.print(confirm ? F("YES >NO") : F(">YES NO"));
    drawn = true;
  }
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyXVal < joyCenterMin || joyXVal > joyCenterMax) {
      confirm = !confirm;
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
      drawn = false;
    }
  }
  
  if (btnJustPressed) {
    if (confirm) { 
      // NO selected
      btnJustPressed = false;
      currentState = STATE_MENU_SETTINGS;
    } else {
      // YES selected
      resetHighScores();
      lcd.clear();
      lcd.print(F("Scores Reset!"));
      
      if(btnJustPressed){
        currentState = STATE_MENU_SETTINGS;
      }
    }
    playSoundSequence(seqMenuSelect, 2);
    drawn = false;
  }
}

void handleAbout() {
  static bool drawn = false;

  if (!drawn) {
    lcd.clear();
    lcd.print(F("Maze Runner v1"));
    lcd.setCursor(0, 1);
    lcd.print(F("By MateiHsn"));
    lc.clearDisplay(0);
    for(uint8_t i = 0; i < matrixSize; i++) {
      lc.setColumn(0, i, iconInfo[i]);
    }
    drawn = true;
  }
  if (btnJustPressed) {
    currentState = STATE_MENU_MAIN;
    drawn = false;
  }
}

void handleHowTo() {
  static int8_t page = 0;
  static bool drawn = false;
  
  if (!drawn) {
    lcd.clear();
    if (page == 0) {
      lcd.print(F("Collect Stars"));
      lcd.setCursor(0, 1);
      lcd.print(F("Reach the Exit"));
    } else {
      lcd.print(settingIMUEnabled ? F("Tilt to Move") : F("Joy to Move"));
    }
    lc.clearDisplay(0);
    for(uint8_t i=0; i < matrixSize; i++) lc.setColumn(0, i, iconQuestion[i]);
    drawn = true;
  }
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyXVal < joyCenterMin || joyXVal > joyCenterMax) {
      page = !page;
      drawn = false;
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    }
  }
  
  if (btnJustPressed) {
    currentState = STATE_MENU_MAIN;
    drawn = false;
    page = 0;
  }
}

void handleHighScores() {
  static uint8_t idx = 0;
  static bool drawn = false;
  
  if (!drawn) {
    lcd.clear();
    lcd.print(F("High Scores:"));
    lcd.setCursor(0, 1);
    lcd.print(idx + 1); lcd.print(F(". "));
    lcd.print(highScores[idx].name);
    lcd.print(F(" "));
    lcd.print(highScores[idx].score);
    
    lc.clearDisplay(0);
    for(uint8_t i=0; i < matrixSize; i++) {
      lc.setColumn(0, i, iconTrophy[i]);
    }
    drawn = true;
  }
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyYVal < joyCenterMin) {
      idx = (idx - 1 + highScoreCount) % highScoreCount;
      drawn = false;
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    } else if (joyYVal > joyCenterMax) {
      idx = (idx + 1) % highScoreCount;
      drawn = false;
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    }
  }
  
  if (btnJustPressed) {
    currentState = STATE_MENU_MAIN;
    drawn = false;
    idx = 0;
  }
}

void handleGamePlay() {
  // 1. Movement Logic
  if (millis() - lastGameMoveTime > moveCooldown) {
    int8_t deltaCol = 0;
    int8_t deltaRow = 0;
    
    // Determine input source
    if (settingIMUEnabled && imuHardwareAvailable) {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      float ax = a.acceleration.x;
      float ay = a.acceleration.y;
      
      if (abs(ax) > abs(ay) && abs(ax) > imuTiltThreshold) {
         if (ax < 0) deltaCol = 1; else deltaCol = -1;
      } else if (abs(ay) > abs(ax) && abs(ay) > imuTiltThreshold) {
         if (ay > 0) deltaRow = 1; else deltaRow = -1;
      }
    } else {
      // Joystick
      if (joyYVal < joyCenterMin) deltaRow = -1;
      else if (joyYVal > joyCenterMax) deltaRow = 1;
      else if (joyXVal < joyCenterMin) deltaCol = -1;
      else if (joyXVal > joyCenterMax) deltaCol = 1;
    }
    
    if (deltaCol != 0 || deltaRow != 0) {
      int8_t newCol = (int8_t)playerCol + deltaCol;
      int8_t newRow = (int8_t)playerRow + deltaRow;
      
      // Check Bounds
      if (newCol >= 0 && newCol < currentLevelDim && newRow >= 0 && newRow < currentLevelDim) {
        // Check Wall
        if (!isWall(newCol, newRow)) {
          playerCol = newCol;
          playerRow = newRow;
          lastGameMoveTime = millis();
          
          // Check Events
          for(uint8_t i=0; i<currentEntityCount; i++) {
            if (currentEntities[i].col == playerCol && currentEntities[i].row == playerRow && currentEntities[i].type == ENTITY_STAR) {
               // Remove star
               currentEntities[i] = currentEntities[currentEntityCount - 1]; // Swap with last
               currentEntityCount--;
               currentScore += pointsPerStar;
               currentLevelStarsCollected++;
               playSoundSequence(seqCollectStar, 2);
            }
          }
          
          // 2. Check Exit
          if (playerCol == currentLevelExitCol && playerRow == currentLevelExitRow) {
             if (currentLevelStarsCollected >= currentLevelStarsTotal) {
                // Level Clear
                playSoundSequence(seqLevelComplete, 4);
                // Calc Bonus
                uint32_t timeUsed = (millis() - levelStartTime) / 1000;
                uint32_t bonus = baseLevelClearPoints - (timeUsed * timeBonusDeduction);
                if (bonus > 0) currentScore += bonus;
                
                if (currentLevelIndex < totalLevels - 1) {
                  // Next Level
                  initLevels(currentLevelIndex + 1);
                } else {
                  // Victory
                  currentState = STATE_GAME_VICTORY;
                  lcd.clear();
                }
             }
          }
        }
      }
    }
  }
  
  // 2. Update LCD (Score)
  static uint32_t lastLCDUpdate = 0;
  if (millis() - lastLCDUpdate > LCDupdateInteval) {
    lcd.setCursor(0, 0);
    lcd.print(F("Lv:")); lcd.print(currentLevelIndex+1);
    lcd.print(F(" Stars:")); lcd.print(currentLevelStarsCollected);
    lcd.print(F("/")); lcd.print(currentLevelStarsTotal);
    
    lcd.setCursor(0, 1);
    lcd.print(F("Score: ")); lcd.print(currentScore);
    lastLCDUpdate = millis();
  }
  
  // 3. Render Matrix
  updateMatrixViewport();
  
  // 4. Pause Check
  if (btnJustPressed) {
    currentState = STATE_GAME_PAUSED;
    playSoundSequence(seqMenuSelect, 2);
    lcd.clear();
  }
}

void handleGamePaused() {
  static bool drawn = false;
  static int8_t lastOpt = -1;
  
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    if (joyXVal < joyCenterMin || joyXVal > joyCenterMax) {
      pausedSelectedOption = !pausedSelectedOption;
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
      drawn = false;
    }
  }
  
  if (!drawn) {
    lcd.setCursor(0, 0); lcd.print(F("PAUSED"));
    lcd.setCursor(0, 1);
    lcd.print(pausedSelectedOption == 0 ? F(">Continue  Exit") : F(" Continue >Exit"));
    drawn = true;
  }
  
  if (btnJustPressed) {
    playSoundSequence(seqMenuSelect, sizeof(seqMenuSelect) / sizeof(ToneSequence));
    if (pausedSelectedOption == 0) {
      currentState = STATE_GAME_PLAYING;
      lcd.clear();
    } else {
      currentState = STATE_MENU_MAIN;
      lcd.clear();
    }
    drawn = false;
  }
}

void handleVictory() {
  static bool drawn = false;
  if (!drawn) {
    lcd.clear();
    lcd.print(F("VICTORY!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Score: ")); lcd.print(currentScore);
    playSoundSequence(seqVictory, 4);
    
    lc.clearDisplay(0);
    // Happy face or Trophy
    for(uint8_t i = 0; i < matrixSize; i ++) {
      lc.setColumn(0, i, iconTrophy[i]);
    }
    drawn = true;
  }
  
  if (btnJustPressed) {
    // Check if high score
    bool isHighScore = false;
    for(uint8_t i=0; i<highScoreCount; i++) {
      if (currentScore > highScores[i].score) {
        isHighScore = true;
        break;
      }
    }
    
    if (isHighScore) {
      currentState = STATE_NAME_ENTRY;
    } else {
      currentState = STATE_MENU_MAIN;
    }
    drawn = false;
  }
}

void handleNameEntry() {
  static bool drawn = false;
  static uint8_t charIdx = 0;
  static char currentName[4] = "AAA";
  
  if (!drawn) {
    lcd.clear();
    lcd.print(F("New High Score!"));
    lcd.setCursor(0, 1);
    lcd.print(F("Name: ")); lcd.print(currentName);
    drawn = true;
  }
  
  // Edit logic
  if (millis() - lastInputMoveTime > menuMoveCooldown) {
    bool changed = false;
    // Y axis changes letter
    if (joyYVal < joyCenterMin) {
      currentName[charIdx]++;
      if (currentName[charIdx] > 'Z') currentName[charIdx] = 'A';
      changed = true;
    } else if (joyYVal > joyCenterMax) {
      currentName[charIdx]--;
      if (currentName[charIdx] < 'A') currentName[charIdx] = 'Z';
      changed = true;
    }
    
    // X axis changes position
    if (joyXVal > joyCenterMax) {
      charIdx = (charIdx + 1) % 3;
      changed = true;
    } else if (joyXVal < joyCenterMin) {
      charIdx = (charIdx - 1 + 3) % 3;
      changed = true;
    }
    
    if (changed) {
      lcd.setCursor(6, 1);
      lcd.print(currentName);
      // Blink cursor pos?
      lcd.setCursor(6+charIdx, 1);
      lcd.cursor();
      playSoundSequence(seqMenuMove, 1);
      lastInputMoveTime = millis();
    }
  }
  
  if (btnJustPressed) {
    lcd.noCursor();
    // Save Score
    // Shift scores down
    for(uint8_t i=0; i<highScoreCount; i++) {
      if (currentScore > highScores[i].score) {
        // Shift lower scores
        for(uint8_t j=highScoreCount-1; j>i; j--) {
          highScores[j] = highScores[j-1];
        }
        // Insert new
        highScores[i].score = currentScore;
        strcpy(highScores[i].name, currentName);
        break;
      }
    }
    saveHighScores();
    playSoundSequence(seqMenuSelect, 2);
    currentState = STATE_MENU_HIGHSCORES;
    drawn = false;
  }
}


void setup() {
  // 1. Hardware Init
  pinMode(PIN_JOY_BTN, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
  
  // Seed random
  randomSeed(analogRead(PIN_RANDOM_SEED));
  
  // LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print(F("Initializing..."));
  
  // Matrix
  lc.shutdown(0, false);
  lc.clearDisplay(0);
  
  // EEPROM
  loadSettings();
  applyLCDBrightness();
  applyMatrixBrightness();
  loadHighScores();
  
  // IMU
  Wire.begin();
  if (!mpu.begin()) {
    imuHardwareAvailable = false;
  } else {
    imuHardwareAvailable = true;
    mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  }
  
  // Init Inputs
  readInputs();
  
  // Start
  currentState = STATE_INTRO;
}

void loop() {
  uint32_t currentTime = millis();
  
  // Global Hardware Updates
  readInputs();
  updateAudio();
  
  // Global Blink Timer (for Stars/Cursor)
  if (currentTime - lastStarDisplayBlinkTime > starBlinkPeriod) {
    blinkStateStar = !blinkStateStar;
    lastStarDisplayBlinkTime = currentTime;
  }

  if(currentTime - lastPlayerDisplayBlinkTime > playerBlinkPeriod) {
    blinkStatePlayer = !blinkStatePlayer;
    lastPlayerDisplayBlinkTime = currentTime;
  }
  
  // Long press back to menu (Global safeguard, except during game)
  if (btnPressed && (currentTime - lastDebounceTime > backToMenuDelay)) {
     if (currentState != STATE_GAME_PLAYING && currentState != STATE_INTRO && currentState != STATE_NAME_ENTRY) {
        currentState = STATE_MENU_MAIN;
        backToMenuIssuedTime = currentTime;      
        btnPressed = false; // consume press
     }
  }


    // State Machine
    switch(currentState) {
      case STATE_INTRO:
        handleIntro();
        break;
      case STATE_MENU_MAIN:
        handleMainMenu();
        break;
      case STATE_MENU_HIGHSCORES:
        handleHighScores();
        break;
      case STATE_MENU_SETTINGS:
        handleSettingsMenu();
        break;
      case STATE_MENU_SETTINGS_LCD:
        handleSettingsValue(SET_LCD_BRIGHT);
        break;
      case STATE_MENU_SETTINGS_MATRIX:
        handleSettingsValue(SET_MATRIX_BRIGHT);
        break;
      case STATE_MENU_SETTINGS_SOUND:
        handleSettingsToggle(SET_SOUND);
        break;
      case STATE_MENU_SETTINGS_IMU:
        handleSettingsToggle(SET_IMU);
        break;
      case STATE_MENU_SETTINGS_RESET_SCORES:
        handleSettingsReset();
        break;
      case STATE_MENU_ABOUT:
        handleAbout();
        break;
      case STATE_MENU_HOWTO:
        handleHowTo();
        break;
      case STATE_GAME_PLAYING:
        handleGamePlay();
        break;
      case STATE_GAME_PAUSED:
        handleGamePaused();
        break;
      case STATE_GAME_VICTORY:
        handleVictory();
        break;
      case STATE_NAME_ENTRY:
        handleNameEntry();
        break;
      default:
        currentState = STATE_MENU_MAIN;
        break;
    }
}
