#pragma once

#include "Gamma.h"

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "world_system.h"
#include "entities.h"

struct GameState {
  // @todo this is a function of currentMove.startTime, and can be removed
  bool moving = false;
  float lastMoveInputTime = 0.f;
  World world;
  WorldOrientationState worldOrientationState;
  MoveQueue moves;
  CurrentMove currentMove;
};