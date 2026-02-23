#pragma once
#include <string>
#include <vector>

enum class BlockType : int {
  // Events
  WhenGreenFlag,
  WhenKeyPressed,

  // Motion (13-16)
  MoveSteps,
  TurnRight,
  TurnLeft,
  GoToXY,

  SetX,
  SetY,
  ChangeXBy,
  ChangeYBy,
  GoToRandomPosition,
  GoToMousePointer,
  IfOnEdgeBounce,
  StopAtEdge,

  // Looks
  Say,
  Think,

  // Sound (24-26) - Pink
  PlaySound,            // 24) play sound (no wait)
  PlaySoundUntilDone,   // 25) play sound until done
  StopAllSounds,        // 25) stop all sounds
  SetVolumeTo,          // 26) set volume to (0..100)
  ChangeVolumeBy,       // 26) change volume by
  SetPitchTo,           // 26) set pitch to (semitones)
  ChangePitchBy,        // 26) change pitch by

  SwitchCostumeTo,
  NextCostume,

  SwitchBackdropTo,
  NextBackdrop,

  SetSizeTo,
  ChangeSizeBy,

  Show,
  Hide,

  GoToFrontLayer,
  GoBackLayers,

  LooksReporter,   // 23) reports (placeholder reporter block)

  // Control
  WaitSeconds,
  WaitUntil,
  Repeat,
  RepeatUntil,
  Forever,
  IfThen,

  // Control: stop/time
  StopThisScript,
  StopAll,

  // Sensing (31-34)
  AskAndWait,   // 33) ask ( ) and wait
};

struct Block {
  int id{0};
  BlockType type{BlockType::MoveSteps};

  // workspace UI position
  float x{0}, y{0};

  // args as strings (so UI can edit)
  std::vector<std::string> args;

  // chain links
  int nextId{-1};
  int childHeadId{-1};
};