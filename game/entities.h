#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

struct Staircase {
  GridCoordinates coordinates;
  WorldOrientation orientation;
};

struct WorldOrientationChange {
  GridCoordinates coordinates;
  WorldOrientation target;
};