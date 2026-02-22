#pragma once
#include <string>
#include <vector>

#include <array>
#include <SDL.h>

#include "model/Stage.h"
#include "model/Sprite.h"
#include "core/IdGen.h"
#include "model/Block.h"
#include "model/Script.h"

class Project {
public:
  void resetToDefault();          // New
  void newProject(); //reset everything, clear path/dirty
  bool isDirty() const { return dirty_; }
  void markDirty() { dirty_ = true; }
  void clearDirty() { dirty_ = false; }

  const std::string& filePath() const { return filePath_; }
  void setFilePath(std::string p) { filePath_ = std::move(p); }

  Stage& stage() { return stage_; }
  const Stage& stage() const { return stage_; }

  std::vector<Sprite>& sprites() { return sprites_; }
  const std::vector<Sprite>& sprites() const { return sprites_; }

  Sprite* findSpriteById(int id);

  // --- IDs ---
  int allocSpriteId() { return spriteIds_.next(); }
  int allocScriptId() { return scriptIds_.next(); }
  int allocBlockId()  { return blockIds_.next(); }

  // after loading from file
  void ensureNextIds(int nextSprite, int nextScript, int nextBlock);

  // --- blocks/scripts ---
  Script* findScript(Sprite& sp, int scriptId);
  Block*  findBlock(Sprite& sp, int blockId);

  int createScript(Sprite& sp, float x, float y, BlockType hatType);
  int appendBlockToScript(Sprite& sp, int scriptId, BlockType type);

  // deletion / linking helpers
  bool deleteBlock(Sprite& sp, int blockId);       // delete single block and re-link stack
  bool deleteScript(Sprite& sp, int scriptId);     // delete script and its block chain
  bool linkAfter(Sprite& sp, int afterBlockId, int newNextId); // set after->next = newNext (safe)
  int  findPrevInAnyChain(const Sprite& sp, int targetId) const;

  // chain helpers
  int appendToChain(Sprite& sp, int& headId, BlockType type, float x, float y);

  // delete helpers (recursive)
  void deleteChainRecursive(Sprite& sp, int headId);

    // --- mouse on stage (transient; not serialized) ---
  void setMouseWorld(float x, float y, bool valid) { mouseX_ = x; mouseY_ = y; mouseValid_ = valid; }
  bool mouseWorldValid() const { return mouseValid_; }
  float mouseWorldX() const { return mouseX_; }
  float mouseWorldY() const { return mouseY_; }

  // --- input states (transient; not serialized) ---
  void setKeyDown(SDL_Scancode sc, bool down) {
    if ((int)sc >= 0 && (int)sc < (int)keysDown_.size()) keysDown_[(size_t)sc] = down;
  }
  bool keyDown(SDL_Scancode sc) const {
    if ((int)sc >= 0 && (int)sc < (int)keysDown_.size()) return keysDown_[(size_t)sc];
    return false;
  }
  void setMouseButtonDown(bool down) { mouseDown_ = down; }
  bool mouseDown() const { return mouseDown_; }

  // --- ask/answer (transient; not serialized) ---
  bool askActive() const { return askActive_; }
  const std::string& askPrompt() const { return askPrompt_; }
  const std::string& askDraft() const { return askDraft_; }
  const std::string& answer() const { return answer_; }
  void beginAsk(std::string prompt);
  void setAskDraft(std::string v) { askDraft_ = std::move(v); }
  void submitAskAnswer();
  bool consumeAskAnswered();

  // --- runtime control (transient; not serialized) ---
  void requestStopAllScripts() { stopAllScriptsRequested_ = true; }
  bool consumeStopAllScriptsRequest() {
    bool v = stopAllScriptsRequested_;
    stopAllScriptsRequested_ = false;
    return v;
  }

  // --- file drop (transient) ---
  void setDroppedFile(std::string p) { droppedFile_ = std::move(p); }
  std::string consumeDroppedFile() {
    std::string t = droppedFile_;
    droppedFile_.clear();
    return t;
  }

private:
  bool dirty_{false};
  std::string filePath_;
  Stage stage_;
  std::vector<Sprite> sprites_;

  IdGen spriteIds_;
  IdGen scriptIds_;
  IdGen blockIds_;
  float mouseX_{0}, mouseY_{0};
  bool mouseValid_{false};

  std::array<bool, SDL_NUM_SCANCODES> keysDown_{};
  bool mouseDown_{false};

  bool askActive_{false};
  bool askAnswered_{false};
  std::string askPrompt_;
  std::string askDraft_;
  std::string answer_;

  bool stopAllScriptsRequested_{false};
  std::string droppedFile_;
};