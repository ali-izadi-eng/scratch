#pragma once
#include <SDL.h>
#include <string>
#include <unordered_map>

class TextureCache {
public:
  explicit TextureCache(SDL_Renderer* r) : r_(r) {}
  ~TextureCache();

  SDL_Texture* get(const std::string& path);
  void clear();
  void invalidate(const std::string& path);

private:
  SDL_Renderer* r_{nullptr};
  std::unordered_map<std::string, SDL_Texture*> cache_;
};