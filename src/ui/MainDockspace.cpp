#include "MainDockspace.h"

#include "imgui.h"
#include <algorithm>
#include <cstring>
#include <cstdio>

#include "core/Logger.h"
#include "core/Serialization.h"

#include "ui/Panels/StagePanel.h"
#include "ui/Panels/SpritePanel.h"
#include "ui/Panels/InspectorPanel.h"
#include "ui/Panels/BlocksPalettePanel.h"
#include "ui/Panels/ScriptWorkspacePanel.h"
#include "ui/Panels/CostumesPanel.h"
#include "ui/Panels/SoundsPanel.h"
#include "ui/Panels/SettingsPanel.h"
#include "ui/Panels/ExtensionsPanel.h"
#include "ui/Panels/HelpPanel.h"

// --- small helper to avoid Push/Pop mismatch ---
struct StyleColorGuard {
  int count = 0;
  ~StyleColorGuard() { if (count > 0) ImGui::PopStyleColor(count); }
  void push(ImGuiCol idx, const ImVec4& v) { ImGui::PushStyleColor(idx, v); ++count; }
};

MainDockspace::MainDockspace() {
  Logger::info("UI", "MainDockspace created");
}

void MainDockspace::doNewProject() {
  if (!project_) return;
  project_->resetToDefault();
  selectedSpriteId_ = project_->sprites().empty() ? 0 : project_->sprites().front().id;
  Logger::info("File", "New project created");
}

void MainDockspace::doSaveProject(const std::string& path) {
  if (!project_) return;

  std::string err;
  if (!Serialization::saveToFile(*project_, path, &err)) {
    lastError_ = err;
    Logger::error("File", "Save failed: " + err);
    return;
  }

  project_->setFilePath(path);
  project_->clearDirty();
  lastError_.clear();
  Logger::info("File", "Saved: " + path);
}

void MainDockspace::doLoadProject(const std::string& path) {
  if (!project_) return;

  std::string err;
  if (!Serialization::loadFromFile(*project_, path, &err)) {
    lastError_ = err;
    Logger::error("File", "Load failed: " + err);
    return;
  }

  selectedSpriteId_ = project_->sprites().empty() ? 0 : project_->sprites().front().id;
  lastError_.clear();
  Logger::info("File", "Loaded: " + path);
}

void MainDockspace::popupUnsavedChanges() {
  openUnsavedPopup_ = true;
  askUnsaved_ = true;
}

void MainDockspace::popupSaveAs() {
  if (project_ && !project_->filePath().empty())
    std::strncpy(pathBuf_, project_->filePath().c_str(), sizeof(pathBuf_) - 1);
  else
    std::strncpy(pathBuf_, "project.json", sizeof(pathBuf_) - 1);

  pathBuf_[sizeof(pathBuf_) - 1] = 0;
  openSavePopup_ = true;
}

void MainDockspace::popupLoad() {
  std::strncpy(pathBuf_, "project.json", sizeof(pathBuf_) - 1);
  pathBuf_[sizeof(pathBuf_) - 1] = 0;
  openLoadPopup_ = true;
}

// ---------------- Logger Panel ----------------
void MainDockspace::drawLoggerPanel() {
  if (!showLogger_) return;

  ImGui::Begin("Debug / Logger", &showLogger_);

  if (!lastError_.empty()) {
    ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Last error: %s", lastError_.c_str());
  }

  if (ImGui::Button("Clear Logs")) Logger::clear();
  ImGui::SameLine();
  ImGui::Text("Entries: %d", (int)Logger::entries().size());

  ImGui::Separator();
  ImGui::BeginChild("logscroll");

  for (const auto& e : Logger::entries()) {
    const char* lvl = (e.level == LogLevel::Info) ? "INFO" :
                      (e.level == LogLevel::Warn) ? "WARN" : "ERROR";
    ImGui::Text("[%llu] [%s] %s: %s",
      (unsigned long long)e.tick, lvl, e.tag.c_str(), e.msg.c_str());
  }

  ImGui::EndChild();
  ImGui::End();
}

