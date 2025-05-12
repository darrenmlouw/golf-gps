#include "Page.h"
#include "PageManager.h"
#include "Layout.h"
#include "GpsManager.h"  // <— pull in GPS
#include <Arduino.h>

static constexpr int BACK_BTN_SIZE = 36;
static constexpr int LED_SIZE = 12;

void Page::onDestroy() {
  // delete the GPS‐update timer
  if (gpsTimer_) {
    lv_timer_del(gpsTimer_);
    gpsTimer_ = nullptr;
  }

  // tear down the screen
  if (scr_) {
    lv_obj_del(scr_);
    scr_ = nullptr;
  }
}

void Page::createBase(const char* title, bool canGoBack) {
  // ─── full-screen black background ─────────────────────────────────
  scr_ = lv_obj_create(nullptr);
  static lv_style_t st_bg;
  lv_style_init(&st_bg);
  lv_style_set_bg_color(&st_bg, lv_color_black());
  lv_style_set_pad_all(&st_bg, 0);
  lv_obj_add_style(scr_, &st_bg, LV_PART_MAIN);

  // ─── header bar ────────────────────────────────────────────────────
  lv_coord_t th = lv_font_get_line_height(&lv_font_montserrat_48);
  lv_coord_t header_h = th + PAD;

  static lv_style_t st_hdr;
  static bool hdr_style_inited = false;
  if (!hdr_style_inited) {
    lv_style_init(&st_hdr);
    lv_style_set_bg_opa(&st_hdr, LV_OPA_TRANSP);
    lv_style_set_border_width(&st_hdr, 0);
    lv_style_set_pad_all(&st_hdr, 0);
    hdr_style_inited = true;
  }

  auto hdr = lv_obj_create(scr_);
  lv_obj_add_style(hdr, &st_hdr, LV_PART_MAIN);
  lv_obj_set_size(hdr, LCD_WIDTH, header_h);
  lv_obj_align(hdr, LV_ALIGN_TOP_LEFT, 0, PAD);

  // ─── back button ───────────────────────────────────────────────────
  if (canGoBack) {
    auto btn = lv_btn_create(hdr);
    lv_obj_set_size(btn, BACK_BTN_SIZE, BACK_BTN_SIZE);
    lv_obj_align(btn, LV_ALIGN_LEFT_MID, PAD, 0);
    lv_obj_add_event_cb(btn, backEventCallback, LV_EVENT_CLICKED, nullptr);

    auto lblB = lv_label_create(btn);
    lv_label_set_text(lblB, "<");
    lv_obj_set_style_text_font(lblB, &lv_font_montserrat_36, LV_PART_MAIN);
    lv_obj_set_style_text_color(lblB, lv_color_black(), LV_PART_MAIN);
    lv_obj_center(lblB);

    // keep swipe‐to‐go‐back on the whole screen
    lv_obj_add_event_cb(scr_, swipeEventCallback, LV_EVENT_GESTURE, nullptr);
  }

  // ─── LED status ────────────────────────────────────────────────────
  ledStatus_ = lv_led_create(hdr);
  lv_led_set_brightness(ledStatus_, 255);
  lv_led_on(ledStatus_);
  lv_obj_set_size(ledStatus_, LED_SIZE, LED_SIZE);
  lv_obj_align(ledStatus_, LV_ALIGN_RIGHT_MID, -PAD, 0);

  // ─── title ──────────────────────────────────────────────────────────
  // capture this for later updates
  hdrLabel_ = lv_label_create(hdr);
  lv_label_set_text(hdrLabel_, title);
  lv_obj_set_style_text_font(hdrLabel_, &lv_font_montserrat_48, LV_PART_MAIN);
  lv_obj_set_style_text_color(hdrLabel_, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(hdrLabel_, LV_ALIGN_CENTER, 0, 0);

  // ─── load it ────────────────────────────────────────────────────────
  lv_scr_load(scr_);

  {
    const auto d = GpsManager::instance().fetchData();
    lv_led_set_color(ledStatus_, getFixColor(d));
  }

  // now start the periodic timer for LED + onGpsUpdate()
  gpsTimer_ = lv_timer_create(
    [](lv_timer_t* tmr) {
      auto self = static_cast<Page*>(tmr->user_data);
      const auto d = GpsManager::instance().fetchData();
      lv_led_set_color(self->ledStatus_, self->getFixColor(d));
      self->onGpsUpdate(d);
    },
    200,
    this);
}

void Page::backEventCallback(lv_event_t* e) {
  lv_timer_create(
    [](lv_timer_t* tmr) {
      PageManager::instance().popPage();
      lv_timer_del(tmr);
    },
    1,
    nullptr);
}

void Page::swipeEventCallback(lv_event_t* e) {
  lv_indev_t* indev = lv_indev_get_act();
  if (lv_indev_get_gesture_dir(indev) == LV_DIR_RIGHT) {
    // 1) Wait until the touch is released
    lv_indev_wait_release(indev);
    // 2) Clear all leftover touch state
    lv_indev_reset(indev, nullptr);
    lv_timer_create(
      [](lv_timer_t* tmr) {
        PageManager::instance().popPage();
        lv_timer_del(tmr);
      },
      1,
      nullptr);
  }
}

lv_color_t Page::getFixColor(const GpsData& d) {
  if (!d.fix)
    return lv_palette_main(LV_PALETTE_RED);
  else if (d.fixQuality == 2)
    return lv_palette_main(LV_PALETTE_BLUE);  // Differential fix
  else if (d.hdop <= 1.5f)
    return lv_palette_main(LV_PALETTE_GREEN);  // High accuracy fix
  else
    return lv_palette_main(LV_PALETTE_ORANGE);  // Low accuracy fix
}
