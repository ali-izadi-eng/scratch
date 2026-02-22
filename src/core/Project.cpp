#include "Project.h"
#include "core/Logger.h"
#include <unordered_set>
#include <algorithm>

Sprite* Project::findSpriteById(int id) {
  for (auto& s : sprites_) if (s.id == id) return &s;
  return nullptr;
}

void Project::newProject() {
  *this = Project{};
  resetToDefault();
  setFilePath("");
  clearDirty();
}

void Project::resetToDefault() {
  stage_ = Stage{};
  sprites_.clear();
  filePath_.clear();
  dirty_ = false;
  spriteIds_.reset(1);
  blockIds_.reset(1);
  scriptIds_.reset(1);

  keysDown_.fill(false);
  mouseDown_ = false;
  askActive_ = false;
  askAnswered_ = false;
  askPrompt_.clear();
  askDraft_.clear();
  answer_.clear();
  stopAllScriptsRequested_ = false;
  droppedFile_.clear();

  // پروژه پیش‌فرض: یک Sprite با یک costume خالی (فعلاً بدون تصویر)
  Sprite s;
  s.id = allocSpriteId();
  s.name = "Sprite1";
  s.x = 0;
  s.y = 0;
  s.visible = true;
  s.sizePercent = 100; // پیش‌فرض :contentReference[oaicite:5]{index=5}
  sprites_.push_back(std::move(s));
}

void Project::beginAsk(std::string prompt) {
  askPrompt_ = std::move(prompt);
  askDraft_.clear();
  askActive_ = true;
  askAnswered_ = false;
}

void Project::submitAskAnswer() {
  answer_ = askDraft_;
  askActive_ = false;
  askAnswered_ = true;
}

bool Project::consumeAskAnswered() {
  bool v = askAnswered_;
  askAnswered_ = false;
  return v;
}


Script* Project::findScript(Sprite& sp, int scriptId) {
  for (auto& sc : sp.scripts) if (sc.id == scriptId) return &sc;
  return nullptr;
}

Block* Project::findBlock(Sprite& sp, int blockId) {
  auto it = sp.blocks.find(blockId);
  if (it == sp.blocks.end()) return nullptr;
  return &it->second;
}

int Project::createScript(Sprite& sp, float x, float y, BlockType hatType) {
  Script sc;
  sc.id = allocScriptId();

  Block hat;
  hat.id = allocBlockId();
  hat.type = hatType;
  hat.x = x;
  hat.y = y;
  hat.nextId = -1;

  sc.headBlockId = hat.id;
  sp.blocks[hat.id] = hat;
  sp.scripts.push_back(sc);
  sp.selectedScriptId = sc.id;

  markDirty();
  Logger::info("Script", "Created script id=" + std::to_string(sc.id));
  return sc.id;
}

