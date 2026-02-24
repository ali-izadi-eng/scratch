#include "Renderer2D.h"
#include "core/Logger.h"
#include <algorithm>
#include <cmath>

Renderer2D::Renderer2D(SDL_Renderer* r, TextureCache* cache) : r_(r), cache_(cache) {}

static SDL_FPoint stageToScreen(float x, float y, const SDL_FRect& stageRect) {
  // Scratch stage: معمولا 480x360 و مرکز (0,0)
  // ما فعلاً نگاشت ساده می‌زنیم: x in [-240..240], y in [-180..180]
  float sx = stageRect.x + stageRect.w * ( (x + 240.0f) / 480.0f );
  float sy = stageRect.y + stageRect.h * ( (180.0f - y) / 360.0f ); // y رو برعکس می‌کنیم
  return SDL_FPoint{sx, sy};
}

void Renderer2D::drawBackdrop(const Stage& stage, const SDL_FRect& rect) {
  // background
  SDL_SetRenderDrawColor(r_, 40, 40, 50, 255);
  SDL_RenderFillRectF(r_, &rect);

  const Costume* bd = stage.backdrop();
  if (!bd) return;

  SDL_Texture* tex = cache_ ? cache_->get(bd->imagePath) : nullptr;
  if (!tex) return;

  // cover fit
  SDL_FRect dst = rect;
  SDL_RenderCopyF(r_, tex, nullptr, &dst);
}

void Renderer2D::drawSprite(const Sprite& sp, const SDL_FRect& stageRect) {
  if (!sp.visible) return;

  SDL_FPoint p = stageToScreen(sp.x, sp.y, stageRect);

  // placeholder size
  float size = 30.0f * (sp.sizePercent / 100.0f);
  size = std::clamp(size, 5.0f, 200.0f);

  const Costume* c = sp.costume();
  SDL_Texture* tex = (c && cache_) ? cache_->get(c->imagePath) : nullptr;

  SDL_FRect dst{ p.x - size * 0.5f, p.y - size * 0.5f, size, size };

  if (tex) {
    // directionDeg: 90 یعنی راست، SDL angle: درجه و جهت ساعتگرد
    double angle = -(sp.directionDeg - 90.0); // تقریب
    SDL_RenderCopyExF(r_, tex, nullptr, &dst, angle, nullptr, SDL_FLIP_NONE);
  } else {
    // placeholder rect
    SDL_SetRenderDrawColor(r_, 220, 220, 60, 255);
    SDL_RenderFillRectF(r_, &dst);
    SDL_SetRenderDrawColor(r_, 20, 20, 20, 255);
    SDL_RenderDrawRectF(r_, &dst);
  }
}

void Renderer2D::drawStage(const Project& project, const SDL_FRect& stageRect) {
  drawBackdrop(project.stage(), stageRect);

  // stage border
  SDL_SetRenderDrawColor(r_, 200, 200, 200, 255);
  SDL_RenderDrawRectF(r_, &stageRect);

  for (const auto& sp : project.sprites()) {
    drawSprite(sp, stageRect);
  }
}