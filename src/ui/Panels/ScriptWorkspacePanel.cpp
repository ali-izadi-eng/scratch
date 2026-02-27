#include "ui/Panels/ScriptWorkspacePanel.h"

#include "imgui.h"
#include "core/Logger.h"
#include <cmath>
#include <string>
#include <algorithm>

static const float BLOCK_W = 220.0f;
static const float BLOCK_H = 40.0f;
static const float BLOCK_STEP_Y = 60.0f;
static const float SNAP = 18.0f;

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
    case BlockType::WaitUntil: return "wait until";
    case BlockType::Repeat: return "repeat";
    case BlockType::RepeatUntil: return "repeat until";
    case BlockType::Forever: return "forever";
    case BlockType::IfThen: return "if then";
    case BlockType::StopThisScript: return "stop this script";
    case BlockType::StopAll: return "stop all";
    case BlockType::AskAndWait: return "ask and wait";
    case BlockType::SetX: return "set x to";
    case BlockType::SetY: return "set y to";
    case BlockType::ChangeXBy: return "change x by";
    case BlockType::ChangeYBy: return "change y by";
    case BlockType::GoToRandomPosition: return "go to random";
    case BlockType::GoToMousePointer: return "go to mouse";
    case BlockType::IfOnEdgeBounce: return "if on edge bounce";
    case BlockType::StopAtEdge: return "stop at edge";
    case BlockType::Think: return "think";
    case BlockType::SwitchCostumeTo: return "switch costume to";
    case BlockType::NextCostume: return "next costume";
    case BlockType::SwitchBackdropTo: return "switch backdrop to";
    case BlockType::NextBackdrop: return "next backdrop";
    case BlockType::SetSizeTo: return "set size to";
    case BlockType::ChangeSizeBy: return "change size by";
    case BlockType::Show: return "show";
    case BlockType::Hide: return "hide";
    case BlockType::GoToFrontLayer: return "go to front layer";
    case BlockType::GoBackLayers: return "go back layers";
    case BlockType::LooksReporter: return "looks report";

    // Sound
    case BlockType::PlaySound: return "play sound";
    case BlockType::PlaySoundUntilDone: return "play sound until done";
    case BlockType::StopAllSounds: return "stop all sounds";
    case BlockType::SetVolumeTo: return "set volume to";
    case BlockType::ChangeVolumeBy: return "change volume by";
    case BlockType::SetPitchTo: return "set pitch to";
    case BlockType::ChangePitchBy: return "change pitch by";
    default: return "block";
  }
}

static bool isControl(BlockType t) {
  return (t == BlockType::Repeat || t == BlockType::RepeatUntil || t == BlockType::Forever || t == BlockType::IfThen);
}

// -------------------- Hit Tests --------------------

int ScriptWorkspacePanel::hitTestBlock(const Sprite& sp, float localX, float localY) const {
  for (const auto& kv : sp.blocks) {
    const Block& b = kv.second;
    if (localX >= b.x && localX <= b.x + BLOCK_W &&
        localY >= b.y && localY <= b.y + BLOCK_H) {
      return b.id;
    }
  }
  return 0;
}

int ScriptWorkspacePanel::hitTestTail(const Sprite& sp, float localX, float localY) const {
  // detect drop near bottom "connector" of last block in each script
  for (const auto& sc : sp.scripts) {
    int cur = sc.headBlockId;
    int safety = 0;
    int last = -1;

    while (cur != -1 && safety++ < 2000) {
      auto it = sp.blocks.find(cur);
      if (it == sp.blocks.end()) break;
      last = cur;
      cur = it->second.nextId;
    }
    if (last == -1) continue;

    auto itLast = sp.blocks.find(last);
    if (itLast == sp.blocks.end()) continue;

    float cx = itLast->second.x + BLOCK_W * 0.5f;
    float cy = itLast->second.y + BLOCK_H; // bottom

    if (std::fabs(localX - cx) <= SNAP && std::fabs(localY - cy) <= SNAP) {
      return last;
    }
  }
  return 0;
}

float ScriptWorkspacePanel::chainHeight(const Sprite& sp, int headId) const {
  // height reserved for an empty body too
  if (headId == -1) return 60.0f;

  float h = 0.0f;
  int cur = headId;
  int safety = 0;
  while (cur != -1 && safety++ < 5000) {
    auto it = sp.blocks.find(cur);
    if (it == sp.blocks.end()) break;

    const Block& b = it->second;
    h += BLOCK_STEP_Y;

    if (b.childHeadId != -1) {
      h += chainHeight(sp, b.childHeadId) + 20.0f;
    }

    cur = b.nextId;
  }

  return std::max(60.0f, h);
}

