#pragma once
#include <string>

class Project;

namespace Serialization {
  bool saveToFile(const Project& project, const std::string& path, std::string* err = nullptr);
  bool loadFromFile(Project& project, const std::string& path, std::string* err = nullptr);
}