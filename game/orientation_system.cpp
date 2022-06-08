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

  switch (state.worldOrientationState.orientation) {
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

void setWorldOrientation(args(), WorldOrientation orientation) {
  auto& camera = getCamera();

  if (state.worldOrientationState.orientation == orientation) {
    return;
  }

  switch (orientation) {
    case POSITIVE_Y_UP:
    case NEGATIVE_Y_UP:
      state.worldOrientationState.movementPlane = Vec3f(1.f, 0, 1.f);
      break;
    case POSITIVE_Z_UP:
    case NEGATIVE_Z_UP:
      state.worldOrientationState.movementPlane = Vec3f(1.f, 1.f, 0);
      break;
    case POSITIVE_X_UP:
    case NEGATIVE_X_UP:
      state.worldOrientationState.movementPlane = Vec3f(0, 1.f, 1.f);
      break;
  }

  state.worldOrientationState.startTime = getRunningTime();
  state.worldOrientationState.orientation = orientation;
  state.worldOrientationState.from = camera.orientation;
}

void handleWorldOrientation(args(), float dt) {
  auto& camera = getCamera();
  auto& from = state.worldOrientationState.from;
  auto alpha = getRunningTime() - state.worldOrientationState.startTime;

  alpha *= 2.f;

  if (alpha >= 1.f) {
    // @todo investigate whether there are any issues
    // with the final tweened orientation being off,
    // since we don't perform easing at alpha = 1.f
    return;
  }

  // @todo control for initial facing direction
  switch (state.worldOrientationState.orientation) {
    case POSITIVE_Y_UP:
      // camera.orientation.pitch = easeOut(from.pitch, 0.f, alpha);
      camera.orientation.roll = easeOutCircular(from.roll, 0.f, alpha);
      break;
    case NEGATIVE_Y_UP:
      camera.orientation.pitch = easeOutCircular(from.pitch, -Gm_PI, alpha);
      camera.orientation.roll = easeOutCircular(from.roll, 0.f, alpha);
      break;
    case POSITIVE_Z_UP:
      camera.orientation.pitch = easeOutCircular(from.pitch, Gm_PI / 2.f, alpha);
      camera.orientation.yaw = easeOutCircular(from.yaw, 0.f, alpha);
      break;
    case NEGATIVE_Z_UP:
      // camera.orientation.pitch = easeOutCircular(from.pitch, Gm_PI / 2.f, alpha);
      camera.orientation.yaw = easeOutCircular(from.yaw, Gm_PI, alpha);
      break;
    case POSITIVE_X_UP:
      camera.orientation.pitch = easeOutCircular(from.pitch, 0.f, alpha);
      camera.orientation.roll = easeOutCircular(from.roll, Gm_PI / 2.f, alpha);
      break;
    case NEGATIVE_X_UP:
      camera.orientation.pitch = easeOutCircular(from.pitch, Gm_PI, alpha);
      camera.orientation.roll = easeOutCircular(from.roll, -Gm_PI / 2.f, alpha);
      break;
  }
}