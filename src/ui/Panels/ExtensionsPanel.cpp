#include "ui/Panels/ExtensionsPanel.h"
#include "imgui.h"

void ExtensionsPanel::draw(Project& project, bool* open, bool* penEnabled, int selectedSpriteId, PenSystem* pen) {
  if (!open || !*open) return;

  (void)project;
  (void)selectedSpriteId;

  ImGui::Begin("Extensions", open);

  bool enabled = (penEnabled ? *penEnabled : false);
  if (ImGui::Checkbox("Enable Pen Extension", &enabled)) {
    if (penEnabled) *penEnabled = enabled;
    if (pen) pen->setEnabled(enabled);
  }

  if (enabled && pen) {
    ImGui::Separator();

    if (ImGui::Button("Erase all")) pen->eraseAll();

    ImGui::Separator();

    if (ImGui::Button("Pen down")) pen->mousePenDown();
    ImGui::SameLine();
    if (ImGui::Button("Pen up")) pen->mousePenUp();
    ImGui::SameLine();
    ImGui::Text("(%s)", pen->mouseIsDown() ? "Down" : "Up");

    ImGui::Separator();

    PenColor c = pen->color();
    float col[4] = { c.r, c.g, c.b, c.a };
    if (ImGui::ColorEdit4("Pen color", col)) {
      pen->setColor(PenColor{col[0], col[1], col[2], col[3]});
    }

    float s = pen->size();
    if (ImGui::SliderFloat("Pen size", &s, 1.0f, 40.0f, "%.1f")) {
      pen->setSize(s);
    }

    ImGui::Separator();
    ImGui::TextWrapped("Draw on Stage: click + drag with left mouse button.");
  }

  ImGui::End();
}