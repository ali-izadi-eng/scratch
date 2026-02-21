#pragma once
#include <cstdint>

class Time {
public:
  static void tick();                 // هر فریم صدا زده شود
  static float deltaSeconds();        // dt
  static double nowSeconds();         // زمان از شروع برنامه
  static uint64_t tickCount();        // شمارنده‌ی فریم

private:
  static uint64_t tick_;
  static double lastNow_;
  static float dt_;
};