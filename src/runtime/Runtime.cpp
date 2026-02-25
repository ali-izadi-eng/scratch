#include "runtime/Runtime.h"

#include <algorithm>
#include <chrono>

#include "core/Logger.h"
#include "audio/AudioEngine.h"

static int clampi(int v, int lo, int hi) {
  if (v < lo) return lo;
  if (v > hi) return hi;
  return v;
}

void Runtime::pauseWithError(const std::string& msg) {
  lastError_ = msg;
  Logger::error("Runtime", msg);
  setPaused(true);
}

void Runtime::startGreenFlag(Project& project) {
  // Scratch-like: starting green flag stops any playing sounds.
  AudioEngine::instance().stopAll();

  runners_.clear();
  lastError_.clear();

  running_ = true;
  paused_ = false;

  for (auto& sp : project.sprites()) {
    for (auto& sc : sp.scripts) {
      if (sc.headBlockId == -1) continue;

      auto it = sp.blocks.find(sc.headBlockId);
      if (it == sp.blocks.end()) continue;

      ScriptRunner r;
      r.start(project, sp.id, sc.id);
      if (!r.isFinished()) runners_.push_back(std::move(r));
    }
  }

  Logger::info("Runtime", "Green flag: started " + std::to_string((int)runners_.size()) + " runner(s)");

  if (runners_.empty()) {
    Logger::warn("Runtime", "No runnable scripts found (headBlockId missing or invalid).");
    running_ = false;
    paused_ = false;
  }
}

void Runtime::stopAll() {
  AudioEngine::instance().stopAll();
  for (auto& r : runners_) r.stop();
  runners_.clear();
  running_ = false;
  paused_ = false;
  lastError_.clear();
  Logger::info("Runtime", "Stopped all");
}

void Runtime::setPaused(bool p) {
  paused_ = p;
  for (auto& r : runners_) r.setPaused(p);
}

void Runtime::tick(Project& project, float dt) {
  if (!running_) return;
  if (paused_) return;

  if (project.consumeStopAllScriptsRequest()) {
    stopAll();
    return;
  }

  safety_.maxStepsPerRunnerPerTick = clampi(safety_.maxStepsPerRunnerPerTick, 1, 200000);
  safety_.maxTotalStepsPerTick     = clampi(safety_.maxTotalStepsPerTick,     1, 500000);
  safety_.maxTickMillis            = clampi(safety_.maxTickMillis,            1, 1000);

  const auto t0 = std::chrono::steady_clock::now();
  int totalSteps = 0;

  for (auto& r : runners_) {
    if (r.isFinished()) continue;

    int remaining = safety_.maxTotalStepsPerTick - totalSteps;
    if (remaining <= 0) break;

    int perRunnerBudget = std::min(safety_.maxStepsPerRunnerPerTick, remaining);

    int stepsDone = 0;
    bool hadError = false;
    std::string errMsg;

    r.tick(project, dt, perRunnerBudget, &stepsDone, &hadError, &errMsg);
    totalSteps += stepsDone;

    if (project.consumeStopAllScriptsRequest()) {
      stopAll();
      return;
    }

    if (hadError) {
      pauseWithError(errMsg.empty() ? "Runtime error (unknown)." : errMsg);
      break;
    }

    auto t1 = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
    if (ms > safety_.maxTickMillis) {
      pauseWithError("Safety pause: tick time budget exceeded (" + std::to_string((int)ms) + " ms).");
      break;
    }
  }

  if (!paused_ && totalSteps >= safety_.maxTotalStepsPerTick) {
    pauseWithError("Safety pause: execution budget exceeded (" + std::to_string(totalSteps) + " steps).");
  }

  runners_.erase(std::remove_if(runners_.begin(), runners_.end(),
                [](const ScriptRunner& r){ return r.isFinished(); }),
                runners_.end());

  if (runners_.empty()) running_ = false;
}

void Runtime::step(Project& project) {
  if (!running_) return;

  for (auto& r : runners_) {
    if (r.isFinished()) continue;

    int stepsDone = 0;
    bool hadError = false;
    std::string errMsg;

    r.tick(project, 0.0f, 1, &stepsDone, &hadError, &errMsg);

    if (hadError) {
      pauseWithError(errMsg.empty() ? "Runtime error (unknown)." : errMsg);
    }
    break;
  }

  runners_.erase(std::remove_if(runners_.begin(), runners_.end(),
                [](const ScriptRunner& r){ return r.isFinished(); }),
                runners_.end());

  if (runners_.empty()) running_ = false;
}