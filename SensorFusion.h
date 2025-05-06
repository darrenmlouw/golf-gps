// SensorFusion.h
#pragma once
#include <Arduino.h>

/// Simple struct to hold position & velocity
struct EKFState {
  float x = 0, y = 0;    // meters
  float vx = 0, vy = 0;  // m/s
};

class SensorFusion {
public:
  /// Get the one-and-only EKF instance
  static SensorFusion& instance() {
    static SensorFusion inst;
    return inst;
  }

  /// Reset/initialize covariances
  void initCov();

  /**
   * Predict step: call at IMU rate with local-frame accel (m/s²).
   * @param ax  accel in X
   * @param ay  accel in Y
   * @param dt  time since last predict in seconds
   */
  void predict(float ax, float ay, float dt);

  /**
   * Update step: call at GPS rate with new lat/lon fix.
   * The first call just sets the local‐ENU origin.
   */
  void updateGPS(float latDeg, float lonDeg);

  /// Retrieve current fused state
  EKFState getState() const { return state; }

  void getFusedLatLon(float& latOut, float& lonOut) const;

private:
  SensorFusion();
  ~SensorFusion() = default;
  SensorFusion(const SensorFusion&)            = delete;
  SensorFusion& operator=(const SensorFusion&) = delete;

  // — the state vector & covariances —
  EKFState state;
  float P[4][4], Q[4][4], R[2][2];

  // origin for lat/lon → meters
  float lat0 = 0, lon0 = 0;
  bool originInit = false;

  // Earth radius for conversion
  static constexpr float R_earth = 6378137.0f;

  // — non‐linear models & Jacobians —
  void f(const float x[4], const float u[2], float dt, float x_out[4]);
  void computeF(const float x[4], const float u[2], float dt, float F_out[4][4]);
  void h(const float x[4], float z_out[2]);
  void computeH(const float x[4], float H_out[2][4]);
};
