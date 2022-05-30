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
  const float t = alpha >= 1.0f ? 1.0f : 1.0f - std::powf(2, -10.0f * alpha);

  return a + delta * t;
}

static void movePlayer(args(), float dt) {
  using namespace Gamma;

  auto& camera = getCamera();
  auto& from = state.currentMove.from;
  auto& to = state.currentMove.to;
  float alpha = context->scene.runningTime - state.currentMove.startTime;

  if (alpha >= 1.0f) {
    camera.position = to;
    state.moving = false;
  } else {
    camera.position.x = easeOut(from.x, to.x, alpha);
    camera.position.y = easeOut(from.y, to.y, alpha);
    camera.position.z = easeOut(from.z, to.z, alpha);
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

  // @todo implement a movement queue instead of
  // allowing keys to affect velocity in real time
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

  if (move != MoveDirection::NONE && (time - state.moves.lastMoveTime) > 0.25f) {
    addMove(state.moves, move, time);
  }

  if (state.moves.size > 0 && (time - state.currentMove.startTime) > 0.25f) {
    auto nextMove = takeNextMove(state.moves);
    auto currentGridCoordinates = getGridCoordinatesFromPosition(camera.position);
    auto targetGridPosition = getPositionFromGridCoordinates(currentGridCoordinates);

    switch (nextMove) {
      case Z_FORWARD:
        targetGridPosition.z += 15.0f;
        break;
      case Z_BACKWARD:
        targetGridPosition.z -= 15.0f;
        break;
      case X_LEFT:
        targetGridPosition.x -= 15.0f;
        break;
      case X_RIGHT:
        targetGridPosition.x += 15.0f;
        break;
    }

    state.moving = true;
    state.currentMove.startTime = time;
    state.currentMove.from = camera.position;
    state.currentMove.to = targetGridPosition;

    if (camera.position.z > targetGridPosition.z && nextMove == Z_FORWARD) {
      state.currentMove.to.z += 15.0f;
    } else if (camera.position.z < targetGridPosition.z && nextMove == Z_BACKWARD) {
      state.currentMove.to.z -= 15.0f;
    } else if (camera.position.x > targetGridPosition.x && nextMove == X_RIGHT) {
      state.currentMove.to.x += 15.0f;
    } else if (camera.position.x < targetGridPosition.x && nextMove == X_LEFT) {
      state.currentMove.to.x -= 15.0f;
    }
  }

  if (state.moving) {
    movePlayer(context, state, dt);
  }
}