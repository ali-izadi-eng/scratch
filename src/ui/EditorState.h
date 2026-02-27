#pragma once
#include <cstdint>

struct EditorState {
  int selectedBlockId{0};
  int selectedScriptId{0};

  // drag
  bool dragging{false};
  int dragRootBlockId{0};  // block id که کلیک/drag ازش شروع شد
  float dragStartMouseX{0}, dragStartMouseY{0};
  float dragStartBlockX{0}, dragStartBlockY{0};
};