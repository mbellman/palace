#include "Gamma.h"

#include "entity_system.h"
#include "movement_system.h"
#include "game_state.h"
#include "game_macros.h"

using namespace Gamma;

static void handleGridEntityBehavior(Globals) {
  if (isMoving(globals)) {
    return;
  }

  // @todo
}

static void handleDynamicEntityBehavior(Globals) {
  // @todo
}

void handleEntityBehavior(Globals, float dt) {
  handleGridEntityBehavior(globals);
  handleDynamicEntityBehavior(globals);
}