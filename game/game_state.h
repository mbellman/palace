#pragma once

#include "Gamma.h"

#include "move_queue.h"

struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct CurrentMove {
  float startTime = 0;
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState {
  bool moving = false;
  float lastMoveInputTime = 0;
  MoveQueue moves;
  CurrentMove currentMove;
};