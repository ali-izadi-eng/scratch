cmake_minimum_required(VERSION 3.16)
project(Scratchy LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---- SDL2 (Linux: module mode, Windows: can still work with vcpkg/SDL2Config) ----
find_package(SDL2 REQUIRED)

# ---- ImGui (vendored) ----
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)

add_library(imgui STATIC
  ${IMGUI_DIR}/imgui.cpp
  ${IMGUI_DIR}/imgui_demo.cpp
  ${IMGUI_DIR}/imgui_draw.cpp
  ${IMGUI_DIR}/imgui_tables.cpp
  ${IMGUI_DIR}/imgui_widgets.cpp

  ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
  ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(imgui PUBLIC
  ${IMGUI_DIR}
  ${IMGUI_DIR}/backends
  ${SDL2_INCLUDE_DIRS}
)


target_link_libraries(imgui PUBLIC ${SDL2_LIBRARIES})

# ---- App ----
add_executable(Scratchy
  src/main.cpp
  src/app/App.h
  src/app/App.cpp
  src/core/Logger.h
  src/core/Logger.cpp
  src/core/Time.h
  src/core/Time.cpp
  src/core/Watchdog.h
  src/core/Watchdog.cpp
  src/ui/MainDockspace.h
  src/ui/MainDockspace.cpp
  src/core/Project.h
  src/core/Project.cpp
  src/core/Serialization.h
  src/core/Serialization.cpp
  src/core/FileUtil.h
  src/core/FileUtil.cpp
  src/model/Stage.h
  src/model/Stage.cpp
  src/model/Sprite.h
  src/model/Sprite.cpp
  src/model/Costume.h
  src/model/Costume.cpp
  src/model/Sound.h
  src/model/Sound.cpp
  src/renderer/TextureCache.h
  src/renderer/TextureCache.cpp
  src/renderer/Renderer2D.h
  src/renderer/Renderer2D.cpp
  src/ui/Panels/StagePanel.h
  src/ui/Panels/StagePanel.cpp
  src/ui/Panels/SpritePanel.h
  src/ui/Panels/SpritePanel.cpp
  src/ui/Panels/InspectorPanel.h
  src/ui/Panels/InspectorPanel.cpp
  src/ui/Panels/BlocksPalettePanel.h
  src/ui/Panels/BlocksPalettePanel.cpp
  src/ui/Panels/ScriptWorkspacePanel.h
  src/ui/Panels/ScriptWorkspacePanel.cpp
  src/ui/Panels/CostumesPanel.h
  src/ui/Panels/CostumesPanel.cpp
  src/ui/Panels/SoundsPanel.h
  src/ui/Panels/SoundsPanel.cpp
  src/ui/Panels/SettingsPanel.h
  src/ui/Panels/SettingsPanel.cpp
  src/ui/Panels/ExtensionsPanel.h
  src/ui/Panels/ExtensionsPanel.cpp
  src/ui/Panels/HelpPanel.h
  src/ui/Panels/HelpPanel.cpp
)

target_include_directories(Scratchy PRIVATE src ${SDL2_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/external/nlohmann)
target_link_libraries(Scratchy PRIVATE imgui ${SDL2_LIBRARIES})

# Linux: some distros need pthread/dl for SDL2; usually handled, but safe:
if(UNIX AND NOT APPLE)
  target_link_libraries(Scratchy PRIVATE pthread dl)
endif()