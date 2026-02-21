#pragma once
#include <string>

namespace FileUtil {
  bool readAllText(const std::string& path, std::string& out);
  bool writeAllText(const std::string& path, const std::string& text);
}