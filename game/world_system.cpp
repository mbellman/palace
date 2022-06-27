#include "world_system.h"
#include "object_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

// @todo relocate to object_system
static void createGroundObject(Globals, const GridCoordinates& coordinates) {
  auto& ground = createObjectFrom("ground");
  auto& params = getObjectParameters(GROUND);

  ground.position = gridCoordinatesToWorldPosition(coordinates);
  ground.scale = params.scale;
  ground.color = params.color;

  commit(ground);
}

// @todo relocate to object_system
static void createStaircaseObject(Globals, const GridCoordinates& coordinates, const Orientation& orientation) {
  auto& staircase = createObjectFrom("staircase");
  auto& params = getObjectParameters(STAIRCASE);

  staircase.position = gridCoordinatesToWorldPosition(coordinates);
  staircase.color = params.color;
  staircase.scale = params.scale;
  staircase.rotation.x = orientation.pitch;
  staircase.rotation.y = orientation.yaw;
  staircase.rotation.z = -orientation.roll;

  commit(staircase);
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
    default:
      break;
  }
}