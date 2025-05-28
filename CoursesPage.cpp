#include "CoursesPage.h"
#include "PageManager.h"
#include "pin_config.h"  // for LCD_WIDTH
#include "Layout.h"
#include <Arduino.h>
#include "HolePage.h"
#include <cmath>
#include <numeric>

// haversine helper
static constexpr double R_earth = 6'371'000.0;
static double haversine(double lat1, double lon1,
                        double lat2, double lon2) {
  auto toRad = [](double d) {
    return d * M_PI / 180.0;
  };
  double dlat = toRad(lat2 - lat1), dlon = toRad(lon2 - lon1);
  double a = sin(dlat / 2) * sin(dlat / 2)
             + cos(toRad(lat1)) * cos(toRad(lat2))
                 * sin(dlon / 2) * sin(dlon / 2);
  return R_earth * 2 * atan2(sqrt(a), sqrt(1 - a));
}

static constexpr int BTN_H = 80;

void CoursesPage::onCreate() {
  createBase("Courses", true);

  // fetch & sort
  auto& courses = CoursesManager::instance().getCourses();
  std::vector<int> idx(courses.size());
  std::iota(idx.begin(), idx.end(), 0);
  auto gps = GpsManager::instance().fetchData();
  if (gps.fix) {
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
      double da = std::pow(courses[a].location.lat - gps.lat, 2)
                  + std::pow(courses[a].location.lon - gps.lon, 2);
      double db = std::pow(courses[b].location.lat - gps.lat, 2)
                  + std::pow(courses[b].location.lon - gps.lon, 2);
      return da < db;
    });
  } else {
    std::sort(idx.begin(), idx.end(),
              [&](int a, int b) {
                return courses[a].name < courses[b].name;
              });
  }

  // button style
  static lv_style_t st_btn;
  lv_style_init(&st_btn);
  lv_style_set_bg_color(&st_btn, lv_color_white());
  lv_style_set_bg_opa(&st_btn, LV_OPA_80);
  lv_style_set_radius(&st_btn, CORNER);

  // build list
  lv_coord_t y0 = PAD + lv_font_get_line_height(&lv_font_montserrat_48) + PAD;
  for (size_t i = 0; i < idx.size(); ++i) {
    int ci = idx[i];
    auto btn = lv_btn_create(scr_);
    lv_obj_add_style(btn, &st_btn, LV_PART_MAIN);
    lv_obj_set_size(btn, LCD_WIDTH - 2 * PAD, BTN_H);
    lv_obj_align(btn, LV_ALIGN_TOP_MID, 0, y0 + i * (BTN_H + PAD));
    lv_obj_set_user_data(btn, (void*)(intptr_t)ci);
    lv_obj_add_event_cb(btn, CoursesPage::event_cb, LV_EVENT_SHORT_CLICKED, this);
    btns_.push_back(btn);

    // name
    auto nm = lv_label_create(btn);
    lv_label_set_text(nm, courses[ci].name.c_str());
    lv_obj_set_style_text_font(nm, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(nm, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(nm, LV_ALIGN_LEFT_MID, 16, 0);

    // distance label (initially hidden)
    auto dl = lv_label_create(btn);
    lv_label_set_text(dl, "");
    lv_obj_set_style_text_font(dl, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(dl, lv_color_black(), LV_PART_MAIN);
    lv_obj_align(dl, LV_ALIGN_RIGHT_MID, -8, 0);
    lblDist_.push_back(dl);

    // spinner (initially shown)
    auto sp = lv_spinner_create(btn, 1000 + i*(61), 240);
    lv_obj_set_size(sp, 32, 32);
    lv_obj_align(sp, LV_ALIGN_RIGHT_MID, -8, 0);

    // Spinning arc (active part)
    lv_obj_set_style_arc_width(sp, 6, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(sp, lv_palette_main(LV_PALETTE_BLUE), LV_PART_INDICATOR);

    // Background ring
    lv_obj_set_style_arc_width(sp, 6, LV_PART_MAIN);
    lv_obj_set_style_arc_color(sp, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_arc_opa(sp, LV_OPA_TRANSP, LV_PART_MAIN);

    distSpinners_.push_back(sp);
  }

  const auto d = GpsManager::instance().fetchData();
  onGpsUpdate(d);
}

void CoursesPage::onDestroy() {
  Page::onDestroy();

  btns_.clear();
  lblDist_.clear();
  distSpinners_.clear();
}

void CoursesPage::event_cb(lv_event_t* e) {
  if (lv_event_get_code(e) != LV_EVENT_SHORT_CLICKED) return;
  auto btn = lv_event_get_target(e);

  int ci = (int)(intptr_t)lv_obj_get_user_data(btn);
  // Push into HolePage for that course
  PageManager::instance().pushPage(new HolePage(ci));
}

void CoursesPage::onGpsUpdate(const GpsData& d) {
  updateLabels(d);
}

void CoursesPage::updateLabels(const GpsData& d) {
  auto& courses = CoursesManager::instance().getCourses();
  for (size_t i = 0; i < btns_.size(); ++i) {
    int ci = (int)(intptr_t)lv_obj_get_user_data(btns_[i]);
    if (d.fix) {
      double m = haversine(
        d.lat, d.lon,
        courses[ci].location.lat,
        courses[ci].location.lon);
      char buf[16];
      if (m >= 1000) snprintf(buf, sizeof(buf), "%.1f km", m / 1000.0);
      else snprintf(buf, sizeof(buf), "%.0f m", m);
      lv_label_set_text(lblDist_[i], buf);
      lv_obj_clear_flag(lblDist_[i], LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(distSpinners_[i], LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(lblDist_[i], LV_OBJ_FLAG_HIDDEN);
      lv_obj_clear_flag(distSpinners_[i], LV_OBJ_FLAG_HIDDEN);
    }
  }
}
