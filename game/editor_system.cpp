#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

void showStaticEntityPlacementPreview(args()) {
#if DEVELOPMENT == 1
  auto& camera = getCamera();
  auto& grid = state.world.grid;
  auto& preview = object("preview");
  float offset = TILE_SIZE * 2.f;
  GridCoordinates previewCoordinates;
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
    previewCoordinates = testCoordinates;
    offset += TILE_SIZE;
  }

  if (canPlaceEntity) {
    // @todo use proper scale/color based on entity type
    preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 2.f) * 0.1f);
    preview.position = gridCoordinatesToWorldPosition(previewCoordinates);
    preview.color = Vec3f(1.f, 0.7f, 0.3f);
  } else {
    preview.scale = 0.f;
  }

  commit(preview);
#endif
}