#pragma once
#include <string>
#include <vector>

#include "core/Project.h"
#include "runtime/ScriptRunner.h"

class Runtime {
public:
  void startGreenFlag(Project& project);
  void stopAll();

  void setPaused(bool p);
  bool isPaused() const { return paused_; }
  bool isRunning() const { return running_ && !runners_.empty(); }

  // main update loop
  void tick(Project& project, float dt);

  // step once (debug)
  void step(Project& project);

  // --- safety / diagnostics ---
  const std::string& lastError() const { return lastError_; }
  void clearError() { lastError_.clear(); }

  struct SafetyConfig {
    int maxStepsPerRunnerPerTick = 200;   // per runner budget
    int maxTotalStepsPerTick     = 5000;  // global budget
    int maxTickMillis            = 8;     // time budget (ms)
  };
  SafetyConfig& safety() { return safety_; }
  const SafetyConfig& safety() const { return safety_; }

private:
  void pauseWithError(const std::string& msg);

private:
  std::vector<ScriptRunner> runners_;
  bool running_{false};
  bool paused_{false};

  SafetyConfig safety_{};
  std::string lastError_;
};