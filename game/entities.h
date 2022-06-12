#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

enum StaticEntityType {
  STAIRCASE_MOVER
};

struct StaticEntity {
  StaticEntityType type;
  GridCoordinates coordinates;
};

struct DynamicEntity {};

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