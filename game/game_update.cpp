#include "Gamma.h"

#include "game_update.h"
#include "movement_system.h"

void updateGame(args(), float dt) {
  handlePlayerMovement(params(), dt);
  // Gm_HandleFreeCameraMode(context, dt);

  for (auto& lamp : objects("lamp")) {
    lamp.position.y = 25.f + std::sinf(getRunningTime() * 0.5f + lamp.position.x + lamp.position.z);

    commit(lamp);
  }
}