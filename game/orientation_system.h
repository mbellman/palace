#pragma once

#include "Gamma.h"

struct GameState;

enum PlayerOrientation {
  POSITIVE_Y_UP,
  NEGATIVE_Y_UP,
  POSITIVE_X_UP,
  NEGATIVE_X_UP,
  POSITIVE_Z_UP,
  NEGATIVE_Z_UP
};

struct PlayerOrientationState {
  float startTime = 0.f;
  float from = 0.f;
  PlayerOrientation orientation = POSITIVE_Y_UP;
};

void updateCameraFromMouseMoveEvent(GmContext* context, GameState& state, const Gamma::MouseMoveEvent& event);
void setPlayerOrientation(GmContext* context, GameState& state, PlayerOrientation orientation);
void handlePlayerOrientation(GmContext* context, GameState& state, float dt);