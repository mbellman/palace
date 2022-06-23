#pragma once

#include "Gamma.h"

#include "game_entities.h"
#include "grid_utilities.h"

#define MAX_EDIT_ACTIONS 3

struct GmContext;
struct GameState;

struct EditAction {
  StaticEntity* newEntity = nullptr;
  StaticEntity* oldEntity = nullptr;
  GridCoordinates coordinates;
};

struct WorldEditor {
  StaticEntityType currentSelectedEntityType = GROUND;  // @todo make adjustable
  EditAction editActions[MAX_EDIT_ACTIONS];
  u8 totalEditActions = 0;
};

void showStaticEntityPlacementPreview(GmContext* context, GameState& state);
void tryPlacingStaticEntity(GmContext* context, GameState& state);
void undoPreviousEditAction(GmContext* context, GameState& state);