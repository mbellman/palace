#include "Gamma.h"

#include "entity_system.h"
#include "movement_system.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

static void stepOnSwitch(Globals) {
  state.isSteppingOnSwitch = true;

  Console::log("Switch!");
}

static void handleGridEntityBehavior(Globals) {
  if (isMoving(globals)) {
    return;
  }

  auto currentGridCoordinates = worldPositionToGridCoordinates(getCamera().position);
  auto downGridCoordinates = getDownGridCoordinates(state.worldOrientationState.worldOrientation);
  auto* entityBelow = state.world.grid.get(currentGridCoordinates + downGridCoordinates);

  // @todo clean up/generalize
  if (entityBelow != nullptr && entityBelow->type == SWITCH) {
    if (!state.isSteppingOnSwitch) {
      stepOnSwitch(globals);
    }
  } else {
    state.isSteppingOnSwitch = false;
  }
}

static void handleDynamicEntityBehavior(Globals) {
  // @todo
}

void handleEntityBehavior(Globals, float dt) {
  handleGridEntityBehavior(globals);
  handleDynamicEntityBehavior(globals);
}