int ScriptWorkspacePanel::hitTestControlBody(const Sprite& sp, float localX, float localY) const {
  const float HEADER_H = BLOCK_H;

  for (const auto& kv : sp.blocks) {
    const Block& b = kv.second;
    if (!isControl(b.type)) continue;

    float bodyH = chainHeight(sp, b.childHeadId);

    // body rect inside the control block (indent)
    float x0 = b.x + 20.0f;
    float y0 = b.y + HEADER_H;
    float x1 = b.x + BLOCK_W - 10.0f;
    float y1 = y0 + bodyH;

    if (localX >= x0 && localX <= x1 && localY >= y0 && localY <= y1) {
      return b.id; // owner control block id
    }
  }

  return 0;
}

// -------------------- Movement --------------------

void ScriptWorkspacePanel::moveStackRecursive(Sprite& sp, int rootBlockId, float dx, float dy) {
  int cur = rootBlockId;
  int safety = 0;
  while (cur != -1 && safety++ < 8000) {
    auto it = sp.blocks.find(cur);
    if (it == sp.blocks.end()) break;

    it->second.x += dx;
    it->second.y += dy;

    if (it->second.childHeadId != -1) {
      moveStackRecursive(sp, it->second.childHeadId, dx, dy);
    }

    cur = it->second.nextId;
  }
}

// -------------------- Main Draw --------------------

