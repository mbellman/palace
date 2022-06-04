#pragma once

#include "Gamma.h"

struct GameState;

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
  float from = 0.f;
  WorldOrientation orientation = POSITIVE_Y_UP;
};

void updateCameraFromMouseMoveEvent(GmContext* context, GameState& state, const Gamma::MouseMoveEvent& event);
void setWorldOrientation(GmContext* context, GameState& state, WorldOrientation orientation);
void handleWorldOrientation(GmContext* context, GameState& state, float dt);