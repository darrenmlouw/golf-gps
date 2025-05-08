#pragma once

#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>

struct GpsData {
  bool     fix         = false;
  uint8_t  fixQuality  = 0;
  float    lat         = 0.0f;
  float    lon         = 0.0f;
  uint8_t  hour        = 0;
  uint8_t  minute      = 0;
  uint8_t  second      = 0;
  uint8_t  day         = 0;
  uint8_t  month       = 0;
  uint16_t year        = 0;
  uint8_t  sats        = 0;
  float    hdop        = 99.0f;
  float    altitude    = 0.0f;
  float    speedKnots  = 0.0f;
  float    trackAngle  = 0.0f;
};

class GpsManager {
public:
  static GpsManager& instance();

  void begin(HardwareSerial* port,
             uint32_t baud,
             int rxPin,
             int txPin,
             const char* outCmd = PMTK_SET_NMEA_OUTPUT_ALLDATA,
             const char* hzCmd  = PMTK_SET_NMEA_UPDATE_5HZ);

  /// Called from ISR
  void update();

  /// Called from loop()
  bool              hasNewData();
  GpsData           fetchData();

private:
  GpsManager();
  HardwareSerial*   gpsSerial = nullptr;
  Adafruit_GPS*     GPS       = nullptr;
  GpsData           data_;
  volatile bool     newData_  = false;
};
