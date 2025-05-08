#include "LocationPage.h"
#include "PageManager.h"
#include "pin_config.h"  // for LCD_WIDTH, LCD_HEIGHT
#include "Layout.h"
#include <Arduino.h>

static constexpr int PANEL_H = 120;
static constexpr int PANEL_PAD = 0;

void LocationPage::onCreate() {
  createBase("Location", true);

  // 2) panel style (glass-morphic)
  static lv_style_t style_panel;
  static bool style_inited = false;
  if (!style_inited) {
    lv_style_init(&style_panel);
    lv_style_set_bg_color(&style_panel, lv_color_hex(0x1F1F2C));
    lv_style_set_bg_opa(&style_panel, LV_OPA_40);
    lv_style_set_border_color(&style_panel, lv_color_white());
    lv_style_set_border_width(&style_panel, 2);
    lv_style_set_radius(&style_panel, 12);
    style_inited = true;
  }

  // 3) Raw panel
  panelRaw_ = lv_obj_create(scr_);
  lv_obj_add_style(panelRaw_, &style_panel, LV_PART_MAIN);
  lv_obj_set_size(panelRaw_, LCD_WIDTH - 2 * PAD, PANEL_H);
  lv_obj_align(panelRaw_, LV_ALIGN_TOP_LEFT,
               PAD, PAD + lv_font_get_line_height(&lv_font_montserrat_48) + PAD);

  auto h1 = lv_label_create(panelRaw_);
  lv_label_set_text(h1, "Raw Loc:");
  lv_obj_set_style_text_color(h1, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(h1, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_align(h1, LV_ALIGN_TOP_LEFT, PANEL_PAD, PANEL_PAD);

  lblRawLat_ = lv_label_create(panelRaw_);
  lv_label_set_text(lblRawLat_, "Lat: --");
  lv_obj_set_style_text_color(lblRawLat_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblRawLat_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblRawLat_, h1, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lblRawLon_ = lv_label_create(panelRaw_);
  lv_label_set_text(lblRawLon_, "Lon: --");
  lv_obj_set_style_text_color(lblRawLon_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblRawLon_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblRawLon_, lblRawLat_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

  // 4) Smoothed panel (reuse same style)
  panelSm_ = lv_obj_create(scr_);
  lv_obj_add_style(panelSm_, &style_panel, LV_PART_MAIN);
  lv_obj_set_size(panelSm_, LCD_WIDTH - 2 * PAD, PANEL_H);
  lv_obj_align_to(panelSm_, panelRaw_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 16);

  auto h2 = lv_label_create(panelSm_);
  lv_label_set_text(h2, "Smoothed:");
  lv_obj_set_style_text_color(h2, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(h2, &lv_font_montserrat_24, LV_PART_MAIN);
  lv_obj_align(h2, LV_ALIGN_TOP_LEFT, PANEL_PAD, PANEL_PAD);

  lblSmLat_ = lv_label_create(panelSm_);
  lv_label_set_text(lblSmLat_, "Lat: --");
  lv_obj_set_style_text_color(lblSmLat_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblSmLat_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblSmLat_, h2, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 8);

  lblSmLon_ = lv_label_create(panelSm_);
  lv_label_set_text(lblSmLon_, "Lon: --");
  lv_obj_set_style_text_color(lblSmLon_, lv_color_white(), LV_PART_MAIN);
  lv_obj_set_style_text_font(lblSmLon_, &lv_font_montserrat_20, LV_PART_MAIN);
  lv_obj_align_to(lblSmLon_, lblSmLat_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

  // 5) Footer
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

  const auto d = GpsManager::instance().fetchData();
  onGpsUpdate(d);
}


void LocationPage::onDestroy() {
  // this will delete the lv_timer, the scr_ and null them out
  Page::onDestroy();
}

void LocationPage::onGpsUpdate(const GpsData& d) {
  updateLabels(d);
}

void LocationPage::updateLabels(const GpsData& d) {
  // Raw
  if (d.fix) {
    lv_label_set_text_fmt(lblRawLat_, "Lat: %.8f", d.lat);
    lv_label_set_text_fmt(lblRawLon_, "Lon: %.8f", d.lon);
  } else {
    lv_label_set_text(lblRawLat_, "Lat: --");
    lv_label_set_text(lblRawLon_, "Lon: --");
  }

  // EMA smoothing
  if (d.fix && d.hdop > 0 && d.hdop <= 3.0f) {
    if (!emaInit_) {
      emaInit_ = true;
      emaLat_ = d.lat;
      emaLon_ = d.lon;
    } else {
      emaLat_ = alpha_ * d.lat + (1 - alpha_) * emaLat_;
      emaLon_ = alpha_ * d.lon + (1 - alpha_) * emaLon_;
    }
    lv_label_set_text_fmt(lblSmLat_, "Lat: %.6f", emaLat_);
    lv_label_set_text_fmt(lblSmLon_, "Lon: %.6f", emaLon_);
  } else {
    lv_label_set_text(lblSmLat_, "Lat: --");
    lv_label_set_text(lblSmLon_, "Lon: --");
  }

  // Footer + LED
  if (d.fix) {
    lv_label_set_text_fmt(lblHdop_, "HDOP: %.1f", d.hdop);
    lv_label_set_text_fmt(lblSats_, "Sats: %d", d.sats);
  } else {
    lv_label_set_text(lblHdop_, "HDOP: --");
    lv_label_set_text(lblSats_, "Sats: --");
  }
}
