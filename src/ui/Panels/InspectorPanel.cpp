#include "InspectorPanel.h"
#include "imgui.h"
#include "core/Logger.h"

static const char* blockLabel(BlockType t) {
  switch (t) {
    case BlockType::WhenGreenFlag: return "when green flag clicked";
    case BlockType::WhenKeyPressed: return "when key pressed";
    case BlockType::MoveSteps: return "move steps";
    case BlockType::TurnRight: return "turn right";
    case BlockType::TurnLeft: return "turn left";
    case BlockType::GoToXY: return "go to x y";
    case BlockType::Say: return "say";
    case BlockType::WaitSeconds: return "wait seconds";
    case BlockType::Repeat: return "repeat (TODO)";
    case BlockType::Forever: return "forever (TODO)";
    case BlockType::IfThen: return "if then (TODO)";
    default: return "block";
  }
}

void InspectorPanel::draw(Project& project, int selectedSpriteId, EditorState& st) {
  ImGui::Begin("Inspector");

  Sprite* sp = project.findSpriteById(selectedSpriteId);
  if (!sp) {
    ImGui::TextDisabled("No sprite selected.");
    ImGui::End();
    return;
  }

  // --- Block inspector (if selected) ---
  if (st.selectedBlockId != 0) {
    Block* b = project.findBlock(*sp, st.selectedBlockId);
    if (b) {
      ImGui::Text("Block: %s", blockLabel(b->type));
      ImGui::Separator();

      // editable args based on type
      if (b->type == BlockType::MoveSteps || b->type == BlockType::TurnRight || b->type == BlockType::TurnLeft || b->type == BlockType::WaitSeconds) {
        if (b->args.empty()) b->args = {"0"};
        char buf[64]{};
        strncpy(buf, b->args[0].c_str(), sizeof(buf)-1);
        if (ImGui::InputText("Value", buf, sizeof(buf))) {
          b->args[0] = buf;
          project.markDirty();
        }
      } else if (b->type == BlockType::Say || b->type == BlockType::Think || b->type == BlockType::AskAndWait) {
        if (b->args.empty()) b->args = {"Hello!"};
        char buf[256]{};
        strncpy(buf, b->args[0].c_str(), sizeof(buf)-1);
        if (ImGui::InputTextMultiline("Text", buf, sizeof(buf), ImVec2(-1, 80))) {
          b->args[0] = buf;
          project.markDirty();
        }
      } else if (b->type == BlockType::GoToXY) {
        while (b->args.size() < 2) b->args.push_back("0");
        char bx[64]{}, by[64]{};
        strncpy(bx, b->args[0].c_str(), sizeof(bx)-1);
        strncpy(by, b->args[1].c_str(), sizeof(by)-1);
        bool changed = false;
        changed |= ImGui::InputText("X", bx, sizeof(bx));
        changed |= ImGui::InputText("Y", by, sizeof(by));
        if (changed) {
          b->args[0] = bx;
          b->args[1] = by;
          project.markDirty();
        }
      } else {
        ImGui::TextDisabled("No editable args for this block yet.");
      }

      ImGui::Separator();
      if (ImGui::Button("Delete Block")) {
        // delete block by unlinking from stack + erase
        // (we do a safe delete implemented in Workspace step; here just flag)
        Logger::warn("TODO", "Delete block from Inspector not implemented yet (will be in Workspace logic).");
      }

      ImGui::End();
      return; // وقتی block انتخاب شده، تنظیمات sprite را پایین نشون نمی‌دیم
    } else {
      // selected block invalid -> clear
      st.selectedBlockId = 0;
    }
  }

  // --- Sprite inspector (default) ---
  char nameBuf[128]{};
  strncpy(nameBuf, sp->name.c_str(), sizeof(nameBuf)-1);
  if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
    sp->name = nameBuf;
    project.markDirty();
  }

  if (ImGui::Checkbox("Visible", &sp->visible)) project.markDirty();
  if (ImGui::DragFloat("X", &sp->x, 1.0f, -240.0f, 240.0f)) project.markDirty();
  if (ImGui::DragFloat("Y", &sp->y, 1.0f, -180.0f, 180.0f)) project.markDirty();
  if (ImGui::DragFloat("Direction", &sp->directionDeg, 1.0f, -360.0f, 360.0f)) project.markDirty();
  if (ImGui::SliderFloat("Size %", &sp->sizePercent, 1.0f, 300.0f)) project.markDirty();

  ImGui::End();
}