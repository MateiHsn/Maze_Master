#ifndef IMU_CONTROLLER_HPP
#define IMU_CONTROLLER_HPP

#include <Defaults.hpp>

enum class TiltDirection {
  NONE,
  UP,
  DOWN,
  LEFT,
  RIGHT
};

class IMUController {
private:
  Adafruit_MPU6050 mpu;

  static const int16_t tiltThreshold = 4;
  static const uint32_t readInterval = 100;
  uint32_t lastReadTime;

  float accelX, accelY, accelZ;
  TiltDirection currentDirection;
  TiltDirection lastDirection;

  bool imuAvailable;

  void readAccelData();
  TiltDirection calculateTiltDirection();

public:
  IMUController();

  bool begin();
  void update();

  TiltDirection getDirection() const;
  bool hasNewDirection() const;
  void clearDirection();

  float getAccelX() const { return accelX; }
  float getAccelY() const { return accelY; }
  float getAccelZ() const { return accelZ; }

  bool isAvailable() const { return imuAvailable; }

};
#endif