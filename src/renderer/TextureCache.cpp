#include "renderer/TextureCache.h"

#include <SDL_image.h>
#include "core/Logger.h"

TextureCache::~TextureCache() {
  clear();
}

void TextureCache::clear() {
  for (auto& kv : cache_) {
    if (kv.second) SDL_DestroyTexture(kv.second);
  }
  cache_.clear();
}

void TextureCache::invalidate(const std::string& path) {
  auto it = cache_.find(path);
  if (it == cache_.end()) return;
  if (it->second) SDL_DestroyTexture(it->second);
  cache_.erase(it);
}

SDL_Texture* TextureCache::get(const std::string& path) {
  if (!r_ || path.empty()) return nullptr;

  auto it = cache_.find(path);
  if (it != cache_.end()) return it->second;

  // ✅ PNG/JPG/BMP/... via SDL_image
  SDL_Texture* tex = IMG_LoadTexture(r_, path.c_str());
  if (!tex) {
    Logger::error("TextureCache",
                  std::string("IMG_LoadTexture failed for '") + path +
                  "': " + IMG_GetError());
    cache_[path] = nullptr;
    return nullptr;
  }

  cache_[path] = tex;
  return tex;
}