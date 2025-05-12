#include "HolePage.h"
#include "PageManager.h"
#include "GpsManager.h"
#include "Layout.h"        // for LCD_WIDTH, PAD
#include <lvgl.h>
#include <algorithm>       // for std::clamp
#include <cmath>           // for haversine
#include <cstdio>          // for snprintf
#include "lv_font_montserrat_64.h"
#include "lv_font_montserrat_56.h"


using std::clamp;

// ─── haversine helper ───────────────────────────────────────────────────
static constexpr double R_earth = 6'371'000.0;
static double haversine(double lat1, double lon1,
                        double lat2, double lon2) {
  auto toRad = [](double d){ return d * M_PI/180.0; };
  double dlat = toRad(lat2 - lat1), dlon = toRad(lon2 - lon1);
  double a = sin(dlat/2)*sin(dlat/2)
           + cos(toRad(lat1))*cos(toRad(lat2))
             * sin(dlon/2)*sin(dlon/2);
  return R_earth * 2 * atan2(sqrt(a), sqrt(1 - a));
}

HolePage::HolePage(int courseIdx)
  : courseIdx_(courseIdx) {}

void HolePage::onCreate() {
  // 1) Initial header string "#1  Par 4"
  char buf[32];
  auto& holes = CoursesManager::instance()
                             .getCourses()[courseIdx_].holes;
  if (!holes.empty()) {
    snprintf(buf, sizeof(buf), "#%d  Par %d",
             holes[0].number, holes[0].par);
  } else {
    strcpy(buf, "No hole data");
  }

  // 2) Create base (this sets hdrLabel_)
  createBase(buf, true);
  lv_obj_set_style_text_font(hdrLabel_, &lv_font_montserrat_36, LV_PART_MAIN);

  // 3) Swipe up/down
  lv_obj_add_event_cb(scr_,
                      HolePage::gestureCb,
                      LV_EVENT_GESTURE,
                      this);

  // 4) Prepare distance labels
  lv_coord_t fh = lv_font_get_line_height(&lv_font_montserrat_64);
  int quarter = LCD_WIDTH / 4;

  // FRONT
  lblFront_ = lv_label_create(scr_);
  lv_obj_set_width(lblFront_, LCD_WIDTH/2 - PAD*2);
  lv_obj_set_style_text_align(lblFront_, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_style_text_font(lblFront_, &lv_font_montserrat_56, LV_PART_MAIN);
  lv_obj_set_style_text_color(lblFront_, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(lblFront_, LV_ALIGN_CENTER, -quarter, -fh);

  // MID
  lblMid_ = lv_label_create(scr_);
  lv_obj_set_width(lblMid_, LCD_WIDTH/2 - PAD*2);
  lv_obj_set_style_text_align(lblMid_, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_style_text_font(lblMid_, &lv_font_montserrat_64, LV_PART_MAIN);
  lv_obj_set_style_text_color(lblMid_, lv_color_hex(0xEFBF04), LV_PART_MAIN);
  lv_obj_align(lblMid_, LV_ALIGN_CENTER, -quarter, 0);

  // BACK
  lblBack_ = lv_label_create(scr_);
  lv_obj_set_width(lblBack_, LCD_WIDTH/2 - PAD*2);
  lv_obj_set_style_text_align(lblBack_, LV_TEXT_ALIGN_RIGHT, LV_PART_MAIN);
  lv_obj_set_style_text_font(lblBack_, &lv_font_montserrat_56, LV_PART_MAIN);
  lv_obj_set_style_text_color(lblBack_, lv_color_white(), LV_PART_MAIN);
  lv_obj_align(lblBack_, LV_ALIGN_CENTER, -quarter, +fh);

  // 5) Show hole #0
  navigateTo(0);
  updateDistances(GpsManager::instance().fetchData());
}

void HolePage::onDestroy() {
  Page::onDestroy();
}

void HolePage::navigateTo(int newIdx) {
  auto& holes = CoursesManager::instance()
                             .getCourses()[courseIdx_].holes;
  if (holes.empty()) return;

  holeIdx_ = clamp(newIdx, 0, int(holes.size()) - 1);
  const auto& hole = holes[holeIdx_];

  lv_label_set_text_fmt(
    hdrLabel_,
    "#%d  Par %d",
    hole.number,
    hole.par
  );
}

void HolePage::onGpsUpdate(const GpsData& d) {
  updateDistances(d);
}

void HolePage::updateDistances(const GpsData& d) {
  auto& holes = CoursesManager::instance()
                             .getCourses()[courseIdx_].holes;
  if (holes.empty()) return;
  const auto& hole = holes[holeIdx_];

  // ensure labels visible
  lv_obj_clear_flag(lblFront_, LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(lblMid_,   LV_OBJ_FLAG_HIDDEN);
  lv_obj_clear_flag(lblBack_,  LV_OBJ_FLAG_HIDDEN);

  if (d.fix) {
    double df = haversine(d.lat, d.lon, hole.front.lat, hole.front.lon);
    double db = haversine(d.lat, d.lon, hole.back.lat,  hole.back.lon);
    double dm = (df + db) / 2.0;

    char buf[16];
    auto fmt = [&](double m) {
      if (m >= 1000.0) {
        // show kilometers to 1 decimal, e.g. "1.2"
        snprintf(buf, sizeof(buf), "%.1f", m / 1000.0);
      } else {
        // show meters as integer, e.g. "999"
        snprintf(buf, sizeof(buf), "%d", int(m));
      }
      return buf;
    };

    lv_label_set_text(lblFront_, fmt(df));
    lv_label_set_text(lblMid_,   fmt(dm));
    lv_label_set_text(lblBack_,  fmt(db));
  }
  else {
    // no fix: placeholders
    lv_label_set_text(lblFront_, "360");
    lv_label_set_text(lblMid_,   "345");
    lv_label_set_text(lblBack_,  "329");
  }
}

void HolePage::gestureCb(lv_event_t* e) {
  auto self = static_cast<HolePage*>(lv_event_get_user_data(e));
  lv_indev_t* indev = lv_indev_get_act();
  lv_indev_wait_release(indev);
  lv_indev_reset(indev, nullptr);
  lv_dir_t dir = lv_indev_get_gesture_dir(indev);
  if (dir == LV_DIR_TOP)
    self->navigateTo(self->holeIdx_ + 1);
  else if (dir == LV_DIR_BOTTOM)
    self->navigateTo(self->holeIdx_ - 1);
}
