#pragma once

#include "Gamma.h"

#include "easing_utilities.h"
#include "game_macros.h"

struct CurrentMove {
  float startTime = 0.f;
  float duration = 0.f;
  // @todo rename to MoveType (initial, continuous, stopping)
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState;

void handlePlayerCameraMovementOnUpdate(Globals, float dt);