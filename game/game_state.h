#pragma once

#include "Gamma.h"

struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;
};

struct SnapToGrid {
  bool active = false;
  float startTime = 0;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState {
  Gamma::Vec3f velocity;
  SnapToGrid snap;
};

#define args() GmContext* context, GameState& state