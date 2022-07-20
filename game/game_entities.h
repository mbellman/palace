#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

enum EntityType {
  GROUND,
  STAIRCASE,
  SWITCH,
  WORLD_ORIENTATION_CHANGE,
  TELEPORTER
};

/**
 * Grid entities
 * -------------
 */
struct GridEntity {
  EntityType type;

  GridEntity(EntityType type): type(type) {};
  virtual ~GridEntity() = default;
};

struct Ground : GridEntity {
  Ground(): GridEntity(GROUND) {};
};

struct Staircase : GridEntity {
  Staircase(): GridEntity(STAIRCASE) {};

  Staircase(const Staircase* entity): Staircase() {
    orientation = entity->orientation;
  }

  Gamma::Orientation orientation;
};

// @todo support switches being permanently pressed down
struct Switch : GridEntity {
  Switch(): GridEntity(SWITCH) {};

  float pressedDuration = 0.f;
};

struct WorldOrientationChange : GridEntity {
  WorldOrientationChange(): GridEntity(WORLD_ORIENTATION_CHANGE) {};

  WorldOrientationChange(const WorldOrientationChange* entity): WorldOrientationChange() {
    targetWorldOrientation = entity->targetWorldOrientation;
  }

  WorldOrientation targetWorldOrientation;
};

struct Teleporter : GridEntity {
  Teleporter(): GridEntity(TELEPORTER) {};

  Teleporter(const Teleporter* entity): Teleporter() {
    toCoordinates = entity->toCoordinates;
    toOrientation = entity->toOrientation;
  }

  GridCoordinates toCoordinates;
  WorldOrientationChange toOrientation;
};

/**
 * Dynamic entities
 * ----------------
 */
struct DynamicEntity {};