#pragma once

#include "Gamma.h"

#define MAX_MOVE_QUEUE_SIZE 3

enum MoveDirection {
  NONE,
  Z_FORWARD,
  Z_BACKWARD,
  X_LEFT,
  X_RIGHT,
  Y_UP,
  Y_DOWN
};

enum EasingType {
  EASE_IN_OUT,
  LINEAR,
  EASE_OUT
};

struct MoveQueue {
  MoveDirection queue[MAX_MOVE_QUEUE_SIZE];
  Gamma::uint8 size = 0;
};

void commitMove(MoveQueue& moves, MoveDirection move);
MoveDirection checkNextMove(MoveQueue& moves, Gamma::uint8 movesAhead = 0);
MoveDirection takeNextMove(MoveQueue& moves);