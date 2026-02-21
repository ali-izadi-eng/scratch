#include "Watchdog.h"
#include "Logger.h"

void Watchdog::beginFrame() {
  instrThisFrame_ = 0;
  tripped_ = false;
}

void Watchdog::addInstructions(uint32_t count) {
  instrThisFrame_ += count;
  if (instrThisFrame_ > limit_) tripped_ = true;
}

bool Watchdog::tripped() const { return tripped_; }

void Watchdog::onFrameEnd() {
  // فعلاً فقط اگر اتفاقی افتاد لاگ کن.
  if (tripped_) {
    Logger::error("Watchdog", "Infinite loop suspected. Execution should be stopped.");
  }
}