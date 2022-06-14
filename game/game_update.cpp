#include "Gamma.h"

#include "game_update.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "game_macros.h"

using namespace Gamma;

void updateGame(args(), float dt) {
  handlePlayerMovement(params(), dt);
  handleWorldOrientation(params(), dt);
  // Gm_HandleFreeCameraMode(context, dt);

  for (auto& lamp : objects("lamp")) {
    lamp.position.y = 25.f + std::sinf(getRunningTime() * 0.5f + lamp.position.x + lamp.position.z);

    commit(lamp);
  }

  auto& camera = getCamera();
  auto& coordinates = worldPositionToGridCoordinates(camera.position);

  std::string positionLabel = "Grid position:"
    + std::to_string(coordinates.x) + ","
    + std::to_string(coordinates.y) + ","
    + std::to_string(coordinates.z);

  addDebugMessage(positionLabel);
}