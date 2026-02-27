#include "SoundsPanel.h"
#include "imgui.h"
#include "audio/AudioEngine.h"

#include <cstring>

void SoundsPanel::draw(Project& project, int selectedSpriteId, bool* open) {
  if (!open || !*open) return;
  ImGui::Begin("Sound Manager", open);

  Sprite* sp = project.findSpriteById(selectedSpriteId);
  if (!sp) {
    ImGui::TextDisabled("Select a sprite to manage sounds.");
    ImGui::End();
    return;
  }

  ImGui::Text("Sprite: %s", sp->name.c_str());
  ImGui::Separator();

  static char nameBuf[128] = "sound1";
  static char pathBuf[512] = "";

  ImGui::TextUnformatted("Add WAV sound (file path on disk):");
  ImGui::InputText("Name", nameBuf, sizeof(nameBuf));
  ImGui::InputText("File path", pathBuf, sizeof(pathBuf));

  if (ImGui::Button("Add Sound")) {
    Sound s;
    s.name = nameBuf;
    s.filePath = pathBuf;
    s.volume = 1.0f;
    s.muted = false;
    sp->sounds.push_back(std::move(s));
  }

  ImGui::Separator();
  ImGui::Text("Sounds: %d", (int)sp->sounds.size());

  for (int i = 0; i < (int)sp->sounds.size(); ++i) {
    Sound& s = sp->sounds[i];
    ImGui::PushID(i);
    ImGui::Text("#%d %s", i + 1, s.name.c_str());
    ImGui::SameLine();
    if (ImGui::Button("Play")) {
      float vol = (sp->soundVolume / 100.0f) * s.volume;
      AudioEngine::instance().playWav(s.filePath, vol, sp->soundPitch);
    }
    ImGui::SameLine();
    if (ImGui::Button("Stop All")) {
      AudioEngine::instance().stopAll();
    }
    ImGui::Checkbox("Muted", &s.muted);
    ImGui::SliderFloat("Sound volume (0..1)", &s.volume, 0.0f, 1.0f);
    ImGui::TextWrapped("Path: %s", s.filePath.c_str());
    ImGui::Separator();
    ImGui::PopID();
  }

  ImGui::End();
}