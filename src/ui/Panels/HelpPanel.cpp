#include "HelpPanel.h"
#include "imgui.h"

void HelpPanel::draw(bool* open) {
  if (!open || !*open) return;
  ImGui::Begin("Help", open);

  ImGui::TextUnformatted("Scratchy - Help");
  ImGui::Separator();

  ImGui::BulletText("File: New / Save / Load");
  ImGui::BulletText("Run: Green Flag (runs all scripts with a hat), Stop All, Pause/Resume, Step");
  ImGui::BulletText("Stage: shows backdrops + sprites. Yellow square is sprite placeholder if no costume.");

  ImGui::Separator();
  ImGui::TextUnformatted("Blocks Palette");
  ImGui::BulletText("Drag a block from the palette and drop it into Script Workspace.");
  ImGui::BulletText("To run, scripts must START with a hat block (e.g. when green flag clicked). Then chain blocks under it.");
  ImGui::BulletText("Control blocks (light orange) have a body: drop blocks INSIDE their body area.");

  ImGui::Separator();
  ImGui::TextUnformatted("Sensing");
  ImGui::BulletText("ask ( ) and wait: opens a popup and stores answer in {answer}.");
  ImGui::BulletText("Conditions are typed as text for now, examples: touching edge | touching mouse | key space | mouse down | distance to mouse < 50");

  ImGui::Separator();
  ImGui::TextUnformatted("Costumes");
  ImGui::BulletText("Add costume by pasting an image path, or drag & drop an image file onto the app window.");

  ImGui::Separator();
  ImGui::TextUnformatted("Pen tools");
  ImGui::BulletText("Extensions -> Pen enables drawing overlay on the Stage.");
  ImGui::BulletText("Use Pen Tools window: Pen / Highlighter / Eraser / Stamp.");

  ImGui::Separator();
  ImGui::TextUnformatted("Debug");
  ImGui::BulletText("Logger panel shows runtime start/stop and warnings.");

  ImGui::End();
}
