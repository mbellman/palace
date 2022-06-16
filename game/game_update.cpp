#include "Gamma.h"

#include "game_update.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

std::string worldOrientationToString(WorldOrientation worldOrientation) {
  switch (worldOrientation) {
    case POSITIVE_Y_UP: return "+Y up";
    case NEGATIVE_Y_UP: return "-Y up";
    case POSITIVE_X_UP: return "+X up";
    case NEGATIVE_X_UP: return "-X up";
    case POSITIVE_Z_UP: return "+Z up";
    case NEGATIVE_Z_UP: return "-Z up";
  }
}

static void addDebugMessages(args()) {
  auto& camera = getCamera();
  auto& coordinates = worldPositionToGridCoordinates(camera.position);

  std::string positionLabel = "Grid position: "
    + std::to_string(coordinates.x) + ","
    + std::to_string(coordinates.y) + ","
    + std::to_string(coordinates.z);

  std::string orientationLabel = std::string("Camera orientation: ")
    + std::to_string(camera.orientation.pitch) + " (pitch), "
    + std::to_string(camera.orientation.yaw) + " (yaw), "
    + std::to_string(camera.orientation.roll) + " (roll)";

  std::string worldOrientationLabel = "World orientation: " + worldOrientationToString(state.worldOrientationState.worldOrientation);

  addDebugMessage(positionLabel);
  addDebugMessage(orientationLabel);
  addDebugMessage(worldOrientationLabel);
}

void updateGame(args(), float dt) {
  handlePlayerMovement(params(), dt);
  // Gm_HandleFreeCameraMode(context, dt);
  handleWorldOrientation(params(), dt);

  addDebugMessages(params());
}