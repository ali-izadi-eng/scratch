#pragma once
#include "core/Project.h"
#include "model/Block.h"
#include "ui/EditorState.h"

class ScriptWorkspacePanel {
public:
  void draw(Project& project, int selectedSpriteId, bool* open, EditorState& st);

private:
  int hitTestBlock(const Sprite& sp, float localX, float localY) const;
  int hitTestTail(const Sprite& sp, float localX, float localY) const;
  float chainHeight(const Sprite& sp, int headId) const;
  int hitTestControlBody(const Sprite& sp, float localX, float localY) const;

  void moveStackRecursive(Sprite& sp, int rootBlockId, float dx, float dy);
};