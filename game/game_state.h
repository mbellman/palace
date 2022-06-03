#pragma once

#include "Gamma.h"

#include "move_queue.h"

struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;
};

enum PlayerOrientation {
  POSITIVE_Y_UP,
  NEGATIVE_Y_UP,
  POSITIVE_X_UP,
  NEGATIVE_X_UP,
  POSITIVE_Z_UP,
  NEGATIVE_Z_UP
};

struct CurrentMove {
  float startTime = 0.f;
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct PlayerOrientationState {
  float startTime = 0.f;
  PlayerOrientation previous;
  PlayerOrientation current = POSITIVE_Y_UP;
  float from = 0.f;
};

struct GameState {
  // @todo this is a function of currentMove.startTime, and can be removed
  bool moving = false;
  float lastMoveInputTime = 0.f;
  MoveQueue moves;
  CurrentMove currentMove;
  PlayerOrientationState orientation;
};