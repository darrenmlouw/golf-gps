#include "PageManager.h"

void PageManager::pushPage(Page* p) {
  // 1) Remember the old page (if any)
  Page* old = stack_.empty() ? nullptr : stack_.back();

  // 2) Push & show the new page
  stack_.push_back(p);
  p->onCreate();  // lv_scr_load(new_scr) runs while old_scr is still valid

  // 3) Now tear down the old page’s UI
  if (old) old->onDestroy();
}

void PageManager::popPage() {
  if (stack_.size() <= 1) return;  // never pop the root

  // 1) Identify top & previous
  Page* top = stack_.back();
  Page* prev = stack_[stack_.size() - 2];

  // 2) Show the previous page *before* deleting the top
  prev->onCreate();  // lv_scr_load(prev_scr) runs while top_scr is still valid

  // 3) Now tear down & delete the top page
  top->onDestroy();
  delete top;
  stack_.pop_back();
}
