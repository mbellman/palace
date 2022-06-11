#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

struct StaircaseMover {
  GridCoordinates stepFromCoordinates;
  GridCoordinates coordinates;
  // @todo change to MoveDirection
  Gamma::Vec3f offset;
};

struct WorldOrientationChange {
  GridCoordinates coordinates;
  WorldOrientation targetWorldOrientation;
};