#include "Gamma.h"

#include "game_state.h"
#include "move_queue.h"

static Gamma::Vec3f getPositionFromGridCoordinates(const GridCoordinates& coordinates) {
  return {
    coordinates.x * 15.0f,
    coordinates.y * 15.0f,
    coordinates.z * 15.0f
  };
}

static GridCoordinates getGridCoordinatesFromPosition(const Gamma::Vec3f& position) {
  return {
    int(std::roundf(position.x / 15.0f)),
    int(std::roundf(position.y / 15.0f)),
    int(std::roundf(position.z / 15.0f))
  };
}

static float easeOut(float a, float b, float alpha) {
  const float delta = b - a;
  const float t = 1.0f - std::powf(2, -10.0f * alpha);

  return a + delta * t;
}

static float easeInOut(float a, float b, float alpha) {
  const float delta = b - a;

  const float t = alpha < 0.5f
    ? 2.0f * alpha * alpha
    : 1.0f - std::powf(-2.0f * alpha + 2.0f, 2.0f) / 2.0f;

  return a + delta * t;
}

static void movePlayer(args(), float dt) {
  using namespace Gamma;

  auto& camera = getCamera();
  auto& from = state.currentMove.from;
  auto& to = state.currentMove.to;
  float alpha = context->scene.runningTime - state.currentMove.startTime;

  if (state.currentMove.easing == EasingType::EASE_IN_OUT) {
    alpha *= 3.0f;
  } else if (state.currentMove.easing == EasingType::LINEAR) {
    alpha *= 4.0f;
  }

  if (alpha >= 1.0f) {
    camera.position = to;
    state.moving = false;
  } else {
    #define EASE(easeFn)\
      camera.position.x = easeFn(from.x, to.x, alpha);\
      camera.position.y = easeFn(from.y, to.y, alpha);\
      camera.position.z = easeFn(from.z, to.z, alpha);

    switch (state.currentMove.easing) {
      case EasingType::EASE_IN_OUT:
        EASE(easeInOut);
        break;
      case EasingType::LINEAR:
        EASE(Gm_Lerpf);
        break;
      case EasingType::EASE_OUT:
        EASE(easeOut);
        break;
    }
  }
}

static Gamma::Vec3f getGridDirection(const Gamma::Vec3f& vector) {
  #define abs(n) (n < 0.0f ? -n : n)

  auto direction = vector.xz();

  if (abs(direction.x) > abs(direction.z)) {
    direction.z = 0.0f;
  } else {
    direction.x = 0.0f;
  }

  return direction.unit();
}

static MoveDirection getMoveDirection(const Gamma::Vec3f& gridDirection) {
  if (gridDirection.x > 0.0f) {
    return MoveDirection::X_RIGHT;
  } else if (gridDirection.x < 0.0f) {
    return MoveDirection::X_LEFT;
  } else if (gridDirection.z > 0.0f) {
    return MoveDirection::Z_FORWARD;
  } else if (gridDirection.z < 0.0f) {
    return MoveDirection::Z_BACKWARD;
  }

  return MoveDirection::NONE;
}

static void updateGame(args(), float dt) {
  using namespace Gamma;

  // Gm_HandleFreeCameraMode(context, dt);

  auto& camera = getCamera();
  auto& input = getInput();
  float time = context->scene.runningTime;
  auto gridDirection = getGridDirection(camera.orientation.getDirection());
  auto leftGridDirection = getGridDirection(camera.orientation.getLeftDirection());

  MoveDirection move = MoveDirection::NONE;

  if (input.isKeyHeld(Key::W)) {
    move = getMoveDirection(gridDirection);
  } else if (input.isKeyHeld(Key::S)) {
    move = getMoveDirection(gridDirection.invert());
  } else if (input.isKeyHeld(Key::A)) {
    move = getMoveDirection(leftGridDirection);
  } else if (input.isKeyHeld(Key::D)) {
    move = getMoveDirection(leftGridDirection.invert());
  }

  if (
    move != MoveDirection::NONE &&
    (time - state.currentMove.startTime) > 0.15f &&
    checkNextMove(state.moves, 1) != move
  ) {
    addMove(state.moves, move);
  }

  if (state.moves.size > 0 && (time - state.currentMove.startTime) > 0.2f) {
    auto nextMove = takeNextMove(state.moves);
    auto currentGridCoordinates = getGridCoordinatesFromPosition(camera.position);
    auto targetWorldPosition = getPositionFromGridCoordinates(currentGridCoordinates);

    switch (nextMove) {
      case Z_FORWARD:
        targetWorldPosition.z += 15.0f;
        break;
      case Z_BACKWARD:
        targetWorldPosition.z -= 15.0f;
        break;
      case X_LEFT:
        targetWorldPosition.x -= 15.0f;
        break;
      case X_RIGHT:
        targetWorldPosition.x += 15.0f;
        break;
    }

    if (!state.moving) {
      // First move
      state.currentMove.easing = EasingType::EASE_IN_OUT;
    } else if (state.moves.size == 0) {
      // Last move
      state.currentMove.easing = EasingType::EASE_OUT;
    } else if (state.moves.size >= 1) {
      // More moves coming
      state.currentMove.easing = EasingType::LINEAR;
    }

    state.moving = true;
    state.currentMove.startTime = time;
    state.currentMove.from = camera.position;
    state.currentMove.to = targetWorldPosition;
  }

  if (state.moving) {
    movePlayer(context, state, dt);
  }
}