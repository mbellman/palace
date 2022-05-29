#pragma once

#include "Gamma.h"

struct GridCoordinates {
  int x;
  int y;
  int z;
};

struct GameState {
  bool isMoving = false;
  float moveStartTime = 0;
  // @todo replace with lastCoordinates/targetCoordinates
  Gamma::Vec3f lastPosition;
  Gamma::Vec3f targetPosition;
};

#define args() GmContext* context, GameState& state