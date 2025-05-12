#pragma once
#include "Page.h"
#include "CoursesManager.h"
#include <lvgl.h>
#include <algorithm>

class HolePage : public Page {
public:
  HolePage(int courseIdx);
  void onCreate() override;
  void onDestroy() override;
  void onGpsUpdate(const GpsData& d) override;   // updates distances & spinner

private:
  int courseIdx_;
  int holeIdx_ = 0;

  // labels for front / mid / back
  lv_obj_t* lblFront_ = nullptr;
  lv_obj_t* lblMid_   = nullptr;
  lv_obj_t* lblBack_  = nullptr;

  void navigateTo(int newIdx);
  void updateDistances(const GpsData& d);
  static void gestureCb(lv_event_t* e);
};
