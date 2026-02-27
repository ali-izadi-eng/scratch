#pragma once

#include <string>

#include "core/Project.h"
#include "renderer/Renderer2D.h"
#include "runtime/Runtime.h"
#include "ui/EditorState.h"
#include "extensions/PenSystem.h"

class MainDockspace {
public:
  MainDockspace();

  void draw(Project& project, Renderer2D& renderer, Runtime& runtime);

  bool wantsExit() const { return requestExit_; }

private:
  enum class PendingAction { None, NewProject, LoadProject, ExitApp };

  void drawMenuBar(Runtime& runtime);
  void drawLoggerPanel();
  void drawLayoutWindows(Renderer2D& renderer);
  void drawAskPopup();

  void doNewProject();
  void doSaveProject(const std::string& path);
  void doLoadProject(const std::string& path);

  // these only set flags/buffers (OpenPopup happens in draw())
  void popupUnsavedChanges();
  void popupSaveAs();
  void popupLoad();
  

private:
  Project* project_ = nullptr;

  // visibility toggles
  bool showStage_ = true;
  bool showSprites_ = true;
  bool showInspector_ = true;
  bool showBlocks_ = true;
  bool showWorkspace_ = true;
  bool showLogger_ = true;

  bool showCostumes_ = false;
  bool showSounds_ = false;
  bool showSettings_ = false;
  bool showExtensions_ = false;
  bool showHelp_ = false;

  bool penEnabled_ = false;

  bool runRequested_ = false;
  bool pauseRequested_ = false;

  // popups
  bool openUnsavedPopup_ = false;
  bool openSavePopup_ = false;
  bool openLoadPopup_ = false;

  bool askUnsaved_ = false;
  PendingAction pending_ = PendingAction::None;

  // buffers & state
  char pathBuf_[512]{};
  std::string lastError_;

  int selectedSpriteId_ = 0;
  EditorState editor_;


  // step mode UI -> App control
  bool stepMode_ = false;
  bool stepOnce_ = false;
  PenSystem pen_;

  bool isStepMode() const { return stepMode_; }
  bool consumeStepOnce() { bool r = stepOnce_; stepOnce_ = false; return r; }
  bool wantsPause() const { return pauseRequested_; }
  bool wantsResume() const { return !pauseRequested_; } // یا جداگانه بگذار

  bool showPenTools_ = true;   // پنل ابزار Pen
  bool requestStamp_ = false;

  bool requestExit_ = false;
};