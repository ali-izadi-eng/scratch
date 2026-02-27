#include "ui/Panels/BlocksPalettePanel.h"

#include "imgui.h"
#include "model/Block.h"   // ✅ BlockType from the real model header

namespace {

void DragBlock(BlockType t, const char* label) {
  ImGui::Button(label, ImVec2(-1, 0));
  if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
    ImGui::SetDragDropPayload("BLOCK_TYPE", &t, sizeof(t));
    ImGui::Text("Add: %s", label);
    ImGui::EndDragDropSource();
  }
}

} // namespace

void BlocksPalettePanel::draw(Project&, bool* open) {
  if (!open || !*open) return;

  if (!ImGui::Begin("Blocks Palette", open)) {
    ImGui::End();
    return;
  }

  // EVENTS
  ImGui::TextUnformatted("Events");
  DragBlock(BlockType::WhenGreenFlag, "when green flag clicked");
  DragBlock(BlockType::WhenKeyPressed, "when key pressed");
  ImGui::Separator();

  // MOTION (BLUE)
  ImGui::TextUnformatted("Motion");

  ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.15f, 0.35f, 0.80f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.45f, 0.95f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.10f, 0.30f, 0.70f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1,1,1,1));

  // 13
  DragBlock(BlockType::MoveSteps, "move (10) steps");
  DragBlock(BlockType::TurnRight, "turn right (15) deg");
  DragBlock(BlockType::TurnLeft,  "turn left (15) deg");

  ImGui::Separator();

  // 14
  DragBlock(BlockType::GoToXY,    "go to x:(0) y:(0)");
  DragBlock(BlockType::SetX,      "set x to (0)");
  DragBlock(BlockType::SetY,      "set y to (0)");
  DragBlock(BlockType::ChangeXBy, "change x by (10)");
  DragBlock(BlockType::ChangeYBy, "change y by (10)");

  ImGui::Separator();

  // 15
  DragBlock(BlockType::GoToRandomPosition, "go to random position");
  DragBlock(BlockType::GoToMousePointer,   "go to mouse-pointer");

  ImGui::Separator();

  // 16
  DragBlock(BlockType::IfOnEdgeBounce, "if on edge, bounce");
  DragBlock(BlockType::StopAtEdge,     "stop at edge");

  ImGui::PopStyleColor(4);

  ImGui::Separator();

  // CONTROL (LIGHT ORANGE)
  ImGui::TextUnformatted("Control");

  ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.95f, 0.65f, 0.20f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.00f, 0.72f, 0.28f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.90f, 0.58f, 0.16f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0,0,0,1));

  DragBlock(BlockType::WaitSeconds, "wait (1) seconds");
  DragBlock(BlockType::WaitUntil,   "wait until <condition>");

  ImGui::Separator();
  DragBlock(BlockType::Repeat,      "repeat (10) { }");
  DragBlock(BlockType::RepeatUntil, "repeat until <condition> { }");
  DragBlock(BlockType::Forever,     "forever { }");

  ImGui::Separator();
  DragBlock(BlockType::IfThen,         "if <condition> then { }");
  DragBlock(BlockType::StopThisScript, "stop this script");
  DragBlock(BlockType::StopAll,        "stop all");

  ImGui::PopStyleColor(4);

  // ---- Sensing (AZURE) ----
  ImGui::Separator();
  ImGui::TextUnformatted("Sensing");

  ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.25f, 0.75f, 0.85f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.30f, 0.85f, 0.95f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.20f, 0.65f, 0.75f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0,0,0,1));

  DragBlock(BlockType::AskAndWait, "ask (question) and wait");

  ImGui::PopStyleColor(4);


  // ---- Looks (INDIGO) ----
  ImGui::TextUnformatted("Looks");

  // نیلی کردن دکمه‌های Looks
  ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.30f, 0.25f, 0.70f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.38f, 0.33f, 0.88f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.26f, 0.22f, 0.62f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1,1,1,1));

  // 17) گرفتن و فکر کردن
  DragBlock(BlockType::Say,   "say (Hello!)");
  DragBlock(BlockType::Think, "think (Hmm...)");

  ImGui::Separator();

  // 18) لباس
  DragBlock(BlockType::SwitchCostumeTo, "switch costume to (1)");
  DragBlock(BlockType::NextCostume,     "next costume");

  ImGui::Separator();

  // 19) پس‌زمینه
  DragBlock(BlockType::SwitchBackdropTo, "switch backdrop to (1)");
  DragBlock(BlockType::NextBackdrop,     "next backdrop");

  ImGui::Separator();

  // 20) اندازه
  DragBlock(BlockType::SetSizeTo,    "set size to (100)%");
  DragBlock(BlockType::ChangeSizeBy, "change size by (10)");

  ImGui::Separator();

  // 21) نمایش/مخفی
  DragBlock(BlockType::Show, "show");
  DragBlock(BlockType::Hide, "hide");

  ImGui::Separator();

  // 22) لایه‌ها
  DragBlock(BlockType::GoToFrontLayer, "go to front layer");
  DragBlock(BlockType::GoBackLayers,   "go back (1) layers");

  ImGui::Separator();

  // 23) گزارش‌های looks (placeholder)
  DragBlock(BlockType::LooksReporter, "looks report (costume #)");

  ImGui::PopStyleColor(4);

  // ---- Sound (PINK) ----
  ImGui::Separator();
  ImGui::TextUnformatted("Sound");

  ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.85f, 0.25f, 0.60f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.95f, 0.32f, 0.70f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.75f, 0.20f, 0.55f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1,1,1,1));

  // 24) play sound
  DragBlock(BlockType::PlaySound, "play sound (1)");

  ImGui::Separator();

  // 25) wait / stop
  DragBlock(BlockType::PlaySoundUntilDone, "play sound (1) until done");
  DragBlock(BlockType::StopAllSounds, "stop all sounds");

  ImGui::Separator();

  // 26) volume / pitch
  DragBlock(BlockType::SetVolumeTo, "set volume to (100)%");
  DragBlock(BlockType::ChangeVolumeBy, "change volume by (10)%");
  DragBlock(BlockType::SetPitchTo, "set pitch to (0) st");
  DragBlock(BlockType::ChangePitchBy, "change pitch by (1) st");

  ImGui::PopStyleColor(4);

  ImGui::End();
}