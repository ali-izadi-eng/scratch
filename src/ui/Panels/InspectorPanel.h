#pragma once
#include "core/Project.h"
#include "ui/EditorState.h"

class InspectorPanel {
public:
  void draw(Project& project, int selectedSpriteId, EditorState& st);
};