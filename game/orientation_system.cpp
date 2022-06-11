#include "orientation_system.h"
#include "easing_utilities.h"
#include "game_state.h"
#include "game_macros.h"

// Wrap a value to within the [0, TAU] range
#define wrap(n) n = n > Gm_TAU ? n - Gm_TAU : n < 0.f ? n + Gm_TAU : n

using namespace Gamma;

void updateCameraFromMouseMoveEvent(args(), const MouseMoveEvent& event) {
  auto& camera = getCamera();
  auto mDeltaY = event.deltaY / 1000.f;
  auto mDeltaX = event.deltaX / 1000.f;

  switch (state.worldOrientationState.worldOrientation) {
    case POSITIVE_Y_UP:
    case POSITIVE_X_UP:
    case NEGATIVE_X_UP:
      camera.orientation.pitch += mDeltaY;
      camera.orientation.yaw += mDeltaX;
      break;
    case NEGATIVE_Y_UP:
      camera.orientation.pitch += mDeltaY;
      camera.orientation.yaw -= mDeltaX;
      break;
    case POSITIVE_Z_UP:
      camera.orientation.pitch += mDeltaY;
      camera.orientation.roll -= mDeltaX;
      break;
    case NEGATIVE_Z_UP:
      camera.orientation.pitch += mDeltaY;
      camera.orientation.roll += mDeltaX;
      break;
  }

  wrap(camera.orientation.pitch);
  wrap(camera.orientation.yaw);
  wrap(camera.orientation.roll);
}

void setWorldOrientation(args(), WorldOrientation worldOrientation) {
  auto& camera = getCamera();

  if (state.worldOrientationState.worldOrientation == worldOrientation) {
    return;
  }

  switch (worldOrientation) {
    case POSITIVE_Y_UP:
    case NEGATIVE_Y_UP: {
      // @todo cleanup
      Orientation o = camera.orientation;
      o.face(o.getDirection(), o.getUpDirection());
      o.roll = 0.f;

      state.worldOrientationState.orientationTo = o;
      state.worldOrientationState.movementPlane = Vec3f(1.f, 0, 1.f);

      break;
    }
    case POSITIVE_Z_UP:
    case NEGATIVE_Z_UP: {
      // @todo cleanup
      Orientation o = camera.orientation;
      Vec3f dir = o.getDirection();

      Orientation t;
      t.roll = atan2f(dir.y, dir.x) - Gm_PI / 2.f;
      // @hack add 0.1f to avoid a pitch drifting error
      t.pitch = dir.z * Gm_PI / 2.f - Gm_PI / 2.f + 0.1f;
      t.yaw = 0.f;

      state.worldOrientationState.orientationTo = t;
      state.worldOrientationState.movementPlane = Vec3f(1.f, 1.f, 0);

      break;
    }
    case POSITIVE_X_UP:
    case NEGATIVE_X_UP: {
      // @todo define orientationTo
      state.worldOrientationState.movementPlane = Vec3f(0, 1.f, 1.f);
      break;
    }
  }

  // @todo use above
  // case POSITIVE_X_UP:
  //   camera.orientation.roll = Gm_PI / 2.f;
  //   break;
  // case NEGATIVE_X_UP:
  //   camera.orientation.roll = -Gm_PI / 2.f;
  //   break;

  state.worldOrientationState.startTime = getRunningTime();
  state.worldOrientationState.worldOrientation = worldOrientation;
  state.worldOrientationState.orientationFrom = camera.orientation;
}

void handleWorldOrientation(args(), float dt) {
  auto& camera = getCamera();
  auto& orientationFrom = state.worldOrientationState.orientationFrom;
  auto& orientationTo = state.worldOrientationState.orientationTo;
  auto alpha = getRunningTime() - state.worldOrientationState.startTime;

  // @todo change easing time based on rotation proximity
  alpha *= 2.f;

  if (state.worldOrientationState.startTime == 0.f) {
    camera.rotation = camera.orientation.toQuaternion();

    return;
  }

  if (alpha >= 1.f) {
    camera.orientation = orientationTo;
    state.worldOrientationState.startTime = 0.f;

    return;
  }

  // @bug this breaks light disc projection, causing lights to occasionally
  // disappear during the world orientation transition
  camera.rotation = Quaternion::slerp(
    orientationFrom.toQuaternion(),
    orientationTo.toQuaternion(),
    easeInOut(0.f, 1.f, alpha)
  );
}

GridCoordinates getUpGridCoordinates(WorldOrientation worldOrientation) {
  switch (worldOrientation) {
    default:
    case POSITIVE_Y_UP:
      return GridCoordinates(0, 1, 0);
    case NEGATIVE_Y_UP:
      return GridCoordinates(0, -1, 0);
    case POSITIVE_Z_UP:
      return GridCoordinates(0, 0, 1);
    case NEGATIVE_Z_UP:
      return GridCoordinates(0, 0, -1);
    case POSITIVE_X_UP:
      return GridCoordinates(1, 0, 0);
    case NEGATIVE_X_UP:
      return GridCoordinates(-1, 0, 0);
  }
}

GridCoordinates getDownGridCoordinates(WorldOrientation worldOrientation) {
  switch (worldOrientation) {
    default:
    case POSITIVE_Y_UP:
      return GridCoordinates(0, -1, 0);
    case NEGATIVE_Y_UP:
      return GridCoordinates(0, 1, 0);
    case POSITIVE_Z_UP:
      return GridCoordinates(0, 0, -1);
    case NEGATIVE_Z_UP:
      return GridCoordinates(0, 0, 1);
    case POSITIVE_X_UP:
      return GridCoordinates(-1, 0, 0);
    case NEGATIVE_X_UP:
      return GridCoordinates(1, 0, 0);
  }
}