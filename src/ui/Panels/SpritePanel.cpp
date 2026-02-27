#include "SpritePanel.h"
#include "imgui.h"
#include "core/Logger.h"
#include <algorithm>

void SpritePanel::draw(Project& project) {
  ImGui::Begin("Sprites");

  if (project.sprites().empty()) {
    ImGui::TextDisabled("No sprites.");
  }

  // Add sprite
  if (ImGui::Button("Add Sprite")) {
    Sprite sp;
    sp.id = project.allocSpriteId();
    sp.name = "Sprite" + std::to_string(sp.id);
    project.sprites().push_back(sp);
    selectedId_ = sp.id;
    project.markDirty();
    Logger::info("Sprite", "Added sprite id=" + std::to_string(sp.id));
  }

  ImGui::SameLine();

  // Remove selected
  bool canRemove = selectedId_ != 0 && (int)project.sprites().size() > 1;
  if (!canRemove) ImGui::BeginDisabled();
  if (ImGui::Button("Remove")) {
    auto& v = project.sprites();
    v.erase(std::remove_if(v.begin(), v.end(),
      [&](const Sprite& s){ return s.id == selectedId_; }), v.end());
    project.markDirty();
    Logger::warn("Sprite", "Removed selected sprite");
    selectedId_ = v.empty() ? 0 : v.front().id;
  }
  if (!canRemove) ImGui::EndDisabled();

  ImGui::Separator();

  // List
  for (auto& sp : project.sprites()) {
    bool selected = (sp.id == selectedId_);
    if (ImGui::Selectable((sp.name + "##" + std::to_string(sp.id)).c_str(), selected)) {
      selectedId_ = sp.id;
    }
  }

  if (selectedId_ == 0 && !project.sprites().empty())
    selectedId_ = project.sprites().front().id;

  bool found = false;
  for (auto& sp : project.sprites()) {
    if (sp.id == selectedId_) { found = true; break; }
  }
  if ((!found || selectedId_ == 0) && !project.sprites().empty()) {
    selectedId_ = project.sprites().front().id;
  }

  ImGui::End();
}