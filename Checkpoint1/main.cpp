#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <LedControl.h>
#include <LiquidCrystal.h>

const uint8_t JOYSTICK_X_PIN = A1;
const uint8_t JOYSTICK_Y_PIN = A2;
const uint8_t JOYSTICK_BUTTON_PIN = 2;

const uint8_t LCD_PIN_D4 = 4;
const uint8_t LCD_PIN_D5 = 5;
const uint8_t LCD_PIN_D6 = 6;
const uint8_t LCD_PIN_D7 = 7;
const uint8_t LCD_PIN_RS = 8;
const uint8_t LCD_PIN_EN = 9;

const uint8_t BUZZER_PIN = 3;

const uint8_t MAX_DIN_PIN = 11;
const uint8_t MAX_CLK_PIN = 13;
const uint8_t MAX_CS_PIN  = 10;

const uint8_t lcdCols = 16;
const uint8_t lcdRows = 2;

const uint8_t matrixCount = 1;
const uint8_t matrixMinIndex = 0;
const uint8_t matrixMaxIndex = 7;
const uint8_t matrixDefaultBrightness = 15;

const uint16_t joystickCenter = 512;
const uint16_t joystickDeadzone = 250;

const uint64_t joystickInterval = 300;
const uint64_t buttonDebounceInterval = 60;
const uint64_t lcdRefreshInterval = 100;
const uint64_t imuUpdateInterval = 10;
const uint64_t splashScreenDuration = 2000;

const float angleMin = -45.0;
const float angleMax =  45.0;
// const float RAD_TO_DEG = 57.2958;

const uint8_t mainMenuCount = 1;
const uint8_t pauseMenuCount = 2;

LiquidCrystal lcd(LCD_PIN_RS, LCD_PIN_EN, LCD_PIN_D4, LCD_PIN_D5, LCD_PIN_D6, LCD_PIN_D7);
LedControl matrix(MAX_DIN_PIN, MAX_CLK_PIN, MAX_CS_PIN, matrixCount);
Adafruit_MPU6050 mpu;

enum AppState {
  STATE_SPLASH,
  STATE_MAIN_MENU,
  STATE_RUNNING,
  STATE_PAUSE_MENU
};

AppState appState = STATE_SPLASH;

const char* mainMenuItems[mainMenuCount] = {"Start Demo"};
const char* pauseMenuItems[pauseMenuCount] = {"Resume", "Main Menu"};

uint8_t mainMenuIndex = 0;
uint8_t pauseMenuIndex = 0;

uint64_t timeNow = 0;
uint64_t lastJoystick = 0;
uint64_t lastButtonTime = 0;
uint64_t lastLcdUpdate = 0;
uint64_t lastImuUpdate = 0;
uint64_t splashStartTime = 0;

uint32_t soundFrequency = 1000;
uint64_t soundDuration = 50;
uint64_t soundStartTime = 0;
bool shouldPlay = false; 

bool lastButtonState = HIGH;
bool btnPressed = false;
bool splashDisplayed = false;

uint8_t dotX = 3;
uint8_t dotY = 3;

int readJoystickAxis(int pin) { 
  uint16_t val = analogRead(pin);
  if(val < joystickCenter - joystickDeadzone) return -1;
  if(val > joystickCenter + joystickDeadzone) return 1;
  return 0;
}

void updateButton() {
  bool reading = digitalRead(JOYSTICK_BUTTON_PIN);

  if(reading != lastButtonState) {
    lastButtonTime = timeNow;

    if(timeNow - lastButtonState > buttonDebounceInterval) {
      if(lastButtonState == HIGH && reading == LOW)
        btnPressed = true;
    }
  }
  lastButtonState = reading;
}

void clearMatrix() {
  for(uint8_t r = matrixMinIndex; r <= matrixMaxIndex; r++) {
    matrix.setRow(0, r, 0);
  }
}

void drawDot(uint8_t x, uint8_t y){
  clearMatrix();
  if(x <= matrixMaxIndex && y <= matrixMaxIndex) {
    matrix.setLed( 0, y, x, true);
  }
}

