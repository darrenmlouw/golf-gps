#pragma once
#include <vector>
#include "Page.h"

/// Manages a stack of Pages.  The bottom (first) page is never popped.
class PageManager {
public:
  static PageManager& instance() {
    static PageManager inst;
    return inst;
  }

  /// Push a new page: show it, then tear down the old one.
  void pushPage(Page* p);

  /// Pop current page: show previous, then destroy the top one.
  void popPage();

private:
  PageManager() = default;
  std::vector<Page*> stack_;
};
