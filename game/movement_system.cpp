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

  // Add dt to the easing alpha value to ensure that
  // the camera is always in motion while executing
  // movement. Since the 'from' position of the current
  // move represents where the camera was when the
  // move started, and since we run this routine on
  // the same frame as updating the current move,
  // we don't want to wait a frame to start moving
  // the camera, as this produces a jarring stutter.
  auto alpha = (getRunningTime() - state.currentMove.startTime) + dt;

  if (state.currentMove.easing == EasingType::EASE_IN_OUT) {
    alpha *= 3.f;
  } else if (state.currentMove.easing == EasingType::LINEAR) {
    alpha *= 4.f;
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

  #define pressed(key) input.getLastKeyDown() == (uint64)key && input.isKeyHeld(key)

  if (pressed(Key::W)) {
    move = getMoveDirection(gridDirection);
  } else if (pressed(Key::S)) {
    move = getMoveDirection(gridDirection.invert());
  } else if (pressed(Key::A)) {
    move = getMoveDirection(leftGridDirection);
  } else if (pressed(Key::D)) {
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
  auto timeSinceLastMoveInput = runningTime - state.lastMovementInputTime;
  auto timeSinceCurrentMoveBegan = runningTime - state.currentMove.startTime;

  // Only advance the target world position along the
  // next move direction if the last move input was
  // sufficiently recent in time. This slightly reduces
  // the incidence of overshot.
  if (timeSinceLastMoveInput < 0.15f) {
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
  }

  if (!state.moving || timeSinceCurrentMoveBegan > 0.4f) {
    // The move was either entered while standing still,
    // or after having sufficiently slowed down from a
    // prior move sequence. An in-out easing works best
    // for single tile moves or the beginning of a move
    // sequence, so we apply it in these cases.
    state.currentMove.easing = EasingType::EASE_IN_OUT;
  } else if (state.moves.size == 0) {
    // Last move in a sequence, so slow down and stop
    state.currentMove.easing = EasingType::EASE_OUT;
  } else if (state.moves.size >= 1) {
    // More moves ahead in the sequence, so move
    // at a constant rate
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
  auto timeSinceCurrentMoveBegan = runningTime - state.currentMove.startTime;
  auto nextStoredMoveTimeThreshold = checkNextMove(state.moves) == move ? 0.175f : 0.1f;

  if (move != MoveDirection::NONE) {
    state.lastMovementInputTime = runningTime;
  }

  if (
    move != MoveDirection::NONE &&
    timeSinceCurrentMoveBegan > nextStoredMoveTimeThreshold &&
    checkNextMove(state.moves, 1) != move
  ) {
    storeMove(state.moves, move);
  }

  if (
    state.moves.size > 0 &&
    timeSinceCurrentMoveBegan > 0.2f
  ) {
    updateCurrentMoveAction(params());
  }

  if (state.moving) {
    movePlayer(params(), dt);
  }
}