void updateIMU() {

  if(timeNow - lastImuUpdate < imuUpdateInterval) return;
  lastImuUpdate = timeNow;

  sensors_event_t a,g,t;

  mpu.getEvent(&a, &g, &t);

  float ax = a.acceleration.x;
  float ay = a.acceleration.y;
  float az = a.acceleration.z;

  float pitch = atan2(ax, sqrt(ay*ay + az*az)) * RAD_TO_DEG;
  float roll = atan2(ay, sqrt(ax*ax + az*az)) * RAD_TO_DEG;

  dotX = map(roll, angleMin, angleMax, matrixMinIndex, matrixMaxIndex);
  dotY = map(pitch, angleMin, angleMax, matrixMinIndex, matrixMaxIndex);

  // dotX = constrain(dotX, matrixMinIndex, matrixMaxIndex);
  // dotY = constrain(dotY, matrixMinIndex, matrixMaxIndex);
}

void updateLCD() {
  if(timeNow - lastLcdUpdate < lcdRefreshInterval) return;

  lastLcdUpdate = timeNow;

  lcd.clear();

  switch(appState) {
    case STATE_MAIN_MENU:
      lcd.setCursor(0, 0);
      lcd.print(">");
      lcd.print(mainMenuItems[mainMenuIndex]);
    break;
    case STATE_PAUSE_MENU:
      lcd.setCursor(0, 0);
      lcd.print(pauseMenuIndex == 0 ? "> Resume" : "  Resume");
      lcd.setCursor(0,1);
      lcd.print(pauseMenuIndex == 1 ? "> Main Menu" : "  Main Menu");
    break;
    case STATE_RUNNING:
      lcd.setCursor(0, 0);
      lcd.print("Demo running");
      lcd.setCursor(0, 1);
      lcd.print("Button: Pause");
  }
}

void updateSound(){
  
  if(shouldPlay) {
    tone(BUZZER_PIN, soundFrequency);
    soundStartTime = millis();
    shouldPlay = false;
  }

  if(!shouldPlay && timeNow - soundStartTime > soundDuration){
    noTone(BUZZER_PIN);
  }

}

void handleMainMenu() {
  bool canMove = (timeNow - lastJoystick > joystickInterval);
  int move = canMove ? readJoystickAxis(JOYSTICK_Y_PIN) : 0;

  if(move) {
    shouldPlay = true;
    mainMenuIndex = (mainMenuIndex + move + mainMenuCount) % mainMenuCount;
    lastJoystick = timeNow;
  }

  if(btnPressed) {
    mpu.begin();
    shouldPlay = true;
    btnPressed = false;
    appState = STATE_RUNNING;
  }
}

void handleRunning() {
  updateIMU();
  drawDot(dotX, dotY);

  if(btnPressed) {
    shouldPlay = true;
    btnPressed = false;
    appState = STATE_PAUSE_MENU;
  }
}

void handlePauseMenu() {
  bool canMove = (timeNow - lastJoystick > joystickInterval);
  int move = canMove ? readJoystickAxis(JOYSTICK_Y_PIN) : 0;

  if(move) {
    shouldPlay = true;
    pauseMenuIndex = (pauseMenuIndex + move + pauseMenuCount) % pauseMenuCount;
    lastJoystick = timeNow;
  }

  if(btnPressed) {
    btnPressed = false;
    shouldPlay = true;
    if(pauseMenuIndex == 0) {
      appState = STATE_RUNNING;
    } else { 
      pauseMenuIndex = 0;
      appState = STATE_MAIN_MENU;
      clearMatrix();
    }
  }
}

void setup() {

  analogReadResolution(10);

  pinMode(JOYSTICK_BUTTON_PIN, INPUT_PULLUP);
  pinMode(JOYSTICK_Y_PIN, INPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  lcd.begin(lcdCols, lcdRows);

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  matrix.shutdown(0, false);
  matrix.setIntensity(0, matrixDefaultBrightness);
  matrix.clearDisplay(0);

  Wire.begin();
  mpu.begin();
}

void loop() {

  timeNow = millis();

  updateButton();
  updateSound();
  updateLCD();

  switch(appState) {
    case STATE_MAIN_MENU:
      handleMainMenu();
    break;

    case STATE_RUNNING:
      handleRunning();
    break;

    case STATE_PAUSE_MENU:
      handlePauseMenu();
    break;

    default:
      appState = STATE_MAIN_MENU;
    break;
  }

}