#pragma once

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "world_system.h"
#include "game_entities.h"

struct GameState {
  float lastMoveInputTime = 0.f;
  World world;
  WorldOrientationState worldOrientationState;
  MoveQueue moves;
  CurrentMove currentMove;

  #if GAMMA_DEVELOPER_MODE == 1
    GridCoordinates lastGridCoordinates;
    bool freeCameraMode = false;
  #endif
};