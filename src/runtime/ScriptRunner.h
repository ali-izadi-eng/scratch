#pragma once
#include <string>
#include <vector>

#include "core/Project.h"

struct Frame {
  int headId{-1};         // start of this frame chain
  int currentId{-1};      // current executing block in chain
  int ownerControlId{0};  // which control block created this frame
  int repeatRemaining{0}; // for Repeat
  int controlMode{0};     // 0 none, 1 repeat(count), 2 forever, 3 repeat-until
};

class ScriptRunner {
public:
  ScriptRunner() = default;

  void start(Project& project, int spriteId, int scriptId);
  void stop();

  bool isFinished() const { return finished_; }
  bool isPaused() const { return paused_; }
  void setPaused(bool p) { paused_ = p; }

  int spriteId() const { return spriteId_; }
  int scriptId() const { return scriptId_; }

  void tick(Project& project,
            float dt,
            int maxStepsPerTick,
            int* outSteps,
            bool* outHadError,
            std::string* outErrorMsg);

private:
  bool stepOnce(Project& project, float dt);
  Sprite* curSprite(Project& project);
  Block* curBlock(Project& project);

  static float toFloat(const std::string& s, float def);

  // --- Sensing / conditions (simple expressions) ---
  bool evalCondition(Project& project, Sprite& sp, const std::string& expr) const;
  bool touching(Project& project, const Sprite& sp, const std::string& target) const;
  float distanceTo(Project& project, const Sprite& sp, const std::string& target) const;

  static std::string trim(std::string s);
  static std::string lower(std::string s);

  static bool parseDistanceComparison(const std::string& expr, std::string* outTarget, std::string* outOp, float* outValue);
  static bool parseMouseComparison(const std::string& expr, char axis, std::string* outOp, float* outValue);
  static bool cmp(float a, const std::string& op, float b);

private:
  int spriteId_{0};
  int scriptId_{0};
  bool finished_{true};
  bool paused_{false};

  std::vector<Frame> stack_;

  float waitRemaining_{0.0f};

  // ask-and-wait state
  bool waitingAsk_{false};
};
