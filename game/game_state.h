#pragma once

#include "Gamma.h"

#include "move_queue.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "entities.h"

struct GameState {
  // @todo this is a function of currentMove.startTime, and can be removed
  bool moving = false;
  float lastMoveInputTime = 0.f;
  MoveQueue moves;
  CurrentMove currentMove;
  WorldOrientationState worldOrientationState;
  // @todo define an EntityManager struct for pooling/allocating entities
  Staircase staircases[2];
  WorldOrientationChange worldOrientationChanges[2];
};