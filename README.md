# Scratchy IDE

A lightweight C++ creative coding environment built with **SDL2** and
**Dear ImGui**.\
Designed as a block-based / sprite-oriented visual programming tool
inspired by Scratch, but implemented in native C++ for performance and
extensibility.

------------------------------------------------------------------------

## ✨ Features

### Core System

-   Create new project
-   Save project
-   Load project
-   Logging system
-   Step-by-step execution
-   Error handling & safety system

### 🟢 Extensions

-   Add Extension
-   Erase All
-   Audio playback
-   Stop all sounds
-   Volume & pitch control

### 🟠 Control Blocks

-   Preprocessing
-   Loops
-   Conditions
-   Time delay control

### 🔵 Sensors

-   Collision detection
-   Distance detection
-   Ask & Answer system
-   Keyboard & Mouse input detection

### 🟣 Looks

-   Say / Think
-   Costume controls
-   Background controls
-   Size control
-   Show / Hide
-   Layer management
-   Looks reports

------------------------------------------------------------------------

## 🏗 Architecture

.
├── app
│   ├── App.cpp
│   └── App.h
├── audio
│   ├── AudioEngine.cpp
│   └── AudioEngine.h
├── core
│   ├── FileUtil.cpp
│   ├── FileUtil.h
│   ├── IdGen.cpp
│   ├── IdGen.h
│   ├── Logger.cpp
│   ├── Logger.h
│   ├── Project.cpp
│   ├── Project.h
│   ├── Serialization.cpp
│   ├── Serialization.h
│   ├── Time.cpp
│   ├── Time.h
│   ├── Watchdog.cpp
│   └── Watchdog.h
├── extensions
│   ├── PenSystem.cpp
│   └── PenSystem.h
├── main.cpp
├── model
│   ├── Block.cpp
│   ├── Block.h
│   ├── Costume.cpp
│   ├── Costume.h
│   ├── Script.cpp
│   ├── Script.h
│   ├── Sound.cpp
│   ├── Sound.h
│   ├── Sprite.cpp
│   ├── Sprite.h
│   ├── Stage.cpp
│   └── Stage.h
├── renderer
│   ├── Renderer2D.cpp
│   ├── Renderer2D.h
│   ├── TextureCache.cpp
│   └── TextureCache.h
├── runtime
│   ├── Runtime.cpp
│   ├── Runtime.h
│   ├── ScriptRunner.cpp
│   └── ScriptRunner.h
└── ui
    ├── EditorState.h
    ├── MainDockspace.cpp
    ├── MainDockspace.h
    └── Panels
        ├── BlocksPalettePanel.cpp
        ├── BlocksPalettePanel.h
        ├── CostumesPanel.cpp
        ├── CostumesPanel.h
        ├── ExtensionsPanel.cpp
        ├── ExtensionsPanel.h
        ├── HelpPanel.cpp
        ├── HelpPanel.h
        ├── InspectorPanel.cpp
        ├── InspectorPanel.h
        ├── ScriptWorkspacePanel.cpp
        ├── ScriptWorkspacePanel.h
        ├── SettingsPanel.cpp
        ├── SettingsPanel.h
        ├── SoundsPanel.cpp
        ├── SoundsPanel.h
        ├── SpritePanel.cpp
        ├── SpritePanel.h
        ├── StagePanel.cpp
        └── StagePanel.h

10 directories, 64 files


------------------------------------------------------------------------

## 🔧 Dependencies

-   C++20
-   SDL2
-   Dear ImGui
-   CMake \>= 3.16

------------------------------------------------------------------------

## 🚀 Build (Linux)

cmake -S . -B build -DCMAKE_BUILD_TYPE=Release cmake --build build -j

Run: ./build/Scratchy

------------------------------------------------------------------------

## 🚀 Build (Windows)

cmake -S . -B build -G "Visual Studio 17 2022" cmake --build build
--config Release

------------------------------------------------------------------------

## 📦 Packaging

cmake --install build --config Release --prefix dist/Scratchy cpack
--config build/CPackConfig.cmake

------------------------------------------------------------------------

## 🎯 Design Goals

-   Modular block system
-   Sprite-based execution engine
-   Real-time rendering
-   Cross-platform (Linux / Windows)
-   Expandable plugin architecture

------------------------------------------------------------------------

## 📜 License

Sharif university on Technology

------------------------------------------------------------------------

## 👤 Author

👑 mohammad hossein abbasi aghbelagh
👤 Ali izadi
👤 sepehr gol niay
