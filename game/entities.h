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

struct StaircaseMover : StaticEntity {
  StaircaseMover(): StaticEntity(STAIRCASE_MOVER) {};
};

/**
 * Trigger Entities
 * ----------------
 */
enum TriggerEntityType {
  WORLD_ORIENTATION_CHANGE
};

struct TriggerEntity {
  TriggerEntityType type;

  TriggerEntity(TriggerEntityType type): type(type) {};
};

struct WorldOrientationChange : TriggerEntity {
  WorldOrientation targetWorldOrientation;

  WorldOrientationChange(): TriggerEntity(WORLD_ORIENTATION_CHANGE) {};
};

/**
 * Dynamic entities
 * ----------------
 */
struct DynamicEntity {};