int Project::appendBlockToScript(Sprite& sp, int scriptId, BlockType type) {
  Script* sc = findScript(sp, scriptId);
  if (!sc) return -1;

  // پیدا کردن tail
  int curId = sc->headBlockId;
  if (curId < 0) return -1;

  Block* cur = findBlock(sp, curId);
  while (cur && cur->nextId != -1) {
    cur = findBlock(sp, cur->nextId);
  }

  Block b;
  b.id = allocBlockId();
  b.type = type;
  b.x = (cur ? cur->x : 0) + 0.0f;
  b.y = (cur ? cur->y : 0) + 60.0f;
  b.nextId = -1;

  // args defaults
  switch (type) {
    case BlockType::MoveSteps: b.args = {"10"}; break;
    case BlockType::TurnRight: b.args = {"15"}; break;
    case BlockType::TurnLeft:  b.args = {"15"}; break;
    case BlockType::GoToXY:    b.args = {"0","0"}; break;
    case BlockType::Say:       b.args = {"Hello!"}; break;
    case BlockType::WaitSeconds: b.args = {"1"}; break;
    case BlockType::WaitUntil:   b.args = {"touching edge"}; break;
    case BlockType::SetX:       b.args = {"0"}; break;
    case BlockType::SetY:       b.args = {"0"}; break;
    case BlockType::ChangeXBy:  b.args = {"10"}; break;
    case BlockType::ChangeYBy:  b.args = {"10"}; break;
    case BlockType::GoToRandomPosition: b.args = {}; break;
    case BlockType::GoToMousePointer:   b.args = {}; break;
    case BlockType::IfOnEdgeBounce:     b.args = {}; break;
    case BlockType::StopAtEdge:         b.args = {}; break;
    case BlockType::Think:            b.args = {"Hmm..."}; break;

    // Control
    case BlockType::Repeat:       b.args = {"10"}; break;
    case BlockType::RepeatUntil:  b.args = {"touching edge"}; break;
    case BlockType::Forever:      b.args = {}; break;
    case BlockType::IfThen:       b.args = {"touching edge"}; break;
    case BlockType::StopThisScript: b.args = {}; break;
    case BlockType::StopAll:        b.args = {}; break;

    // Sensing
    case BlockType::AskAndWait:   b.args = {"What's your name?"}; break;

    case BlockType::SwitchCostumeTo:  b.args = {"1"}; break;   // index as string
    case BlockType::NextCostume:      b.args = {}; break;

    case BlockType::SwitchBackdropTo: b.args = {"1"}; break;   // index
    case BlockType::NextBackdrop:     b.args = {}; break;

    case BlockType::SetSizeTo:        b.args = {"100"}; break; // percent
    case BlockType::ChangeSizeBy:     b.args = {"10"}; break;

    case BlockType::Show:             b.args = {}; break;
    case BlockType::Hide:             b.args = {}; break;

    case BlockType::GoToFrontLayer:   b.args = {}; break;
    case BlockType::GoBackLayers:     b.args = {"1"}; break;

    case BlockType::LooksReporter:    b.args = {"costume #"}; break;

    // Sound (24-26)
    case BlockType::PlaySound:          b.args = {"1"}; break; // sound #1 by default
    case BlockType::PlaySoundUntilDone: b.args = {"1"}; break;
    case BlockType::StopAllSounds:      b.args = {}; break;
    case BlockType::SetVolumeTo:        b.args = {"100"}; break;
    case BlockType::ChangeVolumeBy:     b.args = {"10"}; break;
    case BlockType::SetPitchTo:         b.args = {"0"}; break;
    case BlockType::ChangePitchBy:      b.args = {"1"}; break;
    default: break;
  }

  sp.blocks[b.id] = b;
  if (cur) cur->nextId = b.id;

  markDirty();
  return b.id;
}

void Project::ensureNextIds(int nextSprite, int nextScript, int nextBlock) {
  spriteIds_.ensureAtLeast(nextSprite);
  scriptIds_.ensureAtLeast(nextScript);
  blockIds_.ensureAtLeast(nextBlock);
}


int Project::findPrevInAnyChain(const Sprite& sp, int targetId) const {
  // search through scripts for prev of target
  for (const auto& sc : sp.scripts) {
    int cur = sc.headBlockId;
    int safety = 0;
    while (cur != -1 && safety++ < 1000) {
      auto it = sp.blocks.find(cur);
      if (it == sp.blocks.end()) break;
      int nx = it->second.nextId;
      if (nx == targetId) return cur;
      cur = nx;
    }
  }
  return -1;
}

bool Project::linkAfter(Sprite& sp, int afterBlockId, int newNextId) {
  auto it = sp.blocks.find(afterBlockId);
  if (it == sp.blocks.end()) return false;
  it->second.nextId = newNextId;
  markDirty();
  return true;
}

