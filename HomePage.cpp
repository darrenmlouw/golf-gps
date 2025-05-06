#include "HomePage.h"
#include "PageManager.h"
#include "CoursesPage.h"
#include "LocationPage.h"
#include "Layout.h"
#include "icons.h"

static constexpr int BTN_H = 80;

void HomePage::onCreate() {
  // root, no back button
  createBase("GOLF GPS", /*showBack=*/false);

  // Prepare a reusable button style
  static lv_style_t st_btn;
  static bool btn_style_inited = false;
  if (!btn_style_inited) {
    lv_style_init(&st_btn);
    lv_style_set_bg_color(&st_btn, lv_color_white());
    lv_style_set_bg_opa(&st_btn, LV_OPA_80);
    lv_style_set_radius(&st_btn, LV_RADIUS_CIRCLE);
    btn_style_inited = true;
  }

  // Compute top-of-buttons y-pos (just under the title)
  lv_coord_t th = lv_font_get_line_height(&lv_font_montserrat_48);
  lv_coord_t y0 = PAD + th + PAD;

  // Courses button
   btnCourses_ = lv_btn_create(scr_);
  lv_obj_add_style(btnCourses_, &st_btn, LV_PART_MAIN);
  lv_obj_set_size(btnCourses_, LCD_WIDTH - 2 * PAD, BTN_H);
  lv_obj_align(btnCourses_, LV_ALIGN_TOP_MID, 0, y0);
  lv_obj_add_event_cb(btnCourses_, event_cb, LV_EVENT_CLICKED, this);

  auto iconCourses = lv_img_create(btnCourses_);
  lv_img_set_src(iconCourses, &golf_50);
  lv_obj_align(iconCourses, LV_ALIGN_LEFT_MID, 8, 0);

  auto l1 = lv_label_create(btnCourses_);
  lv_label_set_text(l1, "Courses");
  lv_obj_set_style_text_font(l1, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(l1, lv_color_black(), LV_PART_MAIN);
  lv_obj_align(l1, LV_ALIGN_LEFT_MID, 80, 0);

  // Location button
  btnLocation_ = lv_btn_create(scr_);
  lv_obj_add_style(btnLocation_, &st_btn, LV_PART_MAIN);
  lv_obj_set_size(btnLocation_, LCD_WIDTH - 2 * PAD, BTN_H);
  lv_obj_align(btnLocation_, LV_ALIGN_TOP_MID, 0, y0 + BTN_H + PAD);
  lv_obj_add_event_cb(btnLocation_, event_cb, LV_EVENT_CLICKED, this);

  auto iconLocation = lv_img_create(btnLocation_);
  lv_img_set_src(iconLocation, &gps_50);
  lv_obj_align(iconLocation, LV_ALIGN_LEFT_MID, 8, 0);

  auto l2 = lv_label_create(btnLocation_);
  lv_label_set_text(l2, "Location");
  lv_obj_set_style_text_font(l2, &lv_font_montserrat_32, LV_PART_MAIN);
  lv_obj_set_style_text_color(l2, lv_color_black(), LV_PART_MAIN);
  lv_obj_align(l2, LV_ALIGN_LEFT_MID, 80, 0);

  const auto d = GpsManager::instance().getData();
  onGpsUpdate(d);
}

void HomePage::onDestroy() {
  // First tear down any HomePage‐specific state (none in this case)
  // Then call the base‐class cleanup to kill the timer + delete the screen:
  Page::onDestroy();
}
void HomePage::event_cb(lv_event_t* e) {
  if (lv_event_get_code(e) != LV_EVENT_CLICKED) return;
  auto btn = lv_event_get_target(e);
  auto self = static_cast<HomePage*>(lv_event_get_user_data(e));
  if (btn == self->btnCourses_) {
    PageManager::instance().pushPage(new CoursesPage());
  } else if (btn == self->btnLocation_) {
    PageManager::instance().pushPage(new LocationPage());
  }
}
