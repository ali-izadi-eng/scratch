#include "CostumesPanel.h"

#include "imgui.h"
#include "core/Logger.h"

#include <cstring>

static bool endsWith(const std::string& s, const char* suf) {
  size_t n = std::strlen(suf);
  if (s.size() < n) return false;
  return s.compare(s.size() - n, n, suf) == 0;
}

static bool isImagePath(const std::string& p) {
  std::string s = p;
  for (auto& c : s) c = (char)std::tolower((unsigned char)c);
  return endsWith(s, ".png") || endsWith(s, ".jpg") || endsWith(s, ".jpeg") || endsWith(s, ".bmp");
}

void CostumesPanel::draw(Project& project, int selectedSpriteId, bool* open) {
  if (!open || !*open) return;
  ImGui::Begin("Costumes", open);

  Sprite* sp = project.findSpriteById(selectedSpriteId);
  if (!sp) {
    ImGui::TextDisabled("Select a sprite to manage costumes.");
    ImGui::End();
    return;
  }

  ImGui::Text("Sprite: %s", sp->name.c_str());
  ImGui::Separator();

  // show costumes list
  if (sp->costumes.empty()) {
    ImGui::TextDisabled("No costumes yet.");
  } else {
    for (int i = 0; i < (int)sp->costumes.size(); ++i) {
      const auto& c = sp->costumes[i];
      ImGui::PushID(i);
      bool sel = (sp->currentCostume == i);
      if (ImGui::Selectable(c.name.empty() ? "(unnamed)" : c.name.c_str(), sel)) {
        sp->currentCostume = i;
        project.markDirty();
      }
      ImGui::SameLine();
      ImGui::TextDisabled("%s", c.imagePath.c_str());
      ImGui::PopID();
    }
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Add / Upload image");
  ImGui::TextDisabled("Tip: you can also drag & drop an image file into the app window.");

  static char pathBuf[512] = {0};
  static char nameBuf[128] = {0};

  ImGui::InputText("Image path", pathBuf, sizeof(pathBuf));
  ImGui::InputText("Name", nameBuf, sizeof(nameBuf));

  // if a file was dropped, offer to use it
  std::string dropped = project.consumeDroppedFile();
  if (!dropped.empty()) {
    if (isImagePath(dropped)) {
      std::snprintf(pathBuf, sizeof(pathBuf), "%s", dropped.c_str());
      if (std::strlen(nameBuf) == 0) {
        // default name from filename
        auto pos = dropped.find_last_of("/\\");
        std::string fn = (pos == std::string::npos) ? dropped : dropped.substr(pos + 1);
        std::snprintf(nameBuf, sizeof(nameBuf), "%s", fn.c_str());
      }
      Logger::info("Costumes", "Dropped image: " + dropped);
    }
  }

  if (ImGui::Button("Add costume", ImVec2(140, 0))) {
    std::string p = pathBuf;
    std::string n = nameBuf;
    if (p.empty()) {
      Logger::warn("Costumes", "Empty path");
    } else if (!isImagePath(p)) {
      Logger::warn("Costumes", "Not an image path: " + p);
    } else {
      Costume c;
      c.imagePath = p;
      c.name = n.empty() ? "Costume" + std::to_string((int)sp->costumes.size() + 1) : n;
      sp->costumes.push_back(c);
      sp->currentCostume = (int)sp->costumes.size() - 1;
      project.markDirty();
      Logger::info("Costumes", "Added costume: " + c.name);
    }
  }

  ImGui::SameLine();
  if (ImGui::Button("Clear", ImVec2(80, 0))) {
    pathBuf[0] = 0;
    nameBuf[0] = 0;
  }

  ImGui::Separator();
  ImGui::TextUnformatted("Pen tools (Stage overlay)");
  ImGui::TextDisabled("Use Extensions -> Pen and the 'Pen Tools' window to draw on the stage.");

  ImGui::End();
}
