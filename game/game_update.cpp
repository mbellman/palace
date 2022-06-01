#include "Gamma.h"

#include "movement_system.cpp"

static void updateGame(args(), float dt) {
  handlePlayerMovement(params(), dt);
}