#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <vector>
#include <functional>

// —— Data types ——
struct Geo {
  double lat;
  double lon;
};

struct Hazard {
  String type;
  Geo loc;
};

// CoursesManager.h
struct Hole {
  int   number;
  int   par;            // ← add this
  Geo   pin;
  Geo   front;
  Geo   back;
  std::vector<Hazard> hazards;
};

struct Course {
  String name;
  Geo location;
  std::vector<Hole> holes;
};

class CoursesManager {
public:
  static CoursesManager& instance() {
    static CoursesManager inst;
    return inst;
  }

  /// Load from the `coursesJson` PROGMEM string
  void beginFromFlash();

  const std::vector<Course>& getCourses() const {
    return courses_;
  }

  /// Optional callback when done loading
  void setLoadedCallback(std::function<void()> cb) {
    onLoaded_ = cb;
  }

private:
  CoursesManager() = default;

  /// Break out the JSON-to-object parsing
  void parseCourses(JsonArray arr);

  std::vector<Course> courses_;
  std::function<void()> onLoaded_;
};
