#include "world_system.h"
#include "object_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

// @todo relocate to object_system
static void createGroundObject(Globals, const GridCoordinates& coordinates) {
  auto& object = createObjectFrom("ground");
  auto& params = getObjectParameters(GROUND);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.scale = params.scale;
  object.color = params.color;

  commit(object);
}

// @todo relocate to object_system
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

// @todo relocate to object_system
static void createWorldOrientationChangeObject(Globals, const GridCoordinates& coordinates, WorldOrientation worldOrientation) {
  auto& object = createObjectFrom("trigger-indicator");
  auto& params = getObjectParameters(WORLD_ORIENTATION_CHANGE);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;

  commit(object);
}

// @todo relocate to object_system
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
    case WORLD_ORIENTATION_CHANGE:
      #if DEVELOPMENT == 1
        createWorldOrientationChangeObject(globals, coordinates, ((WorldOrientationChange*)entity)->targetWorldOrientation);
      #endif
      break;
    default:
      break;
  }
}