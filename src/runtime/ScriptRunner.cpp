#include "runtime/ScriptRunner.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <cstdlib>

#include "core/Logger.h"
#include "audio/AudioEngine.h"
#include "ScriptRunner.h"

static bool isHat(BlockType t) {
  return t == BlockType::WhenGreenFlag || t == BlockType::WhenKeyPressed;
}

static const Sound* findSoundByArg(const Sprite& sp, const std::string& arg) {
  if (sp.sounds.empty()) return nullptr;
  if (arg.empty()) return &sp.sounds.front();

  // numeric 1-based index
  char* end = nullptr;
  long idx = std::strtol(arg.c_str(), &end, 10);
  if (end && end != arg.c_str() && *end == '\0') {
    long i = idx - 1;
    if (i >= 0 && i < (long)sp.sounds.size()) return &sp.sounds[(size_t)i];
  }

  for (const auto& s : sp.sounds) {
    if (s.name == arg) return &s;
  }
  return nullptr;
}

float ScriptRunner::toFloat(const std::string& s, float def) {
  if (s.empty()) return def;
  char* end = nullptr;
  float v = std::strtof(s.c_str(), &end);
  if (end == s.c_str()) return def;
  return v;
}

std::string ScriptRunner::trim(std::string s) {
  auto notSpace = [](unsigned char c) { return !std::isspace(c); };
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), notSpace));
  s.erase(std::find_if(s.rbegin(), s.rend(), notSpace).base(), s.end());
  return s;
}

std::string ScriptRunner::lower(std::string s) {
  for (char& c : s) c = (char)std::tolower((unsigned char)c);
  return s;
}

bool ScriptRunner::cmp(float a, const std::string& op, float b) {
  if (op == "<")  return a < b;
  if (op == "<=") return a <= b;
  if (op == ">")  return a > b;
  if (op == ">=") return a >= b;
  if (op == "==") return a == b;
  if (op == "!=") return a != b;
  return false;
}

bool ScriptRunner::parseDistanceComparison(const std::string& expr, std::string* outTarget, std::string* outOp, float* outValue) {
  // forms:
  //   distance to mouse < 50
  //   distance to Sprite1 <= 30
  std::string s = trim(lower(expr));
  const std::string p = "distance to ";
  if (s.rfind(p, 0) != 0) return false;
  s = s.substr(p.size());

  // split target and rest
  // find first space before operator
  size_t sp1 = s.find(' ');
  if (sp1 == std::string::npos) return false;
  std::string target = trim(s.substr(0, sp1));
  std::string rest = trim(s.substr(sp1));

  // op could be <= >= == != < >
  static const char* ops[] = {"<=", ">=", "==", "!=", "<", ">"};
  std::string op;
  for (auto* o : ops) {
    if (rest.rfind(o, 0) == 0) { op = o; rest = trim(rest.substr(op.size())); break; }
  }
  if (op.empty()) return false;
  float v = toFloat(rest, NAN);
  if (std::isnan(v)) return false;
  if (outTarget) *outTarget = target;
  if (outOp) *outOp = op;
  if (outValue) *outValue = v;
  return true;
}

bool ScriptRunner::parseMouseComparison(const std::string& expr, char axis, std::string* outOp, float* outValue) {
  // mouse x > 0
  // mouse y <= -50
  std::string s = trim(lower(expr));
  std::string key = (axis == 'x') ? "mouse x" : "mouse y";
  if (s.rfind(key, 0) != 0) return false;
  s = trim(s.substr(key.size()));

  static const char* ops[] = {"<=", ">=", "==", "!=", "<", ">"};
  std::string op;
  for (auto* o : ops) {
    if (s.rfind(o, 0) == 0) { op = o; s = trim(s.substr(op.size())); break; }
  }
  if (op.empty()) return false;
  float v = toFloat(s, NAN);
  if (std::isnan(v)) return false;
  if (outOp) *outOp = op;
  if (outValue) *outValue = v;
  return true;
}

