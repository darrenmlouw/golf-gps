#include "GpsViewPage.h"
#include "PageManager.h"
#include "pin_config.h"    // for LCD_WIDTH, LCD_HEIGHT
#include <lvgl.h>
#include <Arduino.h>

// Layout constants
static constexpr int PAD      = 24;
static constexpr int CORNER   = 12;
static constexpr int PANEL_H  = 120;
static constexpr int PANEL_PAD = 0;

// “< Back” button callback
static void back_cb(lv_event_t* e) {
  PageManager::instance().popPage();
}

void GpsViewPage::onCreate() {
  // 1) Create the base screen
  scr_ = lv_obj_create(nullptr);

  // 2) Dark gradient background
  static lv_style_t style_screen;
  lv_style_init(&style_screen);
  lv_style_set_bg_color     (&style_screen, lv_color_black());
  // lv_style_set_bg_grad_color(&style_screen, lv_color_hex(0x1C1D2A));
  // lv_style_set_bg_grad_dir  (&style_screen, LV_GRAD_DIR_VER);
  lv_obj_add_style(scr_, &style_screen, LV_PART_MAIN);

  // 3) Screen padding
  lv_obj_set_style_pad_column(scr_, PAD, LV_PART_MAIN);
  lv_obj_set_style_pad_row   (scr_, PAD, LV_PART_MAIN);

  lv_coord_t th = lv_font_get_line_height(&lv_font_montserrat_28);

  // 4) Title in top‐center
  lblTitle_ = lv_label_create(scr_);
  lv_label_set_text(lblTitle_, "Golf GPS");
  lv_obj_set_style_text_color(lblTitle_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font (lblTitle_, &lv_font_montserrat_28, LV_PART_MAIN);
  lv_obj_align(lblTitle_, LV_ALIGN_TOP_MID, 0, PAD);

  // 5) Back-button in top-left, same vertical y=PAD
  {
    static lv_style_t style_back;
    lv_style_init(&style_back);
    lv_style_set_bg_color(&style_back,     lv_color_white());
    lv_style_set_bg_opa  (&style_back,     LV_OPA_70);
    lv_style_set_border_color(&style_back, lv_color_white());
    lv_style_set_border_width(&style_back, 1);
    lv_style_set_radius(&style_back,       CORNER/2);

    auto btn = lv_btn_create(scr_);
    lv_obj_add_style(btn, &style_back, LV_PART_MAIN);
    lv_obj_set_size(btn, th, th);
    // align left at x=PAD, y=PAD
    lv_obj_align(btn, LV_ALIGN_TOP_LEFT, PAD, PAD);
    lv_obj_add_event_cb(btn, back_cb, LV_EVENT_CLICKED, nullptr);

    auto lbl = lv_label_create(btn);
    lv_label_set_text(lbl, "<");
    lv_obj_set_style_text_color(lbl, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font (lbl, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_center(lbl);
  }

  // 6) Status LED in top-right, same vertical y=PAD
  ledStatus_ = lv_led_create(scr_);
  lv_obj_set_size(ledStatus_, 12, 12);
  // exactly PAD pixels from the top and PAD pixels from the right
  lv_obj_align(ledStatus_, LV_ALIGN_TOP_RIGHT, -PAD-6, PAD+12);

  // 6) Glass-morphic panel style
  static lv_style_t style_panel;
  lv_style_init(&style_panel);
  lv_style_set_bg_color(&style_panel, lv_color_hex(0x1F1F2C));
  lv_style_set_bg_opa  (&style_panel, LV_OPA_40);
  lv_style_set_border_color(&style_panel, lv_color_white());
  lv_style_set_border_width(&style_panel, 2);
  lv_style_set_radius(&style_panel, CORNER);

  // Raw panel
  panelRaw_ = lv_obj_create(scr_);
  lv_obj_add_style(panelRaw_, &style_panel, LV_PART_MAIN);
  lv_obj_set_size(panelRaw_, LCD_WIDTH - 2*PAD, PANEL_H);
  lv_obj_align(panelRaw_, LV_ALIGN_TOP_LEFT,
               PAD, PAD + lv_font_get_line_height(&lv_font_montserrat_28) + PAD);

  // Raw header
  {
    auto h = lv_label_create(panelRaw_);
    lv_label_set_text(h, "Raw Loc:");
    lv_obj_set_style_text_color(h, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(h, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(h, LV_ALIGN_TOP_LEFT, PANEL_PAD, PANEL_PAD);
  }
  // Raw Lat / Lon
  lblRawLat_ = lv_label_create(panelRaw_);
  lv_label_set_text(lblRawLat_, "Lat: --");
  lv_obj_set_style_text_color(lblRawLat_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblRawLat_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblRawLat_, lv_obj_get_child(panelRaw_, 0),
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lblRawLon_ = lv_label_create(panelRaw_);
  lv_label_set_text(lblRawLon_, "Lon: --");
  lv_obj_set_style_text_color(lblRawLon_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblRawLon_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblRawLon_, lblRawLat_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

  // Smoothed panel
  panelSm_ = lv_obj_create(scr_);
  lv_obj_add_style(panelSm_, &style_panel, LV_PART_MAIN);
  lv_obj_set_size(panelSm_, LCD_WIDTH - 2*PAD, PANEL_H);
  lv_obj_align_to(panelSm_, panelRaw_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

  // Smoothed header
  {
    auto h = lv_label_create(panelSm_);
    lv_label_set_text(h, "Smoothed:");
    lv_obj_set_style_text_color(h, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(h, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_align(h, LV_ALIGN_TOP_LEFT, PANEL_PAD, PANEL_PAD);
  }
  // Smoothed Lat / Lon
  lblSmLat_ = lv_label_create(panelSm_);
  lv_label_set_text(lblSmLat_, "Lat: --");
  lv_obj_set_style_text_color(lblSmLat_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblSmLat_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblSmLat_, lv_obj_get_child(panelSm_, 0),
                  LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lblSmLon_ = lv_label_create(panelSm_);
  lv_label_set_text(lblSmLon_, "Lon: --");
  lv_obj_set_style_text_color(lblSmLon_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblSmLon_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblSmLon_, lblSmLat_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

  // 7) Separator line
  {
    static lv_point_t pts[] = {
      { PAD,               LCD_HEIGHT - PAD - 40 },
      { LCD_WIDTH - PAD,   LCD_HEIGHT - PAD - 40 }
    };
    lineHR_ = lv_line_create(scr_);
    lv_line_set_points(lineHR_, pts, 2);
    lv_obj_set_style_line_color(lineHR_, lv_color_hex(0x646472), LV_PART_MAIN);
    lv_obj_set_style_line_width(lineHR_, 1, LV_PART_MAIN);
  }

  // 9) Footer: HDOP & Sats
  lblHdop_ = lv_label_create(scr_);
  lv_label_set_text(lblHdop_, "HDOP: --");
  lv_obj_set_style_text_color(lblHdop_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblHdop_, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_align(lblHdop_, LV_ALIGN_BOTTOM_LEFT, PAD, -PAD);

  lblSats_ = lv_label_create(scr_);
  lv_label_set_text(lblSats_, "Sats: --");
  lv_obj_set_style_text_color(lblSats_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblSats_, &lv_font_montserrat_18, LV_PART_MAIN);
  lv_obj_align(lblSats_, LV_ALIGN_BOTTOM_MID, 0, -PAD);

  // 10) Hook up GPS updates
  GpsManager::instance().setCallback(
    [this](const GpsData& d){ updateLabels(d); }
  );

  // 11) Swipe-right gesture to go back
  lv_obj_add_event_cb(scr_,
    [](lv_event_t* e){
      if(lv_indev_get_gesture_dir(lv_indev_get_act()) == LV_DIR_RIGHT)
        PageManager::instance().popPage();
    },
    LV_EVENT_GESTURE, nullptr
  );

  // 12) Finally show the screen
  lv_scr_load(scr_);
}

void GpsViewPage::onDestroy() {
  if(scr_) {
    lv_obj_del(scr_);
    scr_ = nullptr;
  }
  GpsManager::instance().setCallback(nullptr);
  emaInit_ = false;
}

void GpsViewPage::updateLabels(const GpsData& d) {
  // Raw values
  if(d.fix) {
    lv_label_set_text_fmt(lblRawLat_, "Lat: %.8f", d.lat);
    lv_label_set_text_fmt(lblRawLon_, "Lon: %.8f", d.lon);
  } else {
    lv_label_set_text(lblRawLat_, "Lat: --");
    lv_label_set_text(lblRawLon_, "Lon: --");
  }

  // Exponential smoothing when HDOP ≤ 3
  if(d.fix && d.hdop > 0 && d.hdop <= 3.0f) {
    if(!emaInit_) {
      emaInit_ = true;
      emaLat_  = d.lat;
      emaLon_  = d.lon;
    } else {
      emaLat_ = alpha_*d.lat + (1 - alpha_)*emaLat_;
      emaLon_ = alpha_*d.lon + (1 - alpha_)*emaLon_;
    }
    lv_label_set_text_fmt(lblSmLat_, "Lat: %.6f", emaLat_);
    lv_label_set_text_fmt(lblSmLon_, "Lon: %.6f", emaLon_);
  } else {
    lv_label_set_text(lblSmLat_, "Lat: --");
    lv_label_set_text(lblSmLon_, "Lon: --");
  }

  // HDOP/Sats footer and LED color
  if(d.fix) {
    lv_label_set_text_fmt(lblHdop_, "HDOP: %.1f", d.hdop);
    lv_label_set_text_fmt(lblSats_, "Sats: %d", d.sats);
  } else {
    lv_label_set_text(lblHdop_, "HDOP: --");
    lv_label_set_text(lblSats_, "Sats: --");
  }
  lv_led_set_color(ledStatus_,
    lv_palette_main(d.fix ? LV_PALETTE_GREEN : LV_PALETTE_RED)
  );
}
