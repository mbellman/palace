#include "world_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

static void createGroundObject(Globals, const GridCoordinates& coordinates) {
  auto& ground = createObjectFrom("ground");

  ground.position = gridCoordinatesToWorldPosition(coordinates);
  ground.scale = HALF_TILE_SIZE * 0.98f;
  ground.color = Vec3f(1.f, 0.7f, 0.3f);

  commit(ground);
}

static void createStaircaseObject(Globals, const GridCoordinates& coordinates, const Orientation& orientation) {
  auto& staircase = createObjectFrom("staircase");

  staircase.position = gridCoordinatesToWorldPosition(coordinates);
  staircase.color = Vec3f(0.5f);
  staircase.scale = HALF_TILE_SIZE;
  staircase.rotation.x = orientation.pitch;
  // @todo use proper model orientation to avoid the -PI/2 offset here
  staircase.rotation.y = -Gm_PI / 2.f + orientation.yaw;
  staircase.rotation.z = -orientation.roll;

  commit(staircase);
}

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

// @todo write a helper for serializing coordinate
// @todo serialize staircase entities + orientations
std::string serializeWorldGrid(Globals) {
  std::string serialized;

  serialized += "ground\n";

  for (auto& [ coordinates, entity ] : state.world.grid) {
    if (entity->type == GROUND) {
      serialized += std::to_string(coordinates.x) + "," + std::to_string(coordinates.y) + "," + std::to_string(coordinates.z) + "\n";
    }
  }

  return serialized;
}

void loadSerializedWorldGrid(Globals, const std::string& data) {
  // @todo
}