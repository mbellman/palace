#pragma once

#include "grid_utilities.h"
#include "orientation_system.h"

enum EntityType {
  GROUND,
  STAIRCASE,
  WORLD_ORIENTATION_CHANGE
};

/**
 * Tile entities
 * -------------
 */
struct TileEntity {
  EntityType type;

  TileEntity(EntityType type): type(type) {};
  virtual ~TileEntity() = default;
};

struct Ground : TileEntity {
  Ground(): TileEntity(GROUND) {};
};

struct Staircase : TileEntity {
  Staircase(): TileEntity(STAIRCASE) {};

  Staircase(const Staircase* entity): Staircase() {
    orientation = entity->orientation;
  }

  Gamma::Orientation orientation;
};

struct WorldOrientationChange : TileEntity {
  WorldOrientationChange(): TileEntity(WORLD_ORIENTATION_CHANGE) {};

  WorldOrientationChange(const WorldOrientationChange* entity): WorldOrientationChange() {
    targetWorldOrientation = entity->targetWorldOrientation;
  }

  WorldOrientation targetWorldOrientation;
};

/**
 * Dynamic entities
 * ----------------
 */
struct DynamicEntity {};