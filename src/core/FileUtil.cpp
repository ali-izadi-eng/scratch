#include "FileUtil.h"
#include <fstream>
#include <sstream>

namespace FileUtil {

bool readAllText(const std::string& path, std::string& out) {
  std::ifstream f(path, std::ios::in | std::ios::binary);
  if (!f) return false;
  std::ostringstream ss;
  ss << f.rdbuf();
  out = ss.str();
  return true;
}

bool writeAllText(const std::string& path, const std::string& text) {
  std::ofstream f(path, std::ios::out | std::ios::binary);
  if (!f) return false;
  f.write(text.data(), (std::streamsize)text.size());
  return true;
}

}