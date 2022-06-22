#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

#if DEVELOPMENT == 1

static void removeObjectAtPosition(args(), ObjectPool& objects, const Vec3f& position) {
  for (auto& object : objects) {
    if (object.position == position) {
      remove(object);

      break;
    }
  }
}

static void removeStaticEntityAtGridCoordinates(args(), const GridCoordinates& coordinates) {
  auto& grid = state.world.grid;
  auto* entity = grid.get(coordinates);

  if (entity == nullptr) {
    return;
  }

  auto objectPosition = gridCoordinatesToWorldPosition(coordinates);

  switch (entity->type) {
    case GROUND:
      removeObjectAtPosition(params(), mesh("ground-tile")->objects, objectPosition);
      break;
    case STAIRCASE:
      removeObjectAtPosition(params(), mesh("staircase")->objects, objectPosition);
      break;
    case WALKABLE_SPACE:
      removeObjectAtPosition(params(), mesh("entity-indicator")->objects, objectPosition);
      break;
    default:
      break;
  }
}

static bool findStaticEntityPlacementCoordinates(args(), GridCoordinates& coordinates) {
  auto& camera = getCamera();
  auto& grid = state.world.grid;
  float offset = TILE_SIZE * 2.f;
  bool canPlaceEntity = false;

  while (offset < 80.f) {
    auto ray = camera.position + camera.orientation.getDirection() * offset;
    auto testCoordinates = worldPositionToGridCoordinates(ray);

    if (
      grid.has(testCoordinates) &&
      grid.get(testCoordinates)->type == state.editor.currentSelectedEntityType
    ) {
      break;
    }

    canPlaceEntity = true;
    coordinates = testCoordinates;
    offset += TILE_SIZE;
  }

  return canPlaceEntity;
}

void showStaticEntityPlacementPreview(args()) {
  auto& preview = object("preview");
  GridCoordinates previewCoordinates;

  if (findStaticEntityPlacementCoordinates(params(), previewCoordinates)) {
    Vec3f targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

    // @todo use proper scale/color based on entity type
    preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 2.f) * 0.1f);
    preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
    preview.color = Vec3f(1.f, 0.7f, 0.3f);
  } else {
    preview.scale = 0.f;
  }

  commit(preview);
}

void maybePlaceStaticEntity(args()) {
  auto& grid = state.world.grid;
  GridCoordinates targetCoordinates;
  StaticEntity* entity;
  Object object;

  if (!findStaticEntityPlacementCoordinates(params(), targetCoordinates)) {
    return;
  }

  switch (state.editor.currentSelectedEntityType) {
    default:
    case GROUND:
      // @todo world_system -> createGroundTileObject()
      entity = new Ground;
      object = createObjectFrom("ground-tile");
      object.scale = HALF_TILE_SIZE * 0.98f;
      object.color = Vec3f(1.f, 0.7f, 0.3f);
      break;
    case STAIRCASE:
      // @todo world_system -> createStaircaseObject()
      entity = new Staircase;
      object = createObjectFrom("staircase");
      break;
  }

  removeStaticEntityAtGridCoordinates(params(), targetCoordinates);

  grid.set(targetCoordinates, entity);

  object.position = gridCoordinatesToWorldPosition(targetCoordinates);

  commit(object);
}

#endif