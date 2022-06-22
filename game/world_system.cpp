#include "world_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

Object* queryObjectByPosition(args(), ObjectPool& objects, const Vec3f& position) {
  for (auto& object : objects) {
    if (object.position == position) {
      return &object;
    }
  }

  return nullptr;
}

void createGroundObject(args(), const GridCoordinates& coordinates) {
  auto& ground = createObjectFrom("ground-tile");

  ground.position = gridCoordinatesToWorldPosition(coordinates);
  ground.scale = HALF_TILE_SIZE * 0.98f;
  ground.color = Vec3f(1.f, 0.7f, 0.3f);

  commit(ground);
}

void createStaircaseObject(args(), const GridCoordinates& coordinates, const Orientation& orientation) {
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