// ---------------- Pen Tools Panel (Mouse Pen) ----------------
static void DrawPenToolsPanel(bool* open, bool penEnabled, PenSystem& pen) {
  if (!open || !*open) return;
  if (!penEnabled) return;

  ImGuiViewport* vp = ImGui::GetMainViewport();
  ImVec2 wp = vp->WorkPos;

  const float pad = 8.0f;
  const float w = 320.0f;
  const float h = 260.0f;

  ImGui::SetNextWindowPos(ImVec2(wp.x + pad, wp.y + pad), ImGuiCond_Always);
  ImGui::SetNextWindowSize(ImVec2(w, h), ImGuiCond_Always);

  ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse;

  // green header
  ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.05f, 0.55f, 0.12f, 1.0f));
  ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.07f, 0.65f, 0.15f, 1.0f));

  ImGui::Begin("Pen Tools", open, flags);

  // Tool selection
  if (ImGui::Button("Pen", ImVec2(70, 0))) pen.setTool(PenSystem::Tool::Pen);
  ImGui::SameLine();
  if (ImGui::Button("Highlighter", ImVec2(110, 0))) pen.setTool(PenSystem::Tool::Highlighter);
  ImGui::SameLine();
  if (ImGui::Button("Eraser", ImVec2(70, 0))) pen.setTool(PenSystem::Tool::Eraser);
  ImGui::SameLine();
  if (ImGui::Button("Stamp", ImVec2(70, 0))) pen.setTool(PenSystem::Tool::Stamp);

  ImGui::Separator();

  // Common
  if (ImGui::Button("Erase all", ImVec2(140, 0))) pen.eraseAll();

  ImGui::Separator();

  // Pen-like tools controls
  if (pen.tool() != PenSystem::Tool::Stamp) {
    if (ImGui::Button("Pen down", ImVec2(140, 0))) pen.mousePenDown();
    ImGui::SameLine();
    if (ImGui::Button("Pen up", ImVec2(140, 0))) pen.mousePenUp();
    ImGui::SameLine();
    ImGui::Text("%s", pen.mouseIsDown() ? "DOWN" : "UP");
  } else {
    ImGui::TextWrapped("Stamp: click on Stage to place a square.");
  }

  ImGui::Separator();

  PenColor c = pen.color();
  float col[4] = { c.r, c.g, c.b, c.a };
  if (ImGui::ColorEdit4("Color", col, ImGuiColorEditFlags_NoInputs)) {
    pen.setColor(PenColor{col[0], col[1], col[2], col[3]});
  }

  float s = pen.size();
  if (ImGui::SliderFloat("Size", &s, 1.0f, 40.0f, "%.1f")) {
    pen.setSize(s);
  }

  ImGui::End();
  ImGui::PopStyleColor(2);
}

