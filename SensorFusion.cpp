// SensorFusion.cpp
#include "SensorFusion.h"
#include <cstring>
#include <math.h>

//==============================================================================
// Constructor & init
//==============================================================================
SensorFusion::SensorFusion() {
  initCov();
}

void SensorFusion::initCov() {
  // P: initial uncertainty
  memset(P, 0, sizeof(P));
  P[0][0] = P[1][1] = 10;   // 10 m² pos
  P[2][2] = P[3][3] =  1;   // 1 (m/s)² vel

  // Q: process noise (accel & model)
  memset(Q, 0, sizeof(Q));
  Q[0][0] = Q[1][1] = 0.1f;
  Q[2][2] = Q[3][3] = 0.1f;

  // R: measurement noise (~2 m σ)
  memset(R, 0, sizeof(R));
  R[0][0] = R[1][1] = 4.0f;
}

//==============================================================================
// Non‐linear process model: constant accel
// x = [px,py,vx,vy], u = [ax,ay]
void SensorFusion::f(const float x[], const float u[], float dt, float x_out[]) {
  x_out[0] = x[0] + x[2]*dt + 0.5f*u[0]*dt*dt;
  x_out[1] = x[1] + x[3]*dt + 0.5f*u[1]*dt*dt;
  x_out[2] = x[2] +        u[0]*dt;
  x_out[3] = x[3] +        u[1]*dt;
}

// Jacobian ∂f/∂x
void SensorFusion::computeF(const float[], const float[], float dt, float F[4][4]) {
  // For constant‐accel model, F is constant:
  // [1 0 dt 0]
  // [0 1 0 dt]
  // [0 0 1  0]
  // [0 0 0  1]
  memset(F, 0, sizeof(float)*16);
  F[0][0]=1;  F[0][2]=dt;
  F[1][1]=1;  F[1][3]=dt;
  F[2][2]=1;
  F[3][3]=1;
}

//==============================================================================
// Measurement model: GPS gives position in meters
void SensorFusion::h(const float x[], float z_out[2]) {
  z_out[0] = x[0];
  z_out[1] = x[1];
}

// Jacobian ∂h/∂x
void SensorFusion::computeH(const float[], float H[2][4]) {
  // [1 0 0 0]
  // [0 1 0 0]
  memset(H, 0, sizeof(float)*8);
  H[0][0] = 1;
  H[1][1] = 1;
}

//==============================================================================
// Predict step
void SensorFusion::predict(float ax, float ay, float dt) {
  float xk[4] = { state.x, state.y, state.vx, state.vy };
  float uk[2] = { ax, ay };
  float x_pred[4], F[4][4];

  // 1) state prediction
  f(xk, uk, dt, x_pred);

  // 2) covariance prediction: P = F·P·Fᵀ + Q
  computeF(xk, uk, dt, F);
  float tmp[4][4]={}, Pn[4][4]={};
  // tmp = F·P
  for(int i=0;i<4;i++) for(int j=0;j<4;j++)
    for(int k=0;k<4;k++) tmp[i][j] += F[i][k]*P[k][j];
  // Pn = tmp·Fᵀ + Q
  for(int i=0;i<4;i++) for(int j=0;j<4;j++){
    Pn[i][j] = Q[i][j];
    for(int k=0;k<4;k++) Pn[i][j] += tmp[i][k]*F[j][k];
  }
  memcpy(P, Pn, sizeof(P));

  // 3) update state
  state.x  = x_pred[0];
  state.y  = x_pred[1];
  state.vx = x_pred[2];
  state.vy = x_pred[3];
}

//==============================================================================
// Update step with GPS fix
void SensorFusion::updateGPS(float latDeg, float lonDeg) {
  // 1) If first fix, set local ENU origin
  if (!originInit) {
    lat0 = latDeg;
    lon0 = lonDeg;
    originInit = true;
    // leave state.x/y at zero
    return;
  }

  // 2) convert lat/lon→meters
  float dLat = (latDeg - lat0) * (M_PI/180.0f);
  float dLon = (lonDeg - lon0) * (M_PI/180.0f) * cos(lat0 * M_PI/180.0f);
  float z_meas[2] = { R_earth * dLon, R_earth * dLat };

  // 3) measurement prediction
  float z_pred[2];
  h(&state.x, z_pred);

  // 4) innovation
  float y0 = z_meas[0] - z_pred[0];
  float y1 = z_meas[1] - z_pred[1];

  // 5) compute H, S = HPHᵀ + R
  float H[2][4];
  computeH(&state.x, H);
  // because H picks off P[0..1][0..1], S simplifies to:
  float S00 = P[0][0] + R[0][0];
  float S01 = P[0][1];
  float S10 = P[1][0];
  float S11 = P[1][1] + R[1][1];
  float det = S00 * S11 - S01 * S10;
  float Si00 =  S11 / det, Si01 = -S01 / det;
  float Si10 = -S10 / det, Si11 =  S00 / det;

  // 6) Kalman gain K = P·Hᵀ·S⁻¹  (4×2 matrix)
  float K[4][2];
  for(int i=0;i<4;i++){
    K[i][0] = P[i][0]*Si00 + P[i][1]*Si10;
    K[i][1] = P[i][0]*Si01 + P[i][1]*Si11;
  }

  // 7) state update
  state.x  += K[0][0]*y0 + K[0][1]*y1;
  state.y  += K[1][0]*y0 + K[1][1]*y1;
  state.vx += K[2][0]*y0 + K[2][1]*y1;
  state.vy += K[3][0]*y0 + K[3][1]*y1;

  // 8) covariance update P = (I-KH)·P
  float Pn[4][4] = {};
  for(int i=0;i<4;i++) {
    for(int j=0;j<4;j++){
      // (I-KH)[i][0] = −K[i][0], except diag adds 1 for i=0,1
      float ikh0 = (i<2 ? (i==0?1:0) - K[i][0] : -K[i][0]);
      float ikh1 = (i<2 ? (i==1?1:0) - K[i][1] : -K[i][1]);
      // (I-KH)*P row i
      Pn[i][j] = ikh0 * P[0][j] + ikh1 * P[1][j] + ((i>=2)? P[i][j] : 0);
    }
  }
  memcpy(P, Pn, sizeof(P));
}

void SensorFusion::getFusedLatLon(float& latOut, float& lonOut) const {
  if (!originInit) {
    latOut = lonOut = NAN;
    return;
  }
  // Δφ = North (m) / R
  float dLat = state.y / R_earth;
  // Δλ = East (m) / (R·cos φ₀)
  float dLon = state.x / (R_earth * cos(lat0 * PI / 180.0f));

  // convert back to degrees
  latOut = lat0 + dLat * (180.0f / PI);
  lonOut = lon0 + dLon * (180.0f / PI);
}
