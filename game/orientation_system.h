#pragma once

#include "Gamma.h"

#include "grid_utilities.h"
#include "game_macros.h"

enum WorldOrientation {
  POSITIVE_Y_UP,
  NEGATIVE_Y_UP,
  POSITIVE_X_UP,
  NEGATIVE_X_UP,
  POSITIVE_Z_UP,
  NEGATIVE_Z_UP
};

struct WorldOrientationState {
  float startTime = 0.f;               // @todo move to state.cameraState
  Gamma::Orientation orientationFrom;  // @todo move to state.cameraState
  Gamma::Orientation orientationTo;    // @todo move to state.cameraState
  Gamma::Vec3f movementPlane = Gamma::Vec3f(1.f, 0, 1.f);
  WorldOrientation worldOrientation = POSITIVE_Y_UP;
};

struct GameState;

void updateCameraFromMouseMoveEvent(Globals, const Gamma::MouseMoveEvent& event);
void setWorldOrientation(Globals, WorldOrientation worldOrientation);
void immediatelySetWorldOrientation(Globals, WorldOrientation worldOrientation);
void handleWorldOrientation(Globals, float dt);

GridCoordinates getUpGridCoordinates(WorldOrientation worldOrientation);
GridCoordinates getDownGridCoordinates(WorldOrientation worldOrientation);