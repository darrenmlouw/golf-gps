#pragma once
#include "Page.h"
#include <lvgl.h>

class HomePage : public Page {
public:
  void onCreate()  override;
  void onDestroy() override;

  /// LVGL event callback
  static void event_cb(lv_event_t* e);

private:
  lv_obj_t* scr_        = nullptr;
  lv_obj_t* btnCourses_ = nullptr;
  lv_obj_t* btnGpsView_ = nullptr;
};
