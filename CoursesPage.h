#pragma once
#include "Page.h"
#include "GpsManager.h"
#include "CoursesManager.h"
#include <lvgl.h>
#include <vector>

class CoursesPage : public Page {
public:
  void onCreate() override;
  void onDestroy() override;
  void onGpsUpdate(const GpsData& d) override;
  static void event_cb(lv_event_t* e);

  std::vector<lv_obj_t*> distSpinners_;

private:
  lv_obj_t* ledStatus_ = nullptr;
  std::vector<lv_obj_t*> btns_;
  std::vector<lv_obj_t*> lblDist_;

  void updateLabels(const GpsData& d);
};
