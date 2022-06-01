#include "Gamma.h"

#include "game_state.h"
#include "move_queue.h"

using namespace Gamma;

constexpr static float TILE_SIZE = 15.f;

static float easeOut(float a, float b, float alpha) {
  const float delta = b - a;
  const float t = 1.f - std::powf(2, -10.f * alpha);

  return a + delta * t;
}

static float easeInOut(float a, float b, float alpha) {
  const float delta = b - a;

  const float t = alpha < 0.5f
    ? 2.f * alpha * alpha
    : 1.f - std::powf(-2.f * alpha + 2.f, 2.f) / 2.f;

  return a + delta * t;
}

static Vec3f gridCoordinatesToWorldPosition(const GridCoordinates& coordinates) {
  return {
    coordinates.x * TILE_SIZE,
    coordinates.y * TILE_SIZE,
    coordinates.z * TILE_SIZE
  };
}

static GridCoordinates worldPositionToGridCoordinates(const Vec3f& position) {
  return {
    int(std::roundf(position.x / TILE_SIZE)),
    int(std::roundf(position.y / TILE_SIZE)),
    int(std::roundf(position.z / TILE_SIZE))
  };
}

static void movePlayer(args(), float dt) {
  auto& camera = getCamera();
  auto& from = state.currentMove.from;
  auto& to = state.currentMove.to;
  auto alpha = getRunningTime() - state.currentMove.startTime;

  if (state.currentMove.easing == EasingType::EASE_IN_OUT) {
    alpha *= 3.0f;
  } else if (state.currentMove.easing == EasingType::LINEAR) {
    alpha *= 4.0f;
  }

  if (alpha >= 1.f) {
    camera.position = to;
    state.moving = false;
  } else {
    #define easeCamera(easingFn)\
      camera.position.x = easingFn(from.x, to.x, alpha);\
      camera.position.y = easingFn(from.y, to.y, alpha);\
      camera.position.z = easingFn(from.z, to.z, alpha);

    switch (state.currentMove.easing) {
      case EasingType::EASE_IN_OUT:
        easeCamera(easeInOut);
        break;
      case EasingType::LINEAR:
        easeCamera(Gm_Lerpf);
        break;
      case EasingType::EASE_OUT:
        easeCamera(easeOut);
        break;
    }
  }
}

// @todo support y axis directions
static Vec3f worldDirectionToGridDirection(const Vec3f& vector) {
  #define abs(n) (n < 0.0f ? -n : n)

  auto direction = vector.xz();

  if (abs(direction.x) > abs(direction.z)) {
    direction.z = 0.0f;
  } else {
    direction.x = 0.0f;
  }

  return direction.unit();
}

// @todo support Y_UP + Y_DOWN
static MoveDirection getMoveDirection(const Vec3f& gridDirection) {
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

static MoveDirection getMoveDirectionFromKeyboardInput(args()) {
  auto& camera = getCamera();
  auto& input = getInput();
  auto gridDirection = worldDirectionToGridDirection(camera.orientation.getDirection());
  auto leftGridDirection = worldDirectionToGridDirection(camera.orientation.getLeftDirection());

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

  return move;
}

static void updateCurrentMoveAction(args()) {
  auto& camera = getCamera();
  auto runningTime = getRunningTime();
  auto nextMove = takeNextMove(state.moves);
  auto currentGridCoordinates = worldPositionToGridCoordinates(camera.position);
  auto targetWorldPosition = gridCoordinatesToWorldPosition(currentGridCoordinates);

  // @todo handle Y_UP + Y_DOWN
  switch (nextMove) {
    case Z_FORWARD:
      targetWorldPosition.z += TILE_SIZE;
      break;
    case Z_BACKWARD:
      targetWorldPosition.z -= TILE_SIZE;
      break;
    case X_LEFT:
      targetWorldPosition.x -= TILE_SIZE;
      break;
    case X_RIGHT:
      targetWorldPosition.x += TILE_SIZE;
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
  state.currentMove.startTime = runningTime;
  state.currentMove.from = camera.position;
  state.currentMove.to = targetWorldPosition;
}

static void handlePlayerMovement(args(), float dt) {
  auto& camera = getCamera();
  auto& input = getInput();
  auto runningTime = getRunningTime();
  auto move = getMoveDirectionFromKeyboardInput(params());

  if (
    move != MoveDirection::NONE &&
    (runningTime - state.currentMove.startTime) > 0.15f &&
    checkNextMove(state.moves, 1) != move
  ) {
    addMove(state.moves, move);
  }

  if (
    state.moves.size > 0 &&
    (runningTime - state.currentMove.startTime) > 0.2f
  ) {
    updateCurrentMoveAction(params());
  }

  if (state.moving) {
    movePlayer(context, state, dt);
  }
}