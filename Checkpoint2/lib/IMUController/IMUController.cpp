#include <IMUController.hpp>

IMUController::IMUController() {
  accelX = 0;
  accelY = 0;
  accelZ = 0;

  currentDirection = TiltDirection::NONE;
  lastDirection = TiltDirection::NONE;

  lastReadTime = 0;
  imuAvailable = false;
}

bool IMUController::begin() {
  Wire.begin();

  if(!mpu.begin()) {
    imuAvailable = false;
    return imuAvailable;
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  imuAvailable = true;
  return imuAvailable;
}

void IMUController::update() {
  if(!imuAvailable) return;

  uint32_t currentTime = millis();

  if(currentTime - lastReadTime < readInterval) {
    return;
  }

  lastReadTime = currentTime;
  readAccelData();

  lastDirection = currentDirection;
  currentDirection = calculateTiltDirection();
}

void IMUController::readAccelData() {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  accelX = a.acceleration.x;
  accelY = a.acceleration.y;
  accelZ = a.acceleration.z;
}

TiltDirection IMUController::calculateTiltDirection() {
  float absAccelX = abs(accelX);
  float absAccelY = abs(accelY);

  if(absAccelX < tiltThreshold && absAccelY < tiltThreshold) {
    return TiltDirection::NONE;
  }

  if(absAccelX > absAccelY) {
    return (accelX > 0) ? TiltDirection::RIGHT : TiltDirection::LEFT;
  } else {
    return (accelY > 0) ? TiltDirection::DOWN : TiltDirection::UP;
  }
}

TiltDirection IMUController::getDirection() const {
  return currentDirection;
}

bool IMUController::hasNewDirection() const {
  return currentDirection != lastDirection && currentDirection != TiltDirection::NONE;
}

void IMUController::clearDirection() {
  lastDirection = currentDirection;
}