// ---------------- Menu Bar ----------------
void MainDockspace::drawMenuBar(Runtime& runtime) {
  // منوبار سبز پررنگ
  StyleColorGuard bar;
  bar.push(ImGuiCol_HeaderActive, ImVec4(0.08f, 0.62f, 0.15f, 1.0f));

  if (!ImGui::BeginMainMenuBar()) return;

  // FILE
  if (ImGui::BeginMenu("File")) {
    if (ImGui::MenuItem("New Project")) {
      if (project_ && project_->isDirty()) { pending_ = PendingAction::NewProject; popupUnsavedChanges(); }
      else doNewProject();
    }

    if (ImGui::MenuItem("Save")) {
      if (!project_ || project_->filePath().empty()) popupSaveAs();
      else doSaveProject(project_->filePath());
    }

    if (ImGui::MenuItem("Save As...")) popupSaveAs();

    if (ImGui::MenuItem("Load...")) {
      if (project_ && project_->isDirty()) { pending_ = PendingAction::LoadProject; popupUnsavedChanges(); }
      else popupLoad();
    }

    ImGui::Separator();

    if (ImGui::MenuItem("Exit")) {
      if (project_ && project_->isDirty()) { pending_ = PendingAction::ExitApp; popupUnsavedChanges(); }
      else requestExit_ = true;
    }

    ImGui::EndMenu();
  }

  // ADD EXTENSION
  if (ImGui::BeginMenu("Add Extension")) {
    if (ImGui::MenuItem("Open Extensions")) showExtensions_ = true;

    ImGui::Separator();

    // Enable Pen (and open tools area)
    // toggle pen from menu (mouse pen)
    if (ImGui::MenuItem("Pen enabled", nullptr, &penEnabled_)) {
      pen_.setEnabled(penEnabled_);
      if (penEnabled_) {
        showExtensions_ = true;
        showPenTools_ = true;
        pen_.mousePenDown();   // آماده‌ی کشیدن با ماوس
      } else {
        pen_.mousePenUp();
      }
    }

    ImGui::EndMenu();
  }

  // CODE
  if (ImGui::BeginMenu("Code")) {
    ImGui::MenuItem("Blocks Palette", nullptr, &showBlocks_);
    ImGui::MenuItem("Script Workspace", nullptr, &showWorkspace_);
    ImGui::EndMenu();
  }

  // RUN
  if (ImGui::BeginMenu("Run")) {
    ImGui::Text("State: %s", runtime.isRunning() ? (runtime.isPaused() ? "Paused" : "Running") : "Stopped");
    ImGui::Separator();

    if (ImGui::MenuItem("Green Flag")) {
      if (project_) {
        runtime.startGreenFlag(*project_);
        if (stepMode_) runtime.setPaused(true);
      }
    }
    if (ImGui::MenuItem("Stop")) runtime.stopAll();

    ImGui::Separator();
    ImGui::MenuItem("Step Mode", nullptr, &stepMode_);
    if (ImGui::MenuItem("Pause")) { runtime.setPaused(true); stepMode_ = false; }
    if (ImGui::MenuItem("Resume")) { runtime.setPaused(false); stepMode_ = false; }

    if (stepMode_) {
      ImGui::Separator();
      if (ImGui::MenuItem("Step (one tick)")) {
        runtime.setPaused(true);
        if (project_) runtime.step(*project_);
      }
    }

    if (!runtime.lastError().empty()) {
      ImGui::Separator();
      ImGui::TextColored(ImVec4(1, 0.3f, 0.3f, 1), "Runtime error:");
      ImGui::TextWrapped("%s", runtime.lastError().c_str());
      if (ImGui::MenuItem("Clear Error")) runtime.clearError();
    }

    ImGui::EndMenu();
  }

  // COSTUMES
  if (ImGui::BeginMenu("Costumes")) {
    ImGui::MenuItem("Open Costumes", nullptr, &showCostumes_);
    ImGui::EndMenu();
  }

  // SETTINGS
  if (ImGui::BeginMenu("Settings")) {
    ImGui::MenuItem("Open Settings", nullptr, &showSettings_);
    ImGui::EndMenu();
  }

  // SOUND MANAGER
  if (ImGui::BeginMenu("Sound manager")) {
    ImGui::MenuItem("Open Sound Manager", nullptr, &showSounds_);
    ImGui::EndMenu();
  }

  // VIEW
  if (ImGui::BeginMenu("View")) {
    ImGui::MenuItem("Stage", nullptr, &showStage_);
    ImGui::MenuItem("Sprites", nullptr, &showSprites_);
    ImGui::MenuItem("Inspector", nullptr, &showInspector_);
    ImGui::MenuItem("Logger", nullptr, &showLogger_);
    if (penEnabled_) ImGui::MenuItem("Pen Tools", nullptr, &showPenTools_);
    ImGui::EndMenu();
  }

  // HELP
  if (ImGui::BeginMenu("Help")) {
    ImGui::MenuItem("Help Window", nullptr, &showHelp_);
    ImGui::EndMenu();
  }


  ImGui::EndMainMenuBar();
}

