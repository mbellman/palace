#pragma once

#include "Gamma.h"

#include "game_entities.h"

struct ObjectParameters {
  Gamma::Vec3f scale;
  Gamma::Vec3f color;
};

const ObjectParameters& getObjectParameters(EntityType entityType);