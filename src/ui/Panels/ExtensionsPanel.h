#pragma once
#include "core/Project.h"
#include "extensions/PenSystem.h"

class ExtensionsPanel {
public:
  void draw(Project& project, bool* open, bool* penEnabled, int selectedSpriteId, PenSystem* pen);
};