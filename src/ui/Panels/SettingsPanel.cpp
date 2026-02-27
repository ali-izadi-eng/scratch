#include "ui/Panels/SettingsPanel.h"
#include "imgui.h"

void SettingsPanel::draw(Project& project, bool* open) {
  if (!open || !*open) return;

  ImGui::Begin("Settings", open);

  ImGui::TextUnformatted("Settings");
  ImGui::Separator();
  ImGui::Text("Backdrops: %d", (int)project.stage().backdrops.size());
  ImGui::Text("Current backdrop: %d", project.stage().currentBackdrop);

  ImGui::End();
}