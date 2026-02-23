#pragma once
#include <string>
#include <vector>
#include "Costume.h"

struct Stage {
  // Backdrops: مثل Costume ولی برای Stage
  int currentBackdrop{0};
  std::vector<Costume> backdrops;

  const Costume* backdrop() const {
    if (backdrops.empty()) return nullptr;
    int idx = currentBackdrop;
    if (idx < 0 || idx >= (int)backdrops.size()) idx = 0;
    return &backdrops[idx];
  }
};