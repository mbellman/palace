#pragma once

#include <vector>

#include "Gamma.h"

#include "game_entities.h"
#include "grid_utilities.h"
#include "build_flags.h"

#define MAX_EDIT_ACTIONS 5

struct GmContext;
struct GameState;

struct ReplacedEntityRecord {
  GridCoordinates coordinates;
  StaticEntity* entity = nullptr;
};

struct EditAction {
  StaticEntity* oldEntity = nullptr;
  GridCoordinates coordinates;

  bool isRangedPlacementAction = false;
  GridCoordinates rangeFrom;
  GridCoordinates rangeTo;
  std::vector<ReplacedEntityRecord> replacedEntityRecords;
};

struct WorldEditor {
  bool enabled = false;
  bool useRange = false;
  bool rangeFromSelected = false;
  bool deleting = false;
  StaticEntityType currentSelectedEntityType = GROUND;  // @todo make adjustable
  GridCoordinates rangeFrom;
  GridCoordinates rangeTo;
  EditAction editActions[MAX_EDIT_ACTIONS];
  u8 totalEditActions = 0;
};

#if DEVELOPMENT == 1
  void setCurrentSelectedEntityType(GmContext* context, GameState& state, StaticEntityType type);
  void selectRangeFrom(GmContext* context, GameState& state);
  void showStaticEntityPlacementPreview(GmContext* context, GameState& state);
  void showRangeFromSelectionPreview(GmContext* context, GameState& state);
  void showRangedEntityPlacementPreview(GmContext* context, GameState& state);
  void handleEditorSingleTileClickAction(GmContext* context, GameState& state);
  void handleEditorRangedClickAction(GmContext* context, GameState& state);
  void undoPreviousEditAction(GmContext* context, GameState& state);
#endif