#ifndef DEFAULTS_HPP
#define DEFAULTS_HPP

#include <Arduino.h>
#include <LedControl.h>
#include <LiquidCrystal.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

const uint8_t JOYSTICK_BUTTON_PIN = 2;
const uint8_t BUZZER_PIN = 3;
const uint8_t LCD_D4_PIN = 4;
const uint8_t LCD_D5_PIN = 5;
const uint8_t LCD_D6_PIN = 6;
const uint8_t LCD_D7_PIN = 7;
const uint8_t LCD_RS_PIN = 8;
const uint8_t LCD_EN_PIN = 9;
const uint8_t MAX7219_LOAD_PIN = 10;
const uint8_t MAX7219_DIN_PIN = 11;
const uint8_t MAX7219_CLK_PIN = 13;
const uint8_t JOYSTICK_X_AXIS = A1;
const uint8_t JOYSTICK_Y_AXIS = A2;

const uint8_t eepromAddress = 100;

const uint16_t joystickDefaultValue = 512;

#endif