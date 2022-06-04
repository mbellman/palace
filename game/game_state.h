#pragma once

#include "Gamma.h"

#include "move_queue.h"
#include "orientation_system.h"

struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct CurrentMove {
  float startTime = 0.f;
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState {
  // @todo this is a function of currentMove.startTime, and can be removed
  bool moving = false;
  float lastMoveInputTime = 0.f;
  MoveQueue moves;
  CurrentMove currentMove;
  PlayerOrientationState orientationState;
};