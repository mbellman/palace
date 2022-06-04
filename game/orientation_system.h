#pragma once

#include "Gamma.h"

enum WorldOrientation {
  POSITIVE_Y_UP,
  NEGATIVE_Y_UP,
  POSITIVE_X_UP,
  NEGATIVE_X_UP,
  POSITIVE_Z_UP,
  NEGATIVE_Z_UP
};

struct WorldOrientationState {
  float startTime = 0.f;
  float from_deprecated = 0.f;  // @todo remove
  Gamma::Orientation from;
  Gamma::Orientation to;
  WorldOrientation orientation = POSITIVE_Y_UP;
};

struct GameState;

void updateCameraFromMouseMoveEvent(GmContext* context, GameState& state, const Gamma::MouseMoveEvent& event);
void setWorldOrientation(GmContext* context, GameState& state, WorldOrientation orientation);
void handleWorldOrientation(GmContext* context, GameState& state, float dt);