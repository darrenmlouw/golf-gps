// GpsManager.cpp

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

  // start GPS UART
  gpsSerial->begin(baud, SERIAL_8N1, rxPin, txPin);
  GPS->begin(baud);

  // request all sentences at 5 Hz
  GPS->sendCommand(outCmd);
  GPS->sendCommand(hzCmd);

  // 3) Enable SBAS search
  GPS->sendCommand("$PMTK313,1*2E");   // enable SBAS

  // 4) Use WAAS/EGNOS/etc as DGPS correction source
  GPS->sendCommand("$PMTK301,2*2E");   // set DGPS mode to SBAS

  // debug echo on USB serial
  Serial.begin(115200);
}

void GpsManager::update() {
  // 1) Drain parser buffer
  while (GPS->read()) { /* no-op */ }

  // 2) If a new NMEA arrived, attempt to parse it
  if (GPS->newNMEAreceived()) {
    char* nmea = GPS->lastNMEA();
    // Only handle GGA/RMC
    if (   strncmp(nmea, "$GPGGA", 6) == 0
        || strncmp(nmea, "$GNGGA", 6) == 0
        || strncmp(nmea, "$GPRMC", 6) == 0
        || strncmp(nmea, "$GNRMC", 6) == 0) {
      GPS->parse(nmea);

      // update full data_
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
    }
  }

  // 3) Always print status once per update() call
  if (!data_.fix) {
    Serial.println("No fix");
  } else {
    Serial.printf("Fix: %.6f, %.6f\n",
                  data_.lat,
                  data_.lon);
  }
}
