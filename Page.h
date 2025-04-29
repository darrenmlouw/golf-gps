#pragma once
#include <lvgl.h>

/// Base class for any “screen/page”
class Page {
  public:
    virtual ~Page() = default;
    /// Called when this page is pushed onto the stack
    virtual void onCreate() = 0;
    /// Called when this page is popped off the stack
    virtual void onDestroy() = 0;
  protected:
    lv_obj_t* scr_ = nullptr;
};
