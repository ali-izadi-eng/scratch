#pragma once
#include <string>
#include <vector>
#include "Costume.h"
#include "Sound.h"
#include "Script.h"
#include "model/Block.h"
#include "model/Script.h"
#include <unordered_map>

struct Sprite {
  int id{0};
  std::string name{"Sprite"};

  // Transform
  float x{0}, y{0};          // مختصات روی صحنه (بعداً نگاشت به stage coords)
  float directionDeg{90};    // Scratch-like default
  float sizePercent{100};    // پیش‌فرض 100% :contentReference[oaicite:3]{index=3}
  bool visible{true};

  // Looks
  int currentCostume{0};
  std::vector<Costume> costumes;

  // Sound
  std::vector<Sound> sounds;

  // Sound effects (Scratch-like)
  float soundVolume{100.0f}; // 0..100
  float soundPitch{0.0f};    // semitones (rough)

  // Graphics effects (placeholder)
  float colorEffect{0};

  const Costume* costume() const {
    if (costumes.empty()) return nullptr;
    int idx = currentCostume;
    if (idx < 0 || idx >= (int)costumes.size()) idx = 0;
    return &costumes[idx];
  }

  // Scripts + Blocks (visual code)
  std::vector<Script> scripts;
  std::unordered_map<int, Block> blocks; // blockId -> Block

  // editor selection
  int selectedScriptId{0};

  std::string sayText{};
  float sayTimeRemaining{0.0f};
};