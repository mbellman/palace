#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

struct Staircase {
  GridCoordinates coordinates;
  // @todo define a StaircaseDirection struct to control the offset
  Gamma::Vec3f offset;
};

struct WorldOrientationChange {
  GridCoordinates coordinates;
  WorldOrientation targetWorldOrientation;
};