#include "zone_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "build_flags.h"

using namespace Gamma;

static bool cameraIsWithinZoneBoundaries(const Zone& zone, const Camera& camera) {
  auto coordinates = worldPositionToGridCoordinates(camera.position);
  auto& start = zone.start;
  auto& end = zone.end;

  return (
    coordinates.x >= start.x && coordinates.x <= end.x &&
    coordinates.y >= start.y && coordinates.y <= end.y &&
    coordinates.z >= start.z && coordinates.z <= end.z
  );
}

static void toggleMeshesWithinZone(Globals, const Zone& zone, bool enabled) {
  for (auto& meshName : zone.meshNames) {
    mesh(meshName)->disabled = !enabled;
  }
}

void handleZonesOnUpdate(Globals) {
  auto& camera = getCamera();
  auto& zones = state.world.zones;
  auto coordinates = worldPositionToGridCoordinates(camera.position);

  for (auto& zone : zones) {
    bool isActiveZone = cameraIsWithinZoneBoundaries(zone, camera);

    #if DEVELOPMENT == 1
      if (state.editor.enabled) {
        isActiveZone = true;
      }
    #endif

    toggleMeshesWithinZone(globals, zone, isActiveZone);
  }
}