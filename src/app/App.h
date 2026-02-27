#pragma once
#include <SDL.h>

#include "core/Project.h"
#include "runtime/Runtime.h"

class App {
public:
  App();
  ~App();

  int run();

private:
  bool initSDL();
  bool initImGui();
  void shutdownImGui();
  void shutdownSDL();

  void handleEvent(const SDL_Event& e);
  void beginFrame();
  void endFrame();
  void renderFrame();

private:
  SDL_Window* window_{nullptr};
  SDL_Renderer* renderer_{nullptr};
  bool running_{true};

  // state
  Project project_;
  Runtime runtime_;

  // settings
  int winW_{1280};
  int winH_{720};
};