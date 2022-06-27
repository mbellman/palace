#include <vector>
#include <map>

#include "object_system.h"
#include "game_entities.h"
#include "grid_utilities.h"

#define scale(...) Vec3f(__VA_ARGS__)
#define color(...) Vec3f(__VA_ARGS__)

using namespace Gamma;

const static std::map<EntityType, ObjectParameters> entityToObjectParametersMap = {
  {GROUND, {
    scale(HALF_TILE_SIZE * 0.99f),
    color(1.f, 0.7f, 0.3f)
  }},
  {STAIRCASE, {
    scale(HALF_TILE_SIZE),
    color(0.5f)
  }},
  {WORLD_ORIENTATION_CHANGE, {
    scale(TILE_SIZE * 0.15f),
    color(1.f, 0, 0)
  }}
};

const ObjectParameters& getObjectParameters(EntityType entityType) {
  return entityToObjectParametersMap.at(entityType);
}