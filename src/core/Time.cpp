#include "Time.h"
#include <SDL.h>

uint64_t Time::tick_ = 0;
double Time::lastNow_ = 0.0;
float Time::dt_ = 0.0f;

static double secondsNow() {
  static const double freq = (double)SDL_GetPerformanceFrequency();
  return (double)SDL_GetPerformanceCounter() / freq;
}

void Time::tick() {
  double n = secondsNow();
  if (tick_ == 0) {
    dt_ = 1.0f / 60.0f;
  } else {
    dt_ = (float)(n - lastNow_);
    if (dt_ < 0) dt_ = 0;
    if (dt_ > 0.25f) dt_ = 0.25f; // clamp (ایمنی)
  }
  lastNow_ = n;
  tick_++;
}

float Time::deltaSeconds() { return dt_; }
double Time::nowSeconds() { return lastNow_; }
uint64_t Time::tickCount() { return tick_; }