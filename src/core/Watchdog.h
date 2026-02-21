#pragma once
#include <cstdint>

class Watchdog {
public:
  void beginFrame();
  void addInstructions(uint32_t count);
  bool tripped() const;        // آیا بیش از حد مجاز شده؟
  void onFrameEnd();

private:
  uint32_t instrThisFrame_{0};
  uint32_t limit_{20000};      // بعداً قابل تنظیم از UI
  bool tripped_{false};
};