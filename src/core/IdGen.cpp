#include "IdGen.h"

int IdGen::next() { return next_++; }

void IdGen::ensureAtLeast(int minNext) {
  if (next_ < minNext) next_ = minNext;
}