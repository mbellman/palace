#pragma once

#include "Gamma.h"

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "world_system.h"
#include "editor_system.h"
#include "game_entities.h"
#include "build_flags.h"

struct GameState {
  // Tracking variables
  float lastMoveInputTime = 0.f;

  // World/orientation/move management
  World world;
  WorldOrientationState worldOrientationState;
  MoveQueue moves;
  CurrentMove currentMove;

  // Entity behavior
  Switch* lastPressedSwitch = nullptr;

  #if DEVELOPMENT == 1
    WorldEditor editor;
    Gamma::Vec3f cameraStartPosition;
    Gamma::Vec3f cameraTargetPosition;
    float cameraTargetStartTime = 0.f;
  #endif
};