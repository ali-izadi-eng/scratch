#include "app/App.h"


#include <stdexcept>
#include "imgui.h"
#include "backends/imgui_impl_sdl2.h"
#include "backends/imgui_impl_sdlrenderer2.h"
#include <SDL.h>

#include "core/Logger.h"
#include "core/Time.h"
#include "core/Watchdog.h"
#include "ui/MainDockspace.h"
#include "renderer/TextureCache.h"
#include "renderer/Renderer2D.h"
#include <SDL_image.h>

static constexpr const char* kAppName = "Scratchy";

App::App() {
  if (!initSDL()) throw std::runtime_error("SDL init failed");
  if (!initImGui()) throw std::runtime_error("ImGui init failed");

  Logger::init();
  Logger::info("App", "Initialized");
}

App::~App() {
  Logger::info("App", "Shutting down");
  Logger::shutdown();

  shutdownImGui();
  shutdownSDL();
}

bool App::initSDL() {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
    return false;
  }

  // init SDL_image (PNG/JPG)
  int imgFlags = IMG_INIT_PNG | IMG_INIT_JPG;
  if ((IMG_Init(imgFlags) & imgFlags) != imgFlags) {
    // اگر JPG یا PNG یکی‌شون لود نشد هم باز می‌تونی ادامه بدی، ولی بهتره fail کنی:
    // Logger هنوز init نشده، پس فقط false برگردون
    return false;
  }

  window_ = SDL_CreateWindow(
      kAppName,
      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      winW_, winH_,
      SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);

  if (!window_) return false;

  renderer_ = SDL_CreateRenderer(
      window_, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!renderer_) return false;

  // needed for SDL_DROPFILE on some platforms
  SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

  return true;
}

bool App::initImGui() {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO();
  (void)io;

  ImGui::StyleColorsDark();

  if (!ImGui_ImplSDL2_InitForSDLRenderer(window_, renderer_)) return false;
  if (!ImGui_ImplSDLRenderer2_Init(renderer_)) return false;

  return true;
}

void App::shutdownImGui() {
  ImGui_ImplSDLRenderer2_Shutdown();
  ImGui_ImplSDL2_Shutdown();
  ImGui::DestroyContext();
}

void App::shutdownSDL() {
  if (renderer_) SDL_DestroyRenderer(renderer_);
  if (window_) SDL_DestroyWindow(window_);
  IMG_Quit();
  SDL_Quit();
}

void App::handleEvent(const SDL_Event& e) {
  ImGui_ImplSDL2_ProcessEvent(&e);

  if (e.type == SDL_QUIT) running_ = false;

  if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
    running_ = false;
  }

  // --- input for sensing ---
  if (e.type == SDL_KEYDOWN && !e.key.repeat) {
    project_.setKeyDown(e.key.keysym.scancode, true);
  }
  if (e.type == SDL_KEYUP) {
    project_.setKeyDown(e.key.keysym.scancode, false);
  }
  if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
    project_.setMouseButtonDown(true);
  }
  if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
    project_.setMouseButtonDown(false);
  }

  // drag & drop file (Costumes/Sounds)
  if (e.type == SDL_DROPFILE) {
    if (e.drop.file) {
      project_.setDroppedFile(std::string(e.drop.file));
      SDL_free(e.drop.file);
    }
  }
}

void App::beginFrame() {
  ImGui_ImplSDLRenderer2_NewFrame();
  ImGui_ImplSDL2_NewFrame();
  ImGui::NewFrame();
}

void App::endFrame() {
  ImGui::Render();
}

int App::run() {
  MainDockspace ui;
  TextureCache texCache(renderer_);
  Renderer2D renderer2d(renderer_, &texCache);
  Watchdog watchdog;

  uint64_t prevCounter = SDL_GetPerformanceCounter();
  double freq = (double)SDL_GetPerformanceFrequency();

  project_.resetToDefault();

  while (running_) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) handleEvent(e);

    uint64_t now = SDL_GetPerformanceCounter();
    float deltaSeconds = (float)((now - prevCounter) / freq);
    prevCounter = now;
    if (deltaSeconds > 0.05f) deltaSeconds = 0.05f;

    Time::tick();

    // update runtime before drawing
    runtime_.tick(project_, deltaSeconds);

    // Clear BEFORE ImGui draws so stage (SDL) can draw under UI
    SDL_SetRenderDrawColor(renderer_, 20, 20, 20, 255);
    SDL_RenderClear(renderer_);

    beginFrame();

    ui.draw(project_, renderer2d, runtime_);

    if (ui.wantsExit()) {
      SDL_Event quit{};
      quit.type = SDL_QUIT;
      SDL_PushEvent(&quit);
    }

    watchdog.onFrameEnd();

    endFrame();

    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
  }

  return 0;
}
