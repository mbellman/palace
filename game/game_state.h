#pragma once

#include "Gamma.h"

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "world_system.h"
#include "entities.h"

struct GameState {
  float lastMoveInputTime = 0.f;
  World world;
  WorldOrientationState worldOrientationState;
  MoveQueue moves;
  CurrentMove currentMove;
};