#pragma once
#include <SDL.h>
#include "core/Project.h"
#include "renderer/TextureCache.h"

class Renderer2D {
public:
  Renderer2D(SDL_Renderer* r, TextureCache* cache);

  // stageRect: جایی که stage در UI نمایش داده می‌شود (مختصات صفحه SDL)
  void drawStage(const Project& project, const SDL_FRect& stageRect);

private:
  SDL_Renderer* r_{nullptr};
  TextureCache* cache_{nullptr};

  void drawBackdrop(const Stage& stage, const SDL_FRect& rect);
  void drawSprite(const Sprite& sp, const SDL_FRect& rect);
};