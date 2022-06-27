#pragma once

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "world_system.h"
#include "editor_system.h"
#include "game_entities.h"
#include "build_flags.h"

struct GameState {
  float lastMoveInputTime = 0.f;
  World world;
  WorldOrientationState worldOrientationState;
  MoveQueue moves;
  CurrentMove currentMove;

  #if DEVELOPMENT == 1
    WorldEditor editor;
    Gamma::Vec3f cameraStartPosition;
    Gamma::Vec3f cameraTargetPosition;
    float cameraTargetStartTime = 0.f;
  #endif
};