bool ScriptRunner::touching(Project& project, const Sprite& sp, const std::string& target) const {
  std::string t = trim(lower(target));
  if (t.empty()) return false;

  // edge bounds (Scratch-like)
  if (t == "edge") {
    return (sp.x <= -240.0f || sp.x >= 240.0f || sp.y <= -180.0f || sp.y >= 180.0f);
  }

  if (t == "mouse") {
    if (!project.mouseWorldValid()) return false;
    float dx = sp.x - project.mouseWorldX();
    float dy = sp.y - project.mouseWorldY();
    return (dx*dx + dy*dy) <= (15.0f*15.0f);
  }

  // by sprite name
  for (const auto& other : project.sprites()) {
    if (lower(other.name) == t) {
      float dx = sp.x - other.x;
      float dy = sp.y - other.y;
      return (dx*dx + dy*dy) <= (20.0f*20.0f);
    }
  }

  // by 1-based index
  char* end = nullptr;
  long idx = std::strtol(t.c_str(), &end, 10);
  if (end && end != t.c_str() && *end == '\0') {
    long i = idx - 1;
    if (i >= 0 && i < (long)project.sprites().size()) {
      const auto& other = project.sprites()[(size_t)i];
      float dx = sp.x - other.x;
      float dy = sp.y - other.y;
      return (dx*dx + dy*dy) <= (20.0f*20.0f);
    }
  }

  return false;
}

float ScriptRunner::distanceTo(Project& project, const Sprite& sp, const std::string& target) const {
  std::string t = trim(lower(target));
  if (t == "mouse") {
    if (!project.mouseWorldValid()) return 1e9f;
    float dx = sp.x - project.mouseWorldX();
    float dy = sp.y - project.mouseWorldY();
    return std::sqrt(dx*dx + dy*dy);
  }

  for (const auto& other : project.sprites()) {
    if (lower(other.name) == t) {
      float dx = sp.x - other.x;
      float dy = sp.y - other.y;
      return std::sqrt(dx*dx + dy*dy);
    }
  }

  char* end = nullptr;
  long idx = std::strtol(t.c_str(), &end, 10);
  if (end && end != t.c_str() && *end == '\0') {
    long i = idx - 1;
    if (i >= 0 && i < (long)project.sprites().size()) {
      const auto& other = project.sprites()[(size_t)i];
      float dx = sp.x - other.x;
      float dy = sp.y - other.y;
      return std::sqrt(dx*dx + dy*dy);
    }
  }

  return 1e9f;
}

bool ScriptRunner::evalCondition(Project& project, Sprite& sp, const std::string& expr) const {
  std::string s = trim(lower(expr));
  if (s.empty()) return true;
  if (s == "true") return true;
  if (s == "false" || s == "0") return false;

  // touching ...
  if (s.rfind("touching ", 0) == 0) {
    return touching(project, sp, trim(s.substr(std::string("touching ").size())));
  }

  // key ...
  if (s.rfind("key ", 0) == 0) {
    std::string key = trim(s.substr(4));
    SDL_Scancode sc = SDL_SCANCODE_UNKNOWN;
    if (key == "space") sc = SDL_SCANCODE_SPACE;
    else if (key == "left") sc = SDL_SCANCODE_LEFT;
    else if (key == "right") sc = SDL_SCANCODE_RIGHT;
    else if (key == "up") sc = SDL_SCANCODE_UP;
    else if (key == "down") sc = SDL_SCANCODE_DOWN;
    else if (key.size() == 1 && key[0] >= 'a' && key[0] <= 'z') {
      sc = (SDL_Scancode)(SDL_SCANCODE_A + (key[0] - 'a'));
    }
    return (sc != SDL_SCANCODE_UNKNOWN) ? project.keyDown(sc) : false;
  }

  if (s == "mouse down") return project.mouseDown();

  // distance comparisons
  std::string tgt, op;
  float val = 0;
  if (parseDistanceComparison(s, &tgt, &op, &val)) {
    float d = distanceTo(project, sp, tgt);
    return cmp(d, op, val);
  }

  // mouse x/y comparisons
  float mv = 0;
  if (parseMouseComparison(s, 'x', &op, &val)) {
    mv = project.mouseWorldValid() ? project.mouseWorldX() : 0.0f;
    return cmp(mv, op, val);
  }
  if (parseMouseComparison(s, 'y', &op, &val)) {
    mv = project.mouseWorldValid() ? project.mouseWorldY() : 0.0f;
    return cmp(mv, op, val);
  }

  // fallback: treat as truthy string
  return true;
}

void ScriptRunner::start(Project& project, int spriteId, int scriptId) {
  spriteId_ = spriteId;
  scriptId_ = scriptId;

  finished_ = false;
  paused_ = false;
  waitRemaining_ = 0.0f;
  waitingAsk_ = false;
  stack_.clear();

  Sprite* sp = project.findSpriteById(spriteId_);
  if (!sp) { finished_ = true; return; }

  Script* sc = project.findScript(*sp, scriptId_);
  if (!sc) { finished_ = true; return; }

  Frame f;
  f.headId = sc->headBlockId;
  f.currentId = sc->headBlockId;
  stack_.push_back(f);

  Logger::info("Run", "Start sprite=" + std::to_string(spriteId_) + " script=" + std::to_string(scriptId_));
}

