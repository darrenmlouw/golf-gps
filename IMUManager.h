#pragma once

#include <Wire.h>
#include "SensorQMI8658.hpp"

struct ImuRaw { float ax, ay, az, gx, gy, gz; };

class IMUManager {
public:
  /// Get the singleton instance
  static IMUManager& instance() {
    static IMUManager inst;
    return inst;
  }

  /** Power up, configure & enable accel + gyro. Returns false on failure */
  bool begin();

  /** Call regularly (e.g. from a ticker) to pull new samples */
  void update();

  /** Take N gyro readings and zero‐offset them */
  void calibrate(int N = 200);

  /** Last raw readings */
  ImuRaw getRaw() const { return _raw; }

private:
  IMUManager() = default;
  ~IMUManager() = default;
  IMUManager(const IMUManager&)            = delete;
  IMUManager& operator=(const IMUManager&) = delete;

  SensorQMI8658 qmi;
  ImuRaw        _raw{};
  float         _accelOffsetX = 0,
                _accelOffsetY = 0,
                _accelOffsetZ = 0;
  float         _gyroOffsetX  = 0,
                _gyroOffsetY  = 0,
                _gyroOffsetZ  = 0;
};
