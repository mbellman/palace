#pragma once

#include "game_entities.h"

struct GmContext;
struct GameState;

struct WorldEditor {
  StaticEntityType currentSelectedEntityType = GROUND;  // @todo make adjustable
};

void showStaticEntityPlacementPreview(GmContext* context, GameState& state);
void tryPlacingStaticEntity(GmContext* context, GameState& state);