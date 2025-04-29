#include "HomePage.h"
#include "PageManager.h"
#include "GpsViewPage.h"
#include <Arduino.h>
#include <lvgl.h>

void HomePage::event_cb(lv_event_t *e) {
  // recover our 'this'
  HomePage* self = static_cast<HomePage*>(lv_event_get_user_data(e));
  lv_obj_t* targ = lv_event_get_target(e);

  if(targ == self->btnCourses_) {
    Serial.println("🔹 Courses clicked");
    // defer push in 1 ms
    lv_timer_t* t = lv_timer_create([](lv_timer_t* tmr){
      // inside timer callback, LVGL is idle
      // PageManager::instance().pushPage(new CoursesPage());
      lv_timer_del(tmr);
    }, 1, nullptr);

  } else if(targ == self->btnGpsView_) {
    Serial.println("🔹 GPS View clicked");
    // defer GPS view push by one tick
    lv_timer_t* t = lv_timer_create([](lv_timer_t* tmr){
      PageManager::instance().pushPage(new GpsViewPage());
      lv_timer_del(tmr);
    }, 1, nullptr);
  }
}

void HomePage::onCreate() {
  // 1) Build screen
  scr_ = lv_obj_create(nullptr);
  lv_obj_set_style_bg_color(scr_, lv_color_black(), 0);
  lv_obj_set_style_bg_opa  (scr_, LV_OPA_COVER,  0);

  // 2) Title
  {
    auto lbl = lv_label_create(scr_);
    lv_label_set_text(lbl, "Golf GPS Home");
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_set_style_text_font (lbl, &lv_font_montserrat_28, 0);
    lv_obj_align(lbl, LV_ALIGN_TOP_MID, 0, 16);
  }

  // 3) “Courses” button
  btnCourses_ = lv_btn_create(scr_);
  lv_obj_set_size(btnCourses_, 200, 80);
  lv_obj_align   (btnCourses_, LV_ALIGN_CENTER, 0, -60);
  lv_obj_add_event_cb(
    btnCourses_,
    HomePage::event_cb,
    LV_EVENT_CLICKED,
    this           // <-- pass our pointer
  );
  {
    auto l = lv_label_create(btnCourses_);
    lv_label_set_text(l, "Courses");
    lv_obj_center(l);
  }

  // 4) “GPS View” button
  btnGpsView_ = lv_btn_create(scr_);
  lv_obj_set_size(btnGpsView_, 200, 80);
  lv_obj_align   (btnGpsView_, LV_ALIGN_CENTER, 0, +60);
  lv_obj_add_event_cb(
    btnGpsView_,
    HomePage::event_cb,
    LV_EVENT_CLICKED,
    this           // <-- pass us again
  );
  {
    auto l = lv_label_create(btnGpsView_);
    lv_label_set_text(l, "GPS View");
    lv_obj_center(l);
  }

  // 6) Show it
  lv_scr_load(scr_);
}

void HomePage::onDestroy() {
  if(scr_) {
    lv_obj_del(scr_);
    scr_ = nullptr;
  }
}
