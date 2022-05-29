#include "Gamma.h"

#include "game_state.h"

static void movePlayer(args(), float dt) {
  using namespace Gamma;

  auto& camera = getCamera();
  const auto& start = state.lastPosition;
  const auto& end = state.targetPosition;
  const float alpha = 2.0f * (context->scene.runningTime - state.moveStartTime);

  camera.position.x = Gm_Lerpf(start.x, end.x, alpha);
  camera.position.y = Gm_Lerpf(start.y, end.y, alpha);
  camera.position.z = Gm_Lerpf(start.z, end.z, alpha);

  if ((end - camera.position).magnitude() < 0.1f) {
    camera.position = end;

    state.isMoving = false;
    state.lastPosition = camera.position;
  }
}

static void startMovingTo(args(), const Gamma::Vec3f& targetPosition) {
  auto& camera = getCamera();

  state.isMoving = true;
  state.moveStartTime = context->scene.runningTime;
  state.lastPosition = camera.position;
  state.targetPosition = targetPosition;
}

static void updateGame(args(), float dt) {
  using namespace Gamma;

  // Gm_HandleFreeCameraMode(context, dt);

  if (state.isMoving) {
    movePlayer(context, state, dt);
  } else {
    auto& input = getInput();

    if (input.isKeyHeld(Key::W)) {
      startMovingTo(context, state, state.lastPosition + Vec3f(0, 0, 15.0f));
    } else if (input.isKeyHeld(Key::S)) {
      startMovingTo(context, state, state.lastPosition + Vec3f(0, 0, -15.0f));
    } else if (input.isKeyHeld(Key::A)) {
      startMovingTo(context, state, state.lastPosition + Vec3f(-15.0f, 0, 0));
    } else if (input.isKeyHeld(Key::D)) {
      startMovingTo(context, state, state.lastPosition + Vec3f(15.0f, 0, 0));
    }
  }
}