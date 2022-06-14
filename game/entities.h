#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

/**
 * Static entities
 * ---------------
 */
enum StaticEntityType {
  GROUND,
  STAIRCASE_MOVER,
  WORLD_ORIENTATION_CHANGER
};

struct StaticEntity {
  StaticEntityType type;

  StaticEntity(StaticEntityType type): type(type) {};
};

struct Ground : StaticEntity {
  Ground(): StaticEntity(GROUND) {};
};

// @todo refactor as Staircase
struct StaircaseMover : StaticEntity {
  GridCoordinates stepFromCoordinates;
  Gamma::Vec3f movementOffset;

  StaircaseMover(): StaticEntity(STAIRCASE_MOVER) {};
};

struct WorldOrientationChanger : StaticEntity {
  WorldOrientation targetWorldOrientation;

  WorldOrientationChanger(): StaticEntity(WORLD_ORIENTATION_CHANGER) {};
};

/**
 * Dynamic entities
 * ----------------
 */
struct DynamicEntity {};