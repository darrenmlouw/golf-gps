#include "CoursesManager.h"
#include "courses_data.h"  // your R"RAWJSON(...) coursesJson string
#include <ArduinoJson.h>

void CoursesManager::beginFromFlash() {
  DynamicJsonDocument doc(60 * 1024);
  auto err = deserializeJson(doc, coursesJson);
  if (err) {
    Serial.print("JSON parse failed: ");
    Serial.println(err.c_str());
    return;
  }
  parseCourses(doc["courses"].as<JsonArray>());
  if (onLoaded_) onLoaded_();
}

void CoursesManager::parseCourses(JsonArray arr) {
  for (JsonObject courseJson : arr) {
    Course c;
    c.name = courseJson["name"].as<const char*>();
    c.location.lat = courseJson["location"]["lat"].as<double>();
    c.location.lon = courseJson["location"]["lon"].as<double>();

    for (JsonObject holeJson : courseJson["holes"].as<JsonArray>()) {
      Hole h;
      h.number = holeJson["number"].as<int>();
      h.pin.lat = holeJson["pin"]["lat"].as<double>();
      h.pin.lon = holeJson["pin"]["lon"].as<double>();
      h.front.lat = holeJson["front"]["lat"].as<double>();
      h.front.lon = holeJson["front"]["lon"].as<double>();
      h.back.lat = holeJson["back"]["lat"].as<double>();
      h.back.lon = holeJson["back"]["lon"].as<double>();

      for (JsonObject hzJson : holeJson["hazards"].as<JsonArray>()) {
        Hazard hz;
        hz.type = hzJson["type"].as<const char*>();
        hz.loc.lat = hzJson["lat"].as<double>();
        hz.loc.lon = hzJson["lon"].as<double>();
        h.hazards.push_back(hz);
      }

      c.holes.push_back(h);
    }

    courses_.push_back(c);
  }
}
