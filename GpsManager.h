// GpsManager.h
#pragma once

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>

/// Complete GPS fix data from GGA + RMC
struct GpsData {
  // Position & status
  bool     fix         = false;
  uint8_t  fixQuality  = 0;      // 0=none,1=GPS,2=DGPS…
  float    lat         = 0.0f;   // degrees
  float    lon         = 0.0f;   // degrees

  // UTC timestamp (GGA time + RMC date)
  uint8_t  hour        = 0;
  uint8_t  minute      = 0;
  uint8_t  second      = 0;
  uint8_t  day         = 0;
  uint8_t  month       = 0;
  uint16_t year        = 0;      // full year, e.g. 2025

  // From GGA
  uint8_t  sats        = 0;      // satellites in fix
  float    hdop        = 99.0f;  // horizontal dilution of precision
  float    altitude    = 0.0f;   // meters above MSL

  // From RMC/VTG
  float    speedKnots  = 0.0f;   // speed over ground
  float    trackAngle  = 0.0f;   // course over ground, degrees
};

/// Singleton GPS driver
class GpsManager {
public:
  /// Accessor for the single instance
  static GpsManager& instance();

  /**
   * @brief Initialize the GPS module
   * @param port    HardwareSerial port (e.g. &Serial1)
   * @param baud    GPS baud rate (e.g. 9600)
   * @param rxPin   GPS RX pin
   * @param txPin   GPS TX pin
   * @param outCmd  NMEA output mask (default = ALLDATA)
   * @param hzCmd   update rate (default = 5 Hz)
   */
  void begin(HardwareSerial* port,
             uint32_t baud,
             int rxPin,
             int txPin,
             const char* outCmd = PMTK_SET_NMEA_OUTPUT_ALLDATA,
             const char* hzCmd  = PMTK_SET_NMEA_UPDATE_5HZ);

  /// Must be called in loop(): reads, parses, updates data_
  void update();

  /// Retrieve the latest fix
  const GpsData& getData() const { return data_; }

private:
  GpsManager();
  HardwareSerial* gpsSerial = nullptr;
  Adafruit_GPS*   GPS       = nullptr;
  GpsData         data_;
};
