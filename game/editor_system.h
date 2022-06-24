#pragma once

#include "Gamma.h"

#include "game_entities.h"
#include "grid_utilities.h"
#include "build_flags.h"

#define MAX_EDIT_ACTIONS 5

struct GmContext;
struct GameState;

// @todo support storing/undoing ranged placement actions
struct EditAction {
  StaticEntity* oldEntity = nullptr;
  StaticEntity* newEntity = nullptr;
  GridCoordinates coordinates;
};

struct WorldEditor {
  bool enabled = false;
  bool useRange = false;
  bool rangeFromSelected = false;
  StaticEntityType currentSelectedEntityType = GROUND;  // @todo make adjustable
  GridCoordinates rangeFrom;
  GridCoordinates rangeTo;
  EditAction editActions[MAX_EDIT_ACTIONS];
  u8 totalEditActions = 0;
};

#if DEVELOPMENT == 1
  void selectRangeFrom(GmContext* context, GameState& state);
  void showStaticEntityPlacementPreview(GmContext* context, GameState& state);
  void showRangedEntityPlacementPreview(GmContext* context, GameState& state);
  void tryPlacingStaticEntity(GmContext* context, GameState& state);
  void placeStaticEntitiesOverCurrentRange(GmContext* context, GameState& state);
  void undoPreviousEditAction(GmContext* context, GameState& state);
#endif