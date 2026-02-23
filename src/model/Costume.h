#pragma once
#include <string>

struct Costume {
  std::string name;
  std::string imagePath; // فعلاً فقط مسیر فایل (بعداً texture cache)
  int width{0};
  int height{0};
};