void ScriptWorkspacePanel::draw(Project& project, int selectedSpriteId, bool* open, EditorState& st) {
  if (!open || !*open) return;
  ImGui::Begin("Script Workspace", open);

  Sprite* sp = project.findSpriteById(selectedSpriteId);
  if (!sp) {
    ImGui::TextDisabled("Select a sprite.");
    ImGui::End();
    return;
  }

  ImGui::Text("Sprite: %s", sp->name.c_str());
  ImGui::SameLine();
  if (ImGui::Button("New Script (hat)")) {
    project.createScript(*sp, 50, 50, BlockType::WhenGreenFlag);
  }

  ImGui::Separator();

  // Canvas
  ImVec2 canvasPos = ImGui::GetCursorScreenPos();
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();
  if (canvasSize.x < 200) canvasSize.x = 200;
  if (canvasSize.y < 200) canvasSize.y = 200;

  ImGui::InvisibleButton("##canvas", canvasSize,
    ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);

  bool hovered = ImGui::IsItemHovered();
  ImVec2 mouse = ImGui::GetIO().MousePos;

  float localX = mouse.x - canvasPos.x;
  float localY = mouse.y - canvasPos.y;

  // -------------------- Right-click Context --------------------
  if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
    int hit = hitTestBlock(*sp, localX, localY);
    if (hit != 0) {
      st.selectedBlockId = hit;
      ImGui::OpenPopup("BlockContext");
    } else {
      ImGui::OpenPopup("CanvasContext");
    }
  }

  if (ImGui::BeginPopup("CanvasContext")) {
    if (ImGui::MenuItem("New Script (Green Flag)")) {
      project.createScript(*sp, localX, localY, BlockType::WhenGreenFlag);
    }
    if (ImGui::MenuItem("New Script (Key Press)")) {
      project.createScript(*sp, localX, localY, BlockType::WhenKeyPressed);
    }
    ImGui::EndPopup();
  }

  if (ImGui::BeginPopup("BlockContext")) {
    if (ImGui::MenuItem("Delete Block")) {
      if (st.selectedBlockId != 0) {
        project.deleteBlock(*sp, st.selectedBlockId);
        st.selectedBlockId = 0;
      }
    }

    // اگر hat بود، اجازه حذف script
    Block* b = (st.selectedBlockId ? project.findBlock(*sp, st.selectedBlockId) : nullptr);
    bool hat = b && (b->type == BlockType::WhenGreenFlag || b->type == BlockType::WhenKeyPressed);

    if (!hat) ImGui::BeginDisabled();
    if (ImGui::MenuItem("Delete Script")) {
      int sid = 0;
      for (auto& sc : sp->scripts) {
        if (sc.headBlockId == st.selectedBlockId) sid = sc.id;
      }
      if (sid != 0) {
        project.deleteScript(*sp, sid);
        st.selectedBlockId = 0;
      }
    }
    if (!hat) ImGui::EndDisabled();

    ImGui::EndPopup();
  }

  // -------------------- Left click: select + start drag --------------------
  if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    int hit = hitTestBlock(*sp, localX, localY);
    st.selectedBlockId = hit;

    if (hit != 0) {
      st.dragging = true;
      st.dragRootBlockId = hit;
      st.dragStartMouseX = localX;
      st.dragStartMouseY = localY;

      Block* bb = project.findBlock(*sp, hit);
      st.dragStartBlockX = bb ? bb->x : 0;
      st.dragStartBlockY = bb ? bb->y : 0;
    }
  }

  // -------------------- Drag move --------------------
  if (st.dragging) {
    if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      Block* bb = project.findBlock(*sp, st.dragRootBlockId);
      if (bb) {
        float dx = localX - st.dragStartMouseX;
        float dy = localY - st.dragStartMouseY;

        float newX = st.dragStartBlockX + dx;
        float newY = st.dragStartBlockY + dy;

        float ddx = newX - bb->x;
        float ddy = newY - bb->y;

        moveStackRecursive(*sp, st.dragRootBlockId, ddx, ddy);
        project.markDirty();
      }
    } else {
      // mouse released: snap connect to tail if close
      int tail = hitTestTail(*sp, localX, localY);
      if (tail != 0 && st.dragRootBlockId != 0 && tail != st.dragRootBlockId) {

        // detach dragged root from previous chain
        int prev = project.findPrevInAnyChain(*sp, st.dragRootBlockId);
        if (prev != -1) {
          Block* pb = project.findBlock(*sp, prev);
          if (pb) pb->nextId = -1;
        } else {
          // if it was a script head, detach by clearing head
          for (auto& sc : sp->scripts) {
            if (sc.headBlockId == st.dragRootBlockId) {
              sc.headBlockId = -1; // simple detach; later we can prune empty scripts
            }
          }
        }

        // connect tail -> draggedRoot
        project.linkAfter(*sp, tail, st.dragRootBlockId);

        // align position under tail
        Block* tb = project.findBlock(*sp, tail);
        Block* rb = project.findBlock(*sp, st.dragRootBlockId);
        if (tb && rb) {
          float targetX = tb->x;
          float targetY = tb->y + BLOCK_STEP_Y;
          float dx = targetX - rb->x;
          float dy = targetY - rb->y;
          moveStackRecursive(*sp, st.dragRootBlockId, dx, dy);
          project.markDirty();
        }
      }

      st.dragging = false;
      st.dragRootBlockId = 0;
    }
  }

  // -------------------- Drop from palette --------------------
  if (hovered && ImGui::BeginDragDropTarget()) {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("BLOCK_TYPE")) {
      BlockType t = *(const BlockType*)payload->Data;

      // 1) If dropped inside a control-body => append to child chain
      int bodyOwner = hitTestControlBody(*sp, localX, localY);
      if (bodyOwner != 0 &&
          t != BlockType::WhenGreenFlag && t != BlockType::WhenKeyPressed) {

        Block* owner = project.findBlock(*sp, bodyOwner);
        if (owner) {
          float px = owner->x + 30.0f;
          float py = owner->y + 60.0f;
          float offsetY = chainHeight(*sp, owner->childHeadId);

          // IMPORTANT: requires Project::appendToChain
          int newId = project.appendToChain(*sp, owner->childHeadId, t,
                                            px, py + offsetY - 60.0f);

          st.selectedBlockId = newId;
          project.markDirty();
        }

        Logger::info("Workspace", std::string("Dropped into body: ") + blockLabel(t));
        ImGui::EndDragDropTarget();
        ImGui::End(); // end window
        return;
      }

      // 2) If hat => create new script
      if (t == BlockType::WhenGreenFlag || t == BlockType::WhenKeyPressed) {
        project.createScript(*sp, localX, localY, t);
      } else {
        // 3) Normal: append to selected script (or create if none)
        if (sp->scripts.empty()) {
          int sid = project.createScript(*sp, localX, localY, BlockType::WhenGreenFlag);
          int newId = project.appendBlockToScript(*sp, sid, t);
          Block* nb = project.findBlock(*sp, newId);
          if (nb) { nb->x = localX; nb->y = localY; }
        } else {
          int sid = sp->selectedScriptId ? sp->selectedScriptId : sp->scripts.front().id;
          int newId = project.appendBlockToScript(*sp, sid, t);
          Block* nb = project.findBlock(*sp, newId);
          if (nb) { nb->x = localX; nb->y = localY; }
        }
      }

      Logger::info("Workspace", std::string("Dropped block: ") + blockLabel(t));
    }
    ImGui::EndDragDropTarget();
  }

  // -------------------- Draw blocks --------------------
  ImDrawList* dl = ImGui::GetWindowDrawList();
  dl->AddRect(canvasPos,
              ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y),
              IM_COL32(120,120,120,255));

  auto drawOneBlock = [&](const Block& b, const ImVec2& basePos, EditorState& stRef, const Sprite& spRef) {
    ImVec2 p0(basePos.x + b.x, basePos.y + b.y);
    ImVec2 p1(basePos.x + b.x + BLOCK_W, basePos.y + b.y + BLOCK_H);

    ImU32 col = IM_COL32(70, 130, 220, 255);
    if (b.type == BlockType::WhenGreenFlag || b.type == BlockType::WhenKeyPressed) col = IM_COL32(230, 180, 40, 255);
    if (b.type == BlockType::Say) col = IM_COL32(160, 90, 190, 255);
    if (b.type == BlockType::WaitSeconds || isControl(b.type))
      col = IM_COL32(230, 140, 50, 255);
    if (b.type == BlockType::PlaySound || b.type == BlockType::PlaySoundUntilDone || b.type == BlockType::StopAllSounds ||
        b.type == BlockType::SetVolumeTo || b.type == BlockType::ChangeVolumeBy ||
        b.type == BlockType::SetPitchTo || b.type == BlockType::ChangePitchBy)
      col = IM_COL32(220, 80, 150, 255);

    bool selected = (stRef.selectedBlockId == b.id);
    if (selected) {
      dl->AddRectFilled(ImVec2(p0.x-2, p0.y-2), ImVec2(p1.x+2, p1.y+2),
                        IM_COL32(255,255,255,140), 8.0f);
    }

    dl->AddRectFilled(p0, p1, col, 6.0f);
    dl->AddRect(p0, p1, IM_COL32(20,20,20,255), 6.0f);

    std::string text = blockLabel(b.type);
    if (!b.args.empty()) {
      text += " (";
      for (size_t i=0;i<b.args.size();++i) {
        text += b.args[i];
        if (i + 1 < b.args.size()) text += ",";
      }
      text += ")";
    }
    dl->AddText(ImVec2(p0.x + 8, p0.y + 12), IM_COL32(10,10,10,255), text.c_str());

    // body for control
    if (isControl(b.type)) {
      float bodyH = chainHeight(spRef, b.childHeadId);

      ImVec2 bx0(basePos.x + b.x + 20.0f, basePos.y + b.y + BLOCK_H);
      ImVec2 bx1(basePos.x + b.x + BLOCK_W - 10.0f, basePos.y + b.y + BLOCK_H + bodyH);

      dl->AddRectFilled(bx0, bx1, IM_COL32(255,255,255,40), 6.0f);
      dl->AddRect(bx0, bx1, IM_COL32(20,20,20,120), 6.0f);
      dl->AddText(ImVec2(bx0.x + 6, bx0.y + 6), IM_COL32(10,10,10,180), "drop blocks here");
    }
  };

  // draw top-level scripts
  for (const auto& sc : sp->scripts) {
    int cur = sc.headBlockId;
    int safety = 0;
    while (cur != -1 && safety++ < 5000) {
      auto itb = sp->blocks.find(cur);
      if (itb == sp->blocks.end()) break;
      const Block& b = itb->second;

      drawOneBlock(b, canvasPos, st, *sp);

      // draw child chain if any
      if (b.childHeadId != -1) {
        int ccur = b.childHeadId;
        int csafe = 0;
        while (ccur != -1 && csafe++ < 5000) {
          auto itc = sp->blocks.find(ccur);
          if (itc == sp->blocks.end()) break;
          const Block& cb = itc->second;

          drawOneBlock(cb, canvasPos, st, *sp);

          ccur = cb.nextId;
        }
      }

      cur = b.nextId;
    }
  }

  ImGui::End();
}