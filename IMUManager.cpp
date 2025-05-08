#include "IMUManager.h"
#include "pin_config.h" // for IIC_SDA / IIC_SCL
#include <Arduino.h>    // for Serial, micros()

bool IMUManager::begin() {
  // init I2C on the same pins as your touch controller
  if (!qmi.init(Wire, IIC_SDA, IIC_SCL, QMI8658_L_SLAVE_ADDRESS)) {
    Serial.println("QMI init failed!");
    return false;
  }

  // --- configure & enable accelerometer ---
  qmi.configAccelerometer(
    SensorQMI8658::ACC_RANGE_4G,
    SensorQMI8658::ACC_ODR_1000Hz,
    SensorQMI8658::LPF_MODE_0
  );
  qmi.enableAccelerometer();

  // --- configure & enable gyroscope ---
  qmi.configGyroscope(
    SensorQMI8658::GYR_RANGE_512DPS,
    SensorQMI8658::GYR_ODR_1793_6Hz,
    SensorQMI8658::LPF_MODE_0
  );
  qmi.enableGyroscope();

  Serial.println("IMU initialized OK");
  return true;
}

void IMUManager::update() {
  // only pull new data when it's ready
  if (!qmi.getDataReady()) return;

  float ax, ay, az, gx, gy, gz;

  if (qmi.getAccelerometer(ax, ay, az)) {
    _raw.ax = ax - _accelOffsetX;
    _raw.ay = ay - _accelOffsetY;
    _raw.az = az - _accelOffsetZ;
  }

  if (qmi.getGyroscope(gx, gy, gz)) {
    _raw.gx = gx - _gyroOffsetX;
    _raw.gy = gy - _gyroOffsetY;
    _raw.gz = gz - _gyroOffsetZ;
  }

  // debug print every sample
  // Serial.printf(
  //   "Acc: %.3f, %.3f, %.3f  |  Gyro: %.3f, %.3f, %.3f\n",
  //   _raw.ax, _raw.ay, _raw.az,
  //   _raw.gx, _raw.gy, _raw.gz
  // );
}

void IMUManager::calibrate(int N) {
  // average N gyro samples for zero‐offset
  float sumGX=0, sumGY=0, sumGZ=0;
  for (int i = 0; i < N; ++i) {
    // spin until new data
    while (!qmi.getDataReady()) {}
    float gx, gy, gz;
    qmi.getGyroscope(gx, gy, gz);
    sumGX += gx;
    sumGY += gy;
    sumGZ += gz;
    delay(2);
  }
  _gyroOffsetX = sumGX / N;
  _gyroOffsetY = sumGY / N;
  _gyroOffsetZ = sumGZ / N;
  Serial.println("IMU gyro calibrated");
}