void ScriptRunner::stop() {
  finished_ = true;
  paused_ = false;
  waitRemaining_ = 0.0f;
  waitingAsk_ = false;
  stack_.clear();
}

Sprite* ScriptRunner::curSprite(Project& project) {
  return project.findSpriteById(spriteId_);
}

Block* ScriptRunner::curBlock(Project& project) {
  Sprite* sp = curSprite(project);
  if (!sp) return nullptr;
  if (stack_.empty()) return nullptr;
  int id = stack_.back().currentId;
  if (id == -1) return nullptr;
  return project.findBlock(*sp, id);
}

bool ScriptRunner::stepOnce(Project& project, float dt) {
  if (finished_ || paused_) return false;

  Sprite* sp = curSprite(project);
  if (!sp) { finished_ = true; return false; }
  if (stack_.empty()) { finished_ = true; return false; }

  // wait timer
  if (waitRemaining_ > 0.0f) {
    waitRemaining_ -= dt;
    if (waitRemaining_ > 0.0f) return true;
    waitRemaining_ = 0.0f;
  }

  // ask-and-wait blocking
  if (waitingAsk_) {
    if (project.consumeAskAnswered()) {
      waitingAsk_ = false;
    } else {
      return true;
    }
  }

  Frame& fr = stack_.back();
  if (fr.currentId == -1) {
    stack_.pop_back();
    if (stack_.empty()) { finished_ = true; return false; }
    return true;
  }

  Block* b = project.findBlock(*sp, fr.currentId);
  if (!b) {
    fr.currentId = -1;
    return true;
  }

  if (isHat(b->type)) {
    fr.currentId = b->nextId;
    return true;
  }

  switch (b->type) {
    // ---------------- Motion ----------------
    case BlockType::MoveSteps: {
      float steps = b->args.empty() ? 10.0f : toFloat(b->args[0], 10.0f);
      float rad = (sp->directionDeg - 90.0f) * 3.1415926f / 180.0f;
      sp->x += std::cos(rad) * steps;
      sp->y += std::sin(rad) * steps;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::TurnRight: {
      float deg = b->args.empty() ? 15.0f : toFloat(b->args[0], 15.0f);
      sp->directionDeg += deg;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::TurnLeft: {
      float deg = b->args.empty() ? 15.0f : toFloat(b->args[0], 15.0f);
      sp->directionDeg -= deg;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::GoToXY: {
      float x = (b->args.size() >= 1) ? toFloat(b->args[0], 0.0f) : 0.0f;
      float y = (b->args.size() >= 2) ? toFloat(b->args[1], 0.0f) : 0.0f;
      sp->x = x;
      sp->y = y;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::SetX: {
      sp->x = b->args.empty() ? 0.0f : toFloat(b->args[0], 0.0f);
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::SetY: {
      sp->y = b->args.empty() ? 0.0f : toFloat(b->args[0], 0.0f);
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::ChangeXBy: {
      sp->x += b->args.empty() ? 10.0f : toFloat(b->args[0], 10.0f);
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::ChangeYBy: {
      sp->y += b->args.empty() ? 10.0f : toFloat(b->args[0], 10.0f);
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::GoToRandomPosition: {
      float rx = -240.0f + (std::rand() / (float)RAND_MAX) * 480.0f;
      float ry = -180.0f + (std::rand() / (float)RAND_MAX) * 360.0f;
      sp->x = rx;
      sp->y = ry;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::GoToMousePointer: {
      if (project.mouseWorldValid()) {
        sp->x = project.mouseWorldX();
        sp->y = project.mouseWorldY();
      }
      fr.currentId = b->nextId;
      break;
    }

    // ---------------- Looks ----------------
    case BlockType::Say: {
      std::string msg = b->args.empty() ? "Hello!" : b->args[0];
      // {answer} token
      size_t pos = msg.find("{answer}");
      if (pos != std::string::npos) msg.replace(pos, 8, project.answer());
      sp->sayText = msg;
      sp->sayTimeRemaining = 2.0f;
      fr.currentId = b->nextId;
      break;
    }
    case BlockType::Think: {
      std::string msg = b->args.empty() ? "Hmm..." : b->args[0];
      size_t pos = msg.find("{answer}");
      if (pos != std::string::npos) msg.replace(pos, 8, project.answer());
      sp->sayText = msg;
      sp->sayTimeRemaining = 2.0f;
      fr.currentId = b->nextId;
      break;
    }

    // ---------------- Control ----------------
    case BlockType::WaitSeconds: {
      float sec = b->args.empty() ? 1.0f : toFloat(b->args[0], 1.0f);
      waitRemaining_ = std::max(0.0f, sec);
      fr.currentId = b->nextId;
      break;
    }

    case BlockType::WaitUntil: {
      std::string cond = b->args.empty() ? std::string{} : b->args[0];
      if (evalCondition(project, *sp, cond)) {
        fr.currentId = b->nextId;
      } else {
        fr.currentId = b->id; // stay
      }
      break;
    }

    case BlockType::IfThen: {
      std::string cond = b->args.empty() ? std::string{} : b->args[0];
      bool ok = evalCondition(project, *sp, cond);
      if (ok && b->childHeadId != -1) {
        Frame child;
        child.headId = b->childHeadId;
        child.currentId = b->childHeadId;
        child.ownerControlId = b->id;
        stack_.push_back(child);
      } else {
        fr.currentId = b->nextId;
      }
      break;
    }

    case BlockType::Repeat: {
      int n = 10;
      if (!b->args.empty()) n = (int)toFloat(b->args[0], 10.0f);
      n = std::max(0, n);

      if (b->childHeadId == -1 || n == 0) {
        fr.currentId = b->nextId;
        break;
      }

      Frame ctrl;
      ctrl.ownerControlId = b->id;
      ctrl.repeatRemaining = n;
      ctrl.controlMode = 1;
      ctrl.headId = b->childHeadId;
      ctrl.currentId = b->childHeadId;
      stack_.push_back(ctrl);
      break;
    }

    case BlockType::RepeatUntil: {
      if (b->childHeadId == -1) {
        fr.currentId = b->nextId;
        break;
      }
      std::string cond = b->args.empty() ? std::string{} : b->args[0];
      if (evalCondition(project, *sp, cond)) {
        fr.currentId = b->nextId;
        break;
      }
      Frame ctrl;
      ctrl.ownerControlId = b->id;
      ctrl.repeatRemaining = 0;
      ctrl.controlMode = 3;
      ctrl.headId = b->childHeadId;
      ctrl.currentId = b->childHeadId;
      stack_.push_back(ctrl);
      break;
    }

    case BlockType::Forever: {
      if (b->childHeadId == -1) {
        fr.currentId = b->nextId;
        break;
      }
      Frame ctrl;
      ctrl.ownerControlId = b->id;
      ctrl.repeatRemaining = -1;
      ctrl.controlMode = 2;
      ctrl.headId = b->childHeadId;
      ctrl.currentId = b->childHeadId;
      stack_.push_back(ctrl);
      break;
    }

    case BlockType::StopThisScript: {
      stop();
      return false;
    }

    case BlockType::StopAll: {
      project.requestStopAllScripts();
      stop();
      return false;
    }

    // ---------------- Sensing ----------------
    case BlockType::AskAndWait: {
      std::string q = b->args.empty() ? std::string("?") : b->args[0];
      project.beginAsk(q);
      waitingAsk_ = true;
      fr.currentId = b->nextId;
      break;
    }

    // ---------------- Sound ----------------
    case BlockType::PlaySound:
    case BlockType::PlaySoundUntilDone: {
      std::string which = b->args.empty() ? std::string{} : b->args[0];
      const Sound* snd = findSoundByArg(*sp, which);
      if (!snd) {
        Logger::warn("Sound", "No sound found for arg='" + which + "'");
        fr.currentId = b->nextId;
        break;
      }
      if (snd->muted || snd->volume <= 0.0f) {
        fr.currentId = b->nextId;
        break;
      }

      float vol = (sp->soundVolume / 100.0f) * snd->volume;
      vol = std::clamp(vol, 0.0f, 1.0f);
      float pitch = sp->soundPitch;

      auto pr = AudioEngine::instance().playWav(snd->filePath, vol, pitch);
      if (b->type == BlockType::PlaySoundUntilDone && pr.durationSec > 0.0f) {
        waitRemaining_ = pr.durationSec;
      }

      fr.currentId = b->nextId;
      break;
    }

    case BlockType::StopAllSounds: {
      AudioEngine::instance().stopAll();
      fr.currentId = b->nextId;
      break;
    }

    case BlockType::SetVolumeTo: {
      float v = b->args.empty() ? 100.0f : toFloat(b->args[0], 100.0f);
      sp->soundVolume = std::clamp(v, 0.0f, 100.0f);
      fr.currentId = b->nextId;
      break;
    }

    case BlockType::ChangeVolumeBy: {
      float dv = b->args.empty() ? 10.0f : toFloat(b->args[0], 10.0f);
      sp->soundVolume = std::clamp(sp->soundVolume + dv, 0.0f, 100.0f);
      fr.currentId = b->nextId;
      break;
    }

    case BlockType::SetPitchTo: {
      float p = b->args.empty() ? 0.0f : toFloat(b->args[0], 0.0f);
      sp->soundPitch = std::clamp(p, -24.0f, 24.0f);
      fr.currentId = b->nextId;
      break;
    }

    case BlockType::ChangePitchBy: {
      float dp = b->args.empty() ? 1.0f : toFloat(b->args[0], 1.0f);
      sp->soundPitch = std::clamp(sp->soundPitch + dp, -24.0f, 24.0f);
      fr.currentId = b->nextId;
      break;
    }

    default:
      fr.currentId = b->nextId;
      break;
  }

  // unwind finished frames / control re-entry
  while (!stack_.empty()) {
    Frame& top = stack_.back();
    if (top.currentId != -1) break;

    int owner = top.ownerControlId;
    int remaining = top.repeatRemaining;
    int mode = top.controlMode;
    int head = top.headId;
    stack_.pop_back();

    if (owner == 0) continue;

    if (mode == 2) {
      // forever
      Frame ctrl;
      ctrl.ownerControlId = owner;
      ctrl.repeatRemaining = -1;
      ctrl.controlMode = 2;
      ctrl.headId = head;
      ctrl.currentId = head;
      stack_.push_back(ctrl);
      break;
    }

    if (mode == 3) {
      // repeat-until: check condition on owner block
      Sprite* spr = curSprite(project);
      if (spr) {
        Block* ownerBlock = project.findBlock(*spr, owner);
        std::string cond = ownerBlock && !ownerBlock->args.empty() ? ownerBlock->args[0] : std::string{};
        bool done = ownerBlock ? evalCondition(project, *spr, cond) : true;
        if (!done) {
          Frame ctrl;
          ctrl.ownerControlId = owner;
          ctrl.repeatRemaining = 0;
          ctrl.controlMode = 3;
          ctrl.headId = head;
          ctrl.currentId = head;
          stack_.push_back(ctrl);
          break;
        }
        // done: advance owner in parent frame
        if (ownerBlock) {
          for (int i = (int)stack_.size() - 1; i >= 0; --i) {
            if (stack_[i].currentId == owner) {
              stack_[i].currentId = ownerBlock->nextId;
              break;
            }
          }
        }
      }
      continue;
    }

    // repeat(count)
    remaining--;
    if (remaining > 0) {
      Frame ctrl;
      ctrl.ownerControlId = owner;
      ctrl.repeatRemaining = remaining;
      ctrl.controlMode = 1;
      ctrl.headId = head;
      ctrl.currentId = head;
      stack_.push_back(ctrl);
      break;
    } else {
      // finished: advance owner block to next
      Sprite* spr = curSprite(project);
      if (spr) {
        Block* ownerBlock = project.findBlock(*spr, owner);
        if (ownerBlock) {
          for (int i = (int)stack_.size() - 1; i >= 0; --i) {
            if (stack_[i].currentId == owner) {
              stack_[i].currentId = ownerBlock->nextId;
              break;
            }
          }
        }
      }
    }
  }

  return true;
}

void ScriptRunner::tick(Project& project,
                       float dt,
                       int maxStepsPerTick,
                       int* outSteps,
                       bool* outHadError,
                       std::string* outErrorMsg) {
  if (outSteps) *outSteps = 0;
  if (outHadError) *outHadError = false;
  if (outErrorMsg) outErrorMsg->clear();

  if (finished_ || paused_) return;

  // update say bubble timer
  if (Sprite* sp = curSprite(project)) {
    if (sp->sayTimeRemaining > 0.0f) {
      sp->sayTimeRemaining -= dt;
      if (sp->sayTimeRemaining <= 0.0f) {
        sp->sayTimeRemaining = 0.0f;
        sp->sayText.clear();
      }
    }
  }

  int steps = 0;
  try {
    while (!finished_ && !paused_ && steps < maxStepsPerTick) {
      if (!stepOnce(project, dt)) break;
      steps++;
    }
  } catch (const std::exception& ex) {
    finished_ = true;
    if (outHadError) *outHadError = true;
    if (outErrorMsg) *outErrorMsg = std::string("Script error: ") + ex.what();
  } catch (...) {
    finished_ = true;
    if (outHadError) *outHadError = true;
    if (outErrorMsg) *outErrorMsg = "Script error: unknown exception";
  }

  if (outSteps) *outSteps = steps;
}
