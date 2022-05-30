#pragma once

#include "Gamma.h"

enum MoveDirection {
  NONE,
  Z_FORWARD,
  Z_BACKWARD,
  X_LEFT,
  X_RIGHT,
  Y_UP,
  Y_DOWN
};

struct MoveQueue {
  MoveDirection queue[5];
  Gamma::uint8 size = 0;
  float lastMoveTime = 0;
};

void addMove(MoveQueue& moves, MoveDirection move, float time);
MoveDirection takeNextMove(MoveQueue& moves);