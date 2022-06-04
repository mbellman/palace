#include "orientation_system.h"
#include "easing_utilities.h"
#include "game_macros.h"

using namespace Gamma;

void updateCameraFromMouseMoveEvent(args(), const MouseMoveEvent& event) {
  auto& camera = getCamera();
  auto mDeltaY = event.deltaY / 1000.f;
  auto mDeltaX = event.deltaX / 1000.f;

  switch (state.orientationState.orientation) {
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

  // Restrict orientation values within the [0, TAU] range
  #define restrict(n) n = n > Gm_TAU ? n - Gm_TAU : n < 0.f ? n + Gm_TAU : n

  restrict(camera.orientation.pitch);
  restrict(camera.orientation.yaw);
  restrict(camera.orientation.roll);
}

void setPlayerOrientation(args(), PlayerOrientation orientation) {
  auto& camera = getCamera();

  state.orientationState.startTime = getRunningTime();
  state.orientationState.orientation = orientation;

  switch (orientation) {
    case POSITIVE_Y_UP:
    case POSITIVE_X_UP:
    case NEGATIVE_Y_UP:
    case NEGATIVE_X_UP:
      state.orientationState.from = camera.orientation.roll;
      break;
    case POSITIVE_Z_UP:
    case NEGATIVE_Z_UP:
      state.orientationState.from = camera.orientation.yaw;
      break;
  }
}

void handlePlayerOrientation(args(), float dt) {
  auto& camera = getCamera();
  auto alpha = getRunningTime() - state.orientationState.startTime;
  auto from = state.orientationState.from;

  alpha *= 2.f;

  if (alpha >= 1.f) {
    alpha = 1.f;
  }

  // @todo shortest-path easing
  switch (state.orientationState.orientation) {
    case POSITIVE_Y_UP:
    case NEGATIVE_Y_UP:
      camera.orientation.roll = easeOut(from, 0.f, alpha);
      break;
    case POSITIVE_Z_UP:
    case NEGATIVE_Z_UP:
      camera.orientation.yaw = easeOut(from, 0.f, alpha);
      break;
    case POSITIVE_X_UP:
      camera.orientation.roll = easeOut(from, Gm_PI / 2.f, alpha);
      break;
    case NEGATIVE_X_UP:
      camera.orientation.roll = easeOut(from, -Gm_PI / 2.f, alpha);
      break;
  }
}