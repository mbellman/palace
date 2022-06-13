#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

/**
 * Static entities
 * ---------------
 */
enum StaticEntityType {
  GROUND,
  STAIRCASE_MOVER
};

struct StaticEntity {
  StaticEntityType type;

  StaticEntity(StaticEntityType type): type(type) {};
};

struct Ground : StaticEntity {
  Ground(): StaticEntity(GROUND) {};
};

// struct StaircaseMover_ : StaticEntity {
//   StaircaseMover_(): StaticEntity(STAIRCASE_MOVER) {};
// };

/**
 * Dynamic entities
 * ----------------
 */
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