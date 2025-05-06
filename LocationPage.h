#pragma once
#include "Page.h"
#include "GpsManager.h"
#include <lvgl.h>

class LocationPage : public Page {
public:
  void onCreate() override;
  void onDestroy() override;
  void onGpsUpdate(const GpsData& d) override;

private:
  // raw data panel
  lv_obj_t* panelRaw_    = nullptr;
  lv_obj_t* lblRawLat_   = nullptr;
  lv_obj_t* lblRawLon_   = nullptr;

  // smoothed data panel
  lv_obj_t* panelSm_     = nullptr;
  lv_obj_t* lblSmLat_    = nullptr;
  lv_obj_t* lblSmLon_    = nullptr;

  // footer/status
  lv_obj_t* ledStatus_   = nullptr;
  lv_obj_t* lblHdop_     = nullptr;
  lv_obj_t* lblSats_     = nullptr;

  // smoothing state
  bool emaInit_          = false;
  float emaLat_          = 0.0f;
  float emaLon_          = 0.0f;
  static constexpr float alpha_ = 0.2f;

  void updateLabels(const GpsData& d);
};
