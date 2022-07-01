#include "Gamma.h"

#include "entity_system.h"
#include "movement_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

#define clamp(value) value = value > 1.f ? 1.f : value < 0.f ? 0.f : value

static void handleSwitchWhenActive(Globals, Switch* entity, float dt) {
  if (
    state.lastPressedSwitch != nullptr &&
    state.lastPressedSwitch != entity
  ) {
    // If stepping on a new switch while the last pressed switch
    // is still defined, reset the last pressed switch's duration
    state.lastPressedSwitch->pressedDuration = 0.f;
  }

  entity->pressedDuration += dt * 5.f;

  state.lastPressedSwitch = entity;
}

static void handleLastPressedSwitchWhenInactive(Globals, float dt) {
  state.lastPressedSwitch->pressedDuration -= dt * 5.f;
}

static void handleLastPressedSwitch(Globals, Switch* entity) {
  clamp(state.lastPressedSwitch->pressedDuration);

  auto currentGridCoordinates = worldPositionToGridCoordinates(getCamera().position);
  auto entityWorldPosition = gridCoordinatesToWorldPosition(currentGridCoordinates);
  auto& switchLight = light("switch-light");
  auto& switchParticles = mesh("switch-particles")->particleSystem;
  auto alpha = state.lastPressedSwitch->pressedDuration / 1.f;

  switchLight.position = entityWorldPosition;
  switchLight.power = alpha;

  switchParticles.medianSize = alpha * 2.f;
  switchParticles.sizeVariation = alpha;
  // @todo use world orientation for path displacement
  switchParticles.path[0] = entityWorldPosition - Vec3f(0, HALF_TILE_SIZE, 0);
  switchParticles.path[1] = entityWorldPosition + Vec3f(0, HALF_TILE_SIZE, 0);

  if (state.lastPressedSwitch->pressedDuration == 0.f) {
    state.lastPressedSwitch = nullptr;
  }
}

static void handleGridEntityBehavior(Globals, float dt) {
  auto currentGridCoordinates = worldPositionToGridCoordinates(getCamera().position);
  auto downGridCoordinates = getDownGridCoordinates(state.worldOrientationState.worldOrientation);
  auto* entityBelow = state.world.grid.get(currentGridCoordinates + downGridCoordinates);

  // Handle switches
  if (entityBelow != nullptr && entityBelow->type == SWITCH) {
    handleSwitchWhenActive(globals, (Switch*)entityBelow, dt);
  } else if (state.lastPressedSwitch != nullptr) {
    handleLastPressedSwitchWhenInactive(globals, dt);
  }

  if (state.lastPressedSwitch != nullptr) {
    handleLastPressedSwitch(globals, state.lastPressedSwitch);
  }
}

static void handleDynamicEntityBehavior(Globals, float dt) {
  // @todo
}

void handleEntityBehavior(Globals, float dt) {
  handleGridEntityBehavior(globals, dt);
  handleDynamicEntityBehavior(globals, dt);
}