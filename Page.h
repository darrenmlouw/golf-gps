#pragma once
#include <lvgl.h>
#include "GpsManager.h"

class Page {
public:
  virtual ~Page() = default;
  virtual void onCreate()  = 0;
  virtual void onDestroy() = 0;

  /** Pages override this to update their own labels. */
  virtual void onGpsUpdate(const GpsData& d) { /* no-op */ }

protected:
  lv_obj_t* scr_       = nullptr;
  lv_obj_t* ledStatus_ = nullptr;
  lv_timer_t* gpsTimer_ = nullptr;  // <— new
  lv_obj_t* hdrLabel_ = nullptr;

  void createBase(const char* title, bool canGoBack);
  static void backEventCallback(lv_event_t* e);
  static void swipeEventCallback(lv_event_t* e);
  lv_color_t getFixColor(const GpsData& d);
};
