#pragma once
#include <vector>
#include <memory>
#include "Page.h"

class PageManager {
public:
  static PageManager& instance() {
    static PageManager inst;
    return inst;
  }

  /// Push a new page (we own the pointer)
  void pushPage(Page* p) {
    // tear down the old *after* we show the new
    Page* old = current();
    stack_.emplace_back(p);
    p->onCreate();
    if(old) old->onDestroy();
  }

  /// Pop current page and go back
  void popPage() {
    // never pop the root
    if(stack_.size() <= 1) return;

    Page* old  = stack_.back().get();
    Page* prev = stack_[stack_.size() - 2].get();

    // 1) bring the previous page back into view
    prev->onCreate();

    // 2) now tear down the top page
    old->onDestroy();
    stack_.pop_back();
  }

private:
  PageManager() = default;
  Page* current() const {
    return stack_.empty() ? nullptr : stack_.back().get();
  }
  std::vector<std::unique_ptr<Page>> stack_;
};
