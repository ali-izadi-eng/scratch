#pragma once
#include <cstdint>

class IdGen {
public:
  int next();
  void reset(int start = 1) { next_ = start; }
  void ensureAtLeast(int minNext);

private:
  int next_{1};
};