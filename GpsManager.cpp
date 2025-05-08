#include "GpsManager.h"
#include <Arduino.h>

GpsManager& GpsManager::instance() {
  static GpsManager inst;
  return inst;
}

GpsManager::GpsManager() = default;

void GpsManager::begin(HardwareSerial* port,
                       uint32_t baud,
                       int rxPin,
                       int txPin,
                       const char* outCmd,
                       const char* hzCmd) {
  gpsSerial = port;
  GPS       = new Adafruit_GPS(gpsSerial);

  gpsSerial->begin(baud, SERIAL_8N1, rxPin, txPin);
  GPS->begin(baud);

  GPS->sendCommand(outCmd);
  GPS->sendCommand(hzCmd);
  GPS->sendCommand("$PMTK313,1*2E");
  GPS->sendCommand("$PMTK301,2*2E");

  Serial.begin(115200);  // only here
}

void GpsManager::update() {
  // 1) Drain parser buffer
  while (GPS->read()) { /* no-op */ }

  // 2) If new NMEA, parse and update data_
  if (GPS->newNMEAreceived()) {
    char* nmea = GPS->lastNMEA();
    if (!strncmp(nmea, "$GPGGA", 6) ||
        !strncmp(nmea, "$GNGGA", 6) ||
        !strncmp(nmea, "$GPRMC", 6) ||
        !strncmp(nmea, "$GNRMC", 6)) {

      GPS->parse(nmea);
      data_.fix        = GPS->fix;
      data_.fixQuality = GPS->fixquality;
      data_.lat        = GPS->latitudeDegrees;
      data_.lon        = GPS->longitudeDegrees;
      data_.hour       = GPS->hour;
      data_.minute     = GPS->minute;
      data_.second     = GPS->seconds;
      data_.day        = GPS->day;
      data_.month      = GPS->month;
      data_.year       = GPS->year + 2000;
      data_.sats       = GPS->satellites;
      data_.hdop       = GPS->HDOP;
      data_.altitude   = GPS->altitude;
      data_.speedKnots = GPS->speed;
      data_.trackAngle = GPS->angle;

      newData_ = true;
    }
  }
}

bool GpsManager::hasNewData() {
  return newData_;
}

GpsData GpsManager::fetchData() {
  noInterrupts();
  GpsData snapshot = data_;
  newData_ = false;
  interrupts();
  return snapshot;
}
