#pragma once
#include "Page.h"
#include "GpsManager.h"
#include <lvgl.h>
#include "pin_config.h"  // for LCD_WIDTH, LCD_HEIGHT

/// Show live GPS data (raw + smoothed) in two bordered panels
class GpsViewPage : public Page {
public:
  void onCreate() override;
  void onDestroy() override;

private:
  // layout constants
  static constexpr int PAD       = 24;
  static constexpr int CORNER    = 12;
  static constexpr int PANEL_H   = 120;
  static constexpr int PANEL_PAD = 0;

  // LVGL objects
  lv_obj_t* scr_        = nullptr;
  lv_obj_t* lblTitle_   = nullptr;
  lv_obj_t* panelRaw_   = nullptr;
  lv_obj_t* lblRawLat_  = nullptr;
  lv_obj_t* lblRawLon_  = nullptr;
  lv_obj_t* panelSm_    = nullptr;
  lv_obj_t* lblSmLat_   = nullptr;
  lv_obj_t* lblSmLon_   = nullptr;
  lv_obj_t* lineHR_     = nullptr;
  lv_obj_t* ledStatus_  = nullptr;
  lv_obj_t* lblHdop_    = nullptr;
  lv_obj_t* lblSats_    = nullptr;

  // smoothing state (EMA)
  bool   emaInit_ = false;
  float  emaLat_  = 0;
  float  emaLon_  = 0;
  static constexpr float alpha_ = 0.2f;

  void updateLabels(const GpsData& d);
};
