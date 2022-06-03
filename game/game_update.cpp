#include "Gamma.h"

#include "game_update.h"
#include "movement_system.h"

using namespace Gamma;

// @todo create orientation_system.h/cpp?
// @todo smooth interpolation between orientations
static void handlePlayerOrientation(args(), float dt) {
  auto& camera = getCamera();

  switch (state.orientation.current) {
    case POSITIVE_Y_UP:
    case NEGATIVE_Y_UP:
      camera.orientation.roll = 0.f;
      break;
    case POSITIVE_Z_UP:
    case NEGATIVE_Z_UP:
      camera.orientation.yaw = 0.f;
      break;
    case POSITIVE_X_UP:
      camera.orientation.roll = Gm_PI / 2.f;
      break;
    case NEGATIVE_X_UP:
      camera.orientation.roll = -Gm_PI / 2.f;
      break;
  }
}

void updateGame(args(), float dt) {
  handlePlayerMovement(params(), dt);
  handlePlayerOrientation(params(), dt);
  // Gm_HandleFreeCameraMode(context, dt);

  for (auto& lamp : objects("lamp")) {
    lamp.position.y = 25.f + std::sinf(getRunningTime() * 0.5f + lamp.position.x + lamp.position.z);

    commit(lamp);
  }
}