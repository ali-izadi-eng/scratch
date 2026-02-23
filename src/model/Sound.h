#pragma once
#include <string>

struct Sound {
  std::string name;
  std::string filePath;
  float volume{1.0f};     // 0..1
  bool muted{false};
};