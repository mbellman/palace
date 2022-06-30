#include <vector>
#include <map>

#include "object_system.h"
#include "game_entities.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

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
  {SWITCH, {
    scale(HALF_TILE_SIZE),
    color(1.f, 0, 0)
  }},
  {WORLD_ORIENTATION_CHANGE, {
    scale(TILE_SIZE * 0.075f),
    color(1.f, 0, 0)
  }}
};

static void createGroundObject(Globals, const GridCoordinates& coordinates) {
  auto& object = createObjectFrom("ground");
  auto& params = getObjectParameters(GROUND);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.scale = params.scale;
  object.color = params.color;

  commit(object);
}

static void createStaircaseObject(Globals, const GridCoordinates& coordinates, const Orientation& orientation) {
  auto& object = createObjectFrom("staircase");
  auto& params = getObjectParameters(STAIRCASE);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;
  object.rotation.x = orientation.pitch;
  object.rotation.y = orientation.yaw;
  object.rotation.z = -orientation.roll;

  commit(object);
}

static void createSwitchObject(Globals, const GridCoordinates& coordinates) {
  auto& object = createObjectFrom("switch");
  auto& params = getObjectParameters(SWITCH);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;

  commit(object);
}

static void createWorldOrientationChangeObject(Globals, const GridCoordinates& coordinates, WorldOrientation worldOrientation) {
  auto& object = createObjectFrom("trigger-indicator");
  auto& params = getObjectParameters(WORLD_ORIENTATION_CHANGE);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;

  commit(object);
}

const ObjectParameters& getObjectParameters(EntityType entityType) {
  return entityToObjectParametersMap.at(entityType);
}

// @todo queryObjectByGridCoordinates, use a distance allowance term
Object* queryObjectByPosition(Globals, ObjectPool& objects, const Vec3f& position) {
  for (auto& object : objects) {
    if (object.position == position) {
      return &object;
    }
  }

  return nullptr;
}

void createObjectFromCoordinates(Globals, const GridCoordinates& coordinates) {
  auto* entity = state.world.grid.get(coordinates);

  if (entity == nullptr) {
    return;
  }

  switch (entity->type) {
    case GROUND:
      createGroundObject(globals, coordinates);
      break;
    case STAIRCASE:
      createStaircaseObject(globals, coordinates, ((Staircase*)entity)->orientation);        
      break;
    case SWITCH:
      createSwitchObject(globals, coordinates);
      break;
    case WORLD_ORIENTATION_CHANGE:
      #if DEVELOPMENT == 1
        createWorldOrientationChangeObject(globals, coordinates, ((WorldOrientationChange*)entity)->targetWorldOrientation);
      #endif
      break;
    default:
      break;
  }
}