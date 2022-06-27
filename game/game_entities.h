#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

enum EntityType {
  GROUND,
  STAIRCASE,
  WORLD_ORIENTATION_CHANGE
};

/**
 * Static entities
 * ---------------
 */
struct StaticEntity {
  EntityType type;

  StaticEntity(EntityType type): type(type) {};
  virtual ~StaticEntity() = default;
};

struct Ground : StaticEntity {
  Ground(): StaticEntity(GROUND) {};
};

struct Staircase : StaticEntity {
  Staircase(): StaticEntity(STAIRCASE) {};

  Staircase(const Staircase* staircase): Staircase() {
    orientation = staircase->orientation;
  }

  Gamma::Orientation orientation;
};

/**
 * Trigger Entities
 * ----------------
 */
struct TriggerEntity {
  EntityType type;

  TriggerEntity(EntityType type): type(type) {};
};

struct WorldOrientationChange : TriggerEntity {
  WorldOrientationChange(): TriggerEntity(WORLD_ORIENTATION_CHANGE) {};

  WorldOrientation targetWorldOrientation;
};

/**
 * Dynamic entities
 * ----------------
 */
struct DynamicEntity {};