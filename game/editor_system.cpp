#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "world_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

#if DEVELOPMENT == 1
  static void removeObjectAtPosition(args(), ObjectPool& objects, const Vec3f& position) {
    auto* object = queryObjectByPosition(params(), objects, position);

    if (object != nullptr) {
      remove(*object);
    }
  }

  static void removeStaticEntityObjectAtGridCoordinates(args(), const GridCoordinates& coordinates) {
    auto& grid = state.world.grid;
    auto* entity = grid.get(coordinates);

    if (entity == nullptr) {
      return;
    }

    auto objectPosition = gridCoordinatesToWorldPosition(coordinates);
    Mesh* mesh;

    switch (entity->type) {
      default:
      case GROUND:
        mesh = mesh("ground-tile");
        break;
      case STAIRCASE:
        mesh = mesh("staircase");
        break;
      case WALKABLE_SPACE:
        mesh = mesh("entity-indicator");
        break;
    }

    removeObjectAtPosition(params(), mesh->objects, objectPosition);
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
      offset += HALF_TILE_SIZE / 4.f;
    }

    return canPlaceEntity;
  }

  void showStaticEntityPlacementPreview(args()) {
    auto& preview = object("preview");
    GridCoordinates previewCoordinates;

    if (findStaticEntityPlacementCoordinates(params(), previewCoordinates)) {
      Vec3f targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

      // @todo use proper scale/color based on entity type
      preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.color = Vec3f(1.f, 0.7f, 0.3f);
    } else {
      preview.scale = 0.f;
    }

    commit(preview);
  }

  void tryPlacingStaticEntity(args()) {
    GridCoordinates targetCoordinates;

    if (!findStaticEntityPlacementCoordinates(params(), targetCoordinates)) {
      return;
    }

    auto& grid = state.world.grid;
    StaticEntity* entity;

    removeStaticEntityObjectAtGridCoordinates(params(), targetCoordinates);

    switch (state.editor.currentSelectedEntityType) {
      default:
      case GROUND:
        entity = new Ground;

        createGroundObject(params(), targetCoordinates);
        break;
      case STAIRCASE:
        entity = new Staircase;
        // @todo set staircase orientation

        createStaircaseObject(params(), targetCoordinates, { 0.f, 0.f, 0.f });
        break;
    }

    grid.set(targetCoordinates, entity);
  }
#endif