// ---------------- Layout Windows ----------------
void MainDockspace::drawLayoutWindows(Renderer2D& renderer) {
  if (!project_) return;

  static StagePanel stagePanel;
  static SpritePanel spritePanel;
  static InspectorPanel inspectorPanel;
  static BlocksPalettePanel blocksPanel;
  static ScriptWorkspacePanel workspacePanel;
  static CostumesPanel costumesPanel;
  static SoundsPanel soundsPanel;
  static SettingsPanel settingsPanel;
  static ExtensionsPanel extensionsPanel;
  static HelpPanel helpPanel;

  ImGuiViewport* vp = ImGui::GetMainViewport();
  ImVec2 workPos = vp->WorkPos;
  ImVec2 workSize = vp->WorkSize;

  const float pad = 8.0f;

  float bottomH = showLogger_ ? 220.0f : 0.0f;
  float rightW  = 420.0f;
  float leftW   = 320.0f;

  ImVec2 topLeft  = ImVec2(workPos.x + pad, workPos.y + pad);
  ImVec2 topRight = ImVec2(workPos.x + workSize.x - pad, workPos.y + workSize.y - pad);
  float usableH = (topRight.y - topLeft.y) - bottomH;

  // Left: Blocks
  if (showBlocks_) {
    ImGui::SetNextWindowPos(ImVec2(topLeft.x, topLeft.y + (penEnabled_ && showPenTools_ ? 260.0f : 0.0f)), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(leftW, usableH - (penEnabled_ && showPenTools_ ? 260.0f : 0.0f)), ImGuiCond_Always);
    blocksPanel.draw(*project_, &showBlocks_);
  }

  // Right column
  float rightX = topRight.x - rightW;
  float rightY = topLeft.y;

  float stageH = showStage_ ? (usableH * 0.55f) : 0.0f;
  float spritesH = showSprites_ ? 220.0f : 0.0f;
  float inspectorH = usableH - stageH - spritesH - ((showStage_ && showSprites_) ? pad * 2.0f : 0.0f);

  if (showStage_) {
    ImGui::SetNextWindowPos(ImVec2(rightX, rightY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rightW, stageH), ImGuiCond_Always);
    stagePanel.draw(*project_, renderer, &pen_);
    rightY += stageH + pad;
  }

  if (showSprites_) {
    ImGui::SetNextWindowPos(ImVec2(rightX, rightY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rightW, spritesH), ImGuiCond_Always);
    spritePanel.draw(*project_);
    selectedSpriteId_ = spritePanel.selectedSpriteId();
    rightY += spritesH + pad;
  }

  if (showInspector_) {
    ImGui::SetNextWindowPos(ImVec2(rightX, rightY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(rightW, std::max(120.0f, inspectorH)), ImGuiCond_Always);
    inspectorPanel.draw(*project_, selectedSpriteId_, editor_);
  }

  // Center workspace
  if (showWorkspace_) {
    float centerX = topLeft.x + (showBlocks_ ? (leftW + pad) : 0.0f);
    float centerW = (rightX - pad) - centerX;
    ImGui::SetNextWindowPos(ImVec2(centerX, topLeft.y), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(std::max(200.0f, centerW), usableH), ImGuiCond_Always);
    workspacePanel.draw(*project_, selectedSpriteId_, &showWorkspace_, editor_);
  }

  // Floating windows
  costumesPanel.draw(*project_, selectedSpriteId_, &showCostumes_);
  soundsPanel.draw(*project_, selectedSpriteId_, &showSounds_);
  settingsPanel.draw(*project_, &showSettings_);
  extensionsPanel.draw(*project_, &showExtensions_, &penEnabled_, selectedSpriteId_, &pen_);
  helpPanel.draw(&showHelp_);

  // ✅ Pen Tools area (special)
  if (penEnabled_ && showPenTools_) {
    DrawPenToolsPanel(&showPenTools_, penEnabled_, pen_);
  }

  // Logger bottom
  if (showLogger_) {
    ImGui::SetNextWindowPos(ImVec2(topLeft.x, topLeft.y + usableH + pad), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(workSize.x - pad * 2.0f, bottomH - pad), ImGuiCond_Always);
    drawLoggerPanel();
  }
}

// ---------------- Main draw ----------------
void MainDockspace::draw(Project& project, Renderer2D& renderer, Runtime& runtime) {
  project_ = &project;

  // sync pen
  pen_.setEnabled(penEnabled_);

  drawMenuBar(runtime);

  // popups
  if (openUnsavedPopup_) { ImGui::OpenPopup("Unsaved changes?"); openUnsavedPopup_ = false; }
  if (openSavePopup_)    { ImGui::OpenPopup("Save Project");     openSavePopup_ = false; }
  if (openLoadPopup_)    { ImGui::OpenPopup("Load Project");     openLoadPopup_ = false; }

  // Unsaved
  if (ImGui::BeginPopupModal("Unsaved changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Project has unsaved changes. Save before continuing?");
    ImGui::Separator();

    if (ImGui::Button("Save")) {
      ImGui::CloseCurrentPopup();
      askUnsaved_ = false;

      if (project_->filePath().empty()) popupSaveAs();
      else {
        doSaveProject(project_->filePath());
        if (!project_->isDirty()) {
          if (pending_ == PendingAction::NewProject) doNewProject();
          else if (pending_ == PendingAction::LoadProject) popupLoad();
          else if (pending_ == PendingAction::ExitApp) requestExit_ = true;
        }
        pending_ = PendingAction::None;
      }
    }

    ImGui::SameLine();
    if (ImGui::Button("Don't Save")) {
      ImGui::CloseCurrentPopup();
      askUnsaved_ = false;

      if (pending_ == PendingAction::NewProject) doNewProject();
      else if (pending_ == PendingAction::LoadProject) popupLoad();
      else if (pending_ == PendingAction::ExitApp) requestExit_ = true;

      pending_ = PendingAction::None;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
      askUnsaved_ = false;
      pending_ = PendingAction::None;
    }

    ImGui::EndPopup();
  }

  // Save
  if (ImGui::BeginPopupModal("Save Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Path:");
    ImGui::InputText("##savepath", pathBuf_, sizeof(pathBuf_));

    if (ImGui::Button("Save")) {
      doSaveProject(pathBuf_);
      ImGui::CloseCurrentPopup();

      if (!project_->isDirty()) {
        if (pending_ == PendingAction::NewProject) doNewProject();
        else if (pending_ == PendingAction::LoadProject) popupLoad();
        else if (pending_ == PendingAction::ExitApp) requestExit_ = true;
        pending_ = PendingAction::None;
      }
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) ImGui::CloseCurrentPopup();

    ImGui::EndPopup();
  }

  // Load
  if (ImGui::BeginPopupModal("Load Project", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::Text("Path:");
    ImGui::InputText("##loadpath", pathBuf_, sizeof(pathBuf_));

    if (ImGui::Button("Load")) {
      doLoadProject(pathBuf_);
      ImGui::CloseCurrentPopup();
      pending_ = PendingAction::None;
    }

    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
      pending_ = PendingAction::None;
    }

    ImGui::EndPopup();
  }

  // Ask/Answer (Sensing)
  if (project.askActive()) {
    ImGui::OpenPopup("Question");
  }
  if (ImGui::BeginPopupModal("Question", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::TextWrapped("%s", project.askPrompt().c_str());
    static char ansBuf[256] = {0};
    // keep buffer synced while popup is open
    if (ImGui::IsWindowAppearing()) {
      std::snprintf(ansBuf, sizeof(ansBuf), "%s", project.askDraft().c_str());
    }
    if (ImGui::InputText("##answer", ansBuf, sizeof(ansBuf), ImGuiInputTextFlags_EnterReturnsTrue)) {
      project.setAskDraft(ansBuf);
      project.submitAskAnswer();
      ImGui::CloseCurrentPopup();
    }
    if (ImGui::Button("OK")) {
      project.setAskDraft(ansBuf);
      project.submitAskAnswer();
      ImGui::CloseCurrentPopup();
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      project.setAskDraft(std::string{});
      project.submitAskAnswer();
      ImGui::CloseCurrentPopup();
    }
    ImGui::EndPopup();
  }


  drawLayoutWindows(renderer);
}