#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "game_macros.h"

using namespace Gamma;

void showStaticEntityPlacementPreview(args()) {
  auto& camera = getCamera();
  auto& preview = object("preview");
  // @todo march through tiles until a collision is found, or stop at the max distance
  auto& offset = camera.position + camera.orientation.getDirection() * 50.f;
  auto& coordinates = worldPositionToGridCoordinates(offset);

  // @todo use proper scale/color based on entity type
  preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 2.f) * 0.1f);
  preview.position = gridCoordinatesToWorldPosition(coordinates);
  preview.color = Vec3f(1.f, 0.7f, 0.3f);

  commit(preview);
}