#include "ui/Panels/StagePanel.h"
#include "imgui.h"

void StagePanel::draw(Project& project, Renderer2D& renderer, PenSystem* pen) {
  // NoBackground so SDL stage draw stays visible
  ImGui::Begin("Stage", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoBackground);

  ImVec2 canvasPos  = ImGui::GetCursorScreenPos();
  ImVec2 canvasSize = ImGui::GetContentRegionAvail();
  if (canvasSize.x < 50) canvasSize.x = 50;
  if (canvasSize.y < 50) canvasSize.y = 50;
  ImVec2 canvasMax(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);

  // Draw the actual stage via SDL renderer (behind ImGui)
  SDL_FRect stageRect;
  stageRect.x = canvasPos.x;
  stageRect.y = canvasPos.y;
  stageRect.w = canvasSize.x;
  stageRect.h = canvasSize.y;
  renderer.drawStage(project, stageRect);

  // Interaction area
  ImGui::InvisibleButton("##stage_canvas", canvasSize);

  ImVec2 mp = ImGui::GetIO().MousePos;
  const bool hover =
    (mp.x >= canvasPos.x && mp.x <= canvasMax.x &&
     mp.y >= canvasPos.y && mp.y <= canvasMax.y);

  if (hover) {
    // update mouse world coords for sensing
    auto w = PenSystem::screenToWorld(mp, canvasPos, canvasMax);
    project.setMouseWorld(w.x, w.y, true);
  } else {
    project.setMouseWorld(0, 0, false);
  }

  // Pen overlay (draws with ImGui over the stage)
  if (pen && pen->enabled() && hover) {
    ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    if (pen->tool() == PenSystem::Tool::Stamp) {
      if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        auto w = PenSystem::screenToWorld(mp, canvasPos, canvasMax);
        pen->stampAt(w.x, w.y);
      }
    } else {
      if (pen->mouseIsDown()) {
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
          auto w = PenSystem::screenToWorld(mp, canvasPos, canvasMax);
          pen->beginMouseStroke(w.x, w.y);
        }
        if (ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
          auto w = PenSystem::screenToWorld(mp, canvasPos, canvasMax);
          pen->addMousePoint(w.x, w.y);
        }
        if (ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
          pen->endMouseStroke();
        }
      }
    }

    ImDrawList* dl = ImGui::GetWindowDrawList();
    pen->drawOverlay(dl, canvasPos, canvasMax);
  } else if (pen && pen->enabled()) {
    // still draw overlay when not hovering
    ImDrawList* dl = ImGui::GetWindowDrawList();
    pen->drawOverlay(dl, canvasPos, canvasMax);
  }

  ImGui::End();
}
