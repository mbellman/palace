#pragma once

#include "Gamma.h"

#include "easing_utilities.h"

struct CurrentMove {
  float startTime = 0.f;
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState;

void handlePlayerMovement(GmContext* context, GameState& state, float dt);