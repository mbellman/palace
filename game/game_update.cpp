#include "Gamma.h"

#include "game_state.h"

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

  if (state.snap.active) {
    auto& from = state.snap.from;
    auto& to = state.snap.to;
    float alpha = (context->scene.runningTime - state.snap.startTime) + dt;

    if (alpha >= 1.0f) {
      camera.position = to;

      state.snap.active = false;
      state.velocity = Vec3f(0);
    } else {
      camera.position.x = easeOut(from.x, to.x, alpha);
      camera.position.y = easeOut(from.y, to.y, alpha);
      camera.position.z = easeOut(from.z, to.z, alpha);
    }
  } else {
    camera.position += state.velocity * dt;

    state.velocity *= 0.995f - (5.0f * dt);

    if (state.velocity.magnitude() < 0.1f) {
      state.velocity = Vec3f(0);
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

static void updateGame(args(), float dt) {
  using namespace Gamma;

  // Gm_HandleFreeCameraMode(context, dt);

  auto& camera = getCamera();
  auto& input = getInput();
  float acceleration = 500.0f * dt;
  auto gridDirection = getGridDirection(camera.orientation.getDirection());
  auto leftGridDirection = getGridDirection(camera.orientation.getLeftDirection());

  // @todo implement a movement queue instead of
  // allowing keys to affect velocity in real time
  if (input.isKeyHeld(Key::W)) {
    state.velocity += gridDirection * acceleration;
  } else if (input.isKeyHeld(Key::S)) {
    state.velocity += gridDirection.invert() * acceleration;
  } else if (input.isKeyHeld(Key::A)) {
    state.velocity += leftGridDirection * acceleration;
  } else if (input.isKeyHeld(Key::D)) {
    state.velocity += leftGridDirection.invert() * acceleration;
  } else if (state.velocity.magnitude() > 0.0f && !state.snap.active) {
    auto currentGridCoordinates = getGridCoordinatesFromPosition(camera.position);
    auto currentGridPosition = getPositionFromGridCoordinates(currentGridCoordinates);
    auto velocityDirection = getGridDirection(state.velocity);
    
    state.snap.active = true;
    state.snap.startTime = context->scene.runningTime;
    state.snap.from = camera.position;
    state.snap.to = currentGridPosition;

    if (camera.position.z > currentGridPosition.z && velocityDirection.z > 0.0f) {
      state.snap.to.z += 15.0f;
    } else if (camera.position.z < currentGridPosition.z && velocityDirection.z < 0.0f) {
      state.snap.to.z -= 15.0f;
    } else if (camera.position.x > currentGridPosition.x && velocityDirection.x > 0.0f) {
      state.snap.to.x += 15.0f;
    } else if (camera.position.x < currentGridPosition.x && velocityDirection.x < 0.0f) {
      state.snap.to.x -= 15.0f;
    }
  }

  if (state.velocity.magnitude() > 0.0f) {
    movePlayer(context, state, dt);
  }
}