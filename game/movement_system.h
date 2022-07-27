#pragma once

#include "Gamma.h"

#include "easing_utilities.h"
#include "game_macros.h"

struct CurrentMove {
  float startTime = 0.f;
  float duration = 0.f;
  EasingType easing;
  Gamma::Vec3f from;
  Gamma::Vec3f to;
};

struct GameState;

void handlePlayerCameraMovement(Globals, float dt);