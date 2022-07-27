#include "Gamma.h"

#include "game_update.h"
#include "movement_system.h"
#include "orientation_system.h"
#include "entity_system.h"
#include "zone_system.h"
#include "editor_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

#if DEVELOPMENT == 1
  static std::string worldOrientationToString(WorldOrientation worldOrientation) {
    switch (worldOrientation) {
      default:
      case POSITIVE_Y_UP: return "+Y up";
      case NEGATIVE_Y_UP: return "-Y up";
      case POSITIVE_X_UP: return "+X up";
      case NEGATIVE_X_UP: return "-X up";
      case POSITIVE_Z_UP: return "+Z up";
      case NEGATIVE_Z_UP: return "-Z up";
    }
  }

  static std::string gridCoordinatesToString(const GridCoordinates& coordinates) {
    return std::to_string(coordinates.x) + ", " + std::to_string(coordinates.y) + ", " + std::to_string(coordinates.z);
  }

  static void addDebugMessages(Globals) {
    auto& camera = getCamera();
    auto& coordinates = worldPositionToGridCoordinates(camera.position);

    std::string worldPositionLabel = "World position: "
      + std::to_string(camera.position.x) + ","
      + std::to_string(camera.position.y) + ","
      + std::to_string(camera.position.z);

    std::string gridPositionLabel = "Grid position: "
      + std::to_string(coordinates.x) + ","
      + std::to_string(coordinates.y) + ","
      + std::to_string(coordinates.z);

    std::string orientationLabel = std::string("Camera orientation: ")
      + std::to_string(camera.orientation.pitch) + " (pitch), "
      + std::to_string(camera.orientation.yaw) + " (yaw), "
      + std::to_string(camera.orientation.roll) + " (roll)";

    std::string worldOrientationLabel = "World orientation: " + worldOrientationToString(state.worldOrientationState.worldOrientation);
    std::string entitiesLabel = "Total static entities: " + std::to_string(state.world.grid.size());
    std::string editorWorldOrientationLabel = "Editor world orientation: " + worldOrientationToString(state.editor.currentSelectedWorldOrientation);
    std::string editorGridCoordinatesLabel = "Editor grid coordinates: " + gridCoordinatesToString(state.editor.currentSelectedGridCoordinates);

    addDebugMessage(worldPositionLabel);
    addDebugMessage(gridPositionLabel);
    addDebugMessage(orientationLabel);
    addDebugMessage(worldOrientationLabel);
    addDebugMessage(entitiesLabel);
    addDebugMessage(editorWorldOrientationLabel);
    addDebugMessage(editorGridCoordinatesLabel);
  }
#endif

static void animateCameraToTargetOrientation(Globals, float dt) {
  auto& camera = getCamera();
  auto& from = camera.orientation;
  auto& to = state.worldOrientationState.orientationTo;
  float speed = 16.0f * dt;

  if (speed > 1.f) speed = 1.f;

  camera.orientation.roll = Gm_LerpCircularf(from.roll, to.roll, speed, Gm_PI);
  camera.orientation.pitch = Gm_LerpCircularf(from.pitch, to.pitch, speed, Gm_PI);
  camera.orientation.yaw = Gm_LerpCircularf(from.yaw, to.yaw, speed, Gm_PI);
}

void updateGame(Globals, float dt) {
  #if DEVELOPMENT == 1
    if (Gm_IsFlagEnabled(FREE_CAMERA_MODE)) {
      Gm_HandleFreeCameraMode(context, dt);
    } else {
      handlePlayerCameraMovement(globals, dt);
      animateCameraToTargetOrientation(globals, dt);
    }
  #else
    handlePlayerCameraMovement(globals, dt);
    animateCameraToTargetOrientation(globals, dt);
  #endif

  handleWorldOrientation(globals, dt);
  handleEntityBehavior(globals, dt);
  handleZones(globals);

  #if DEVELOPMENT == 1
    if (state.editor.enabled) {
      // @todo move to editor_system
      if (state.editor.isFindingLight) {
        showLightFinderPreview(globals);
      } else if (state.editor.isPlacingLight) {
        showLightPlacementPreview(globals);
      } else if (state.editor.isFindingMesh) {
        showMeshFinderPreview(globals);
      } else if (state.editor.isPlacingMesh) {
        showMeshPlacementPreview(globals);
      } else if (state.editor.rangeFromSelected) {
        showRangedEntityPlacementPreview(globals);
      } else if (state.editor.useRange) {
        showRangeFromSelectionPreview(globals);
      } else {
        showGridEntityPlacementPreview(globals);
      }
    }

    // @todo create a separate routine for object/visibility culling behaviors
    if (state.editor.enabled) {
      objects("tulips").showAll();
      objects("tulip-petals").showAll();
    } else {
      useFrustumCulling({
        "tulips",
        "tulip-petals"
      });
    }

    mesh("trigger-indicator")->disabled = !state.editor.enabled;
    mesh("light-indicator")->disabled = !state.editor.enabled;

    // @todo we may actually have use for camera warp-to behavior
    // during gameplay, so it may be useful to extract this into
    // a gameplay_system module eventually
    float cameraTargetAlpha = 2.f * (getRunningTime() - state.cameraTargetStartTime);

    if (cameraTargetAlpha < 1.f) {
      auto& camera = getCamera();

      camera.position = Vec3f::lerp(
        state.cameraStartPosition,
        state.cameraTargetPosition,
        easeInOut(0.f, 1.f, cameraTargetAlpha)
      );
    }

    addDebugMessages(globals);
  #endif
}