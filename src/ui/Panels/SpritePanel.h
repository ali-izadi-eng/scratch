#pragma once
#include "core/Project.h"

class SpritePanel {
public:
  void draw(Project& project);
  int selectedSpriteId() const { return selectedId_; }

private:
  int selectedId_{0};
};