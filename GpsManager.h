#pragma once
#include <Arduino.h>
#include <Adafruit_GPS.h>
#include <HardwareSerial.h>
#include <functional>

/// Simple POD for your GPS fix
struct GpsData {
  bool   fix  = false;
  float  lat  = 0;
  float  lon  = 0;
  float  hdop = 99;
  int    sats = 0;
};

/// Singleton GPS driver + callback
class GpsManager {
public:
  static GpsManager& instance() {
    static GpsManager inst;
    return inst;
  }

  void begin(HardwareSerial* port, uint32_t baud, int rxPin, int txPin,
             const char* outCmd = PMTK_SET_NMEA_OUTPUT_RMCONLY,
             const char* hzCmd  = PMTK_SET_NMEA_UPDATE_1HZ)
  {
    gpsSerial = port;
    GPS       = new Adafruit_GPS(gpsSerial);
    gpsSerial->begin(baud, SERIAL_8N1, rxPin, txPin);
    GPS->begin(baud);
    GPS->sendCommand(outCmd);
    GPS->sendCommand(hzCmd);
  }

  void update() {
    while(gpsSerial->available()) GPS->read();
    if(GPS->newNMEAreceived() && GPS->parse(GPS->lastNMEA())) {
      GpsData d;
      d.fix  = GPS->fix;
      d.lat  = GPS->latitudeDegrees;
      d.lon  = GPS->longitudeDegrees;
      d.hdop = GPS->HDOP;
      d.sats = GPS->satellites;
      data_ = d;
      if(onData_) onData_(data_);
    }
  }

  const GpsData& getData() const { return data_; }

  void setCallback(std::function<void(const GpsData&)> cb) {
    onData_ = cb;
  }

private:
  GpsManager() = default;
  HardwareSerial*                gpsSerial = nullptr;
  Adafruit_GPS*                  GPS       = nullptr;
  GpsData                        data_;
  std::function<void(const GpsData&)> onData_;
};