int Project::appendToChain(Sprite& sp, int& headId, BlockType type, float x, float y) {
  // create new block with defaults
  Block b;
  b.id = allocBlockId();
  b.type = type;
  b.x = x;
  b.y = y;
  b.nextId = -1;
  b.childHeadId = -1;

  switch (type) {
    case BlockType::MoveSteps: b.args = {"10"}; break;
    case BlockType::TurnRight: b.args = {"15"}; break;
    case BlockType::TurnLeft:  b.args = {"15"}; break;
    case BlockType::GoToXY:    b.args = {"0","0"}; break;
    case BlockType::Say:       b.args = {"Hello!"}; break;
    case BlockType::WaitSeconds: b.args = {"1"}; break;

    case BlockType::Repeat:       b.args = {"10"}; break;
    case BlockType::RepeatUntil:  b.args = {"touching edge"}; break;
    case BlockType::Forever:      b.args = {}; break;
    case BlockType::IfThen:       b.args = {"touching edge"}; break;
    case BlockType::StopThisScript: b.args = {}; break;
    case BlockType::StopAll:        b.args = {}; break;

    // Sensing
    case BlockType::AskAndWait:   b.args = {"What's your name?"}; break;

    // Sound (24-26)
    case BlockType::PlaySound:          b.args = {"1"}; break;
    case BlockType::PlaySoundUntilDone: b.args = {"1"}; break;
    case BlockType::StopAllSounds:      b.args = {}; break;
    case BlockType::SetVolumeTo:        b.args = {"100"}; break;
    case BlockType::ChangeVolumeBy:     b.args = {"10"}; break;
    case BlockType::SetPitchTo:         b.args = {"0"}; break;
    case BlockType::ChangePitchBy:      b.args = {"1"}; break;
    default: break;
  }

  sp.blocks[b.id] = b;

  if (headId == -1) {
    headId = b.id;
    markDirty();
    return b.id;
  }

  // find tail
  int cur = headId;
  int safety = 0;
  while (cur != -1 && safety++ < 2000) {
    auto it = sp.blocks.find(cur);
    if (it == sp.blocks.end()) break;
    if (it->second.nextId == -1) {
      it->second.nextId = b.id;
      break;
    }
    cur = it->second.nextId;
  }

  markDirty();
  return b.id;
}


void Project::deleteChainRecursive(Sprite& sp, int headId) {
  int cur = headId;
  int safety = 0;
  while (cur != -1 && safety++ < 5000) {
    auto it = sp.blocks.find(cur);
    if (it == sp.blocks.end()) break;

    int next = it->second.nextId;
    int child = it->second.childHeadId;

    // delete child first
    if (child != -1) {
      deleteChainRecursive(sp, child);
    }

    sp.blocks.erase(cur);
    cur = next;
  }
}


bool Project::deleteScript(Sprite& sp, int scriptId) {
  auto itSc = std::find_if(sp.scripts.begin(), sp.scripts.end(),
                           [&](const Script& s){ return s.id == scriptId; });
  if (itSc == sp.scripts.end()) return false;

  deleteChainRecursive(sp, itSc->headBlockId);
  sp.scripts.erase(itSc);

  if (sp.selectedScriptId == scriptId) {
    sp.selectedScriptId = sp.scripts.empty() ? 0 : sp.scripts.front().id;
  }

  markDirty();
  return true;
}


bool Project::deleteBlock(Sprite& sp, int blockId) {
  auto itDel = sp.blocks.find(blockId);
  if (itDel == sp.blocks.end()) return false;

  int next = itDel->second.nextId;

  // head of a script?
  for (auto& sc : sp.scripts) {
    if (sc.headBlockId == blockId) {
      if (itDel->second.childHeadId != -1) deleteChainRecursive(sp, itDel->second.childHeadId);
      sp.blocks.erase(blockId);
      sc.headBlockId = next;
      markDirty();
      return true;
    }
  }

  // prev in any script chain?
  int prev = findPrevInAnyChain(sp, blockId);
  if (prev != -1) {
    auto itPrev = sp.blocks.find(prev);
    if (itPrev != sp.blocks.end()) {
      itPrev->second.nextId = next;
    }
    if (itDel->second.childHeadId != -1) deleteChainRecursive(sp, itDel->second.childHeadId);
    sp.blocks.erase(blockId);
    markDirty();
    return true;
  }

  // detached
  if (itDel->second.childHeadId != -1) deleteChainRecursive(sp, itDel->second.childHeadId);
  sp.blocks.erase(blockId);
  markDirty();
  return true;
}