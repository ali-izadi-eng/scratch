#pragma once
#include "core/Project.h"
#include "renderer/Renderer2D.h"
#include "extensions/PenSystem.h"

class StagePanel {
public:
  void draw(Project& project, Renderer2D& renderer, PenSystem* pen);
};