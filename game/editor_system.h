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

// @todo extends ReplacedEntityRecord
struct EditAction {
  // Single-tile actions
  StaticEntity* oldEntity = nullptr;
  GridCoordinates coordinates;

  // Ranged actions
  bool isRangedPlacementAction = false;
  GridCoordinates rangeFrom;
  GridCoordinates rangeTo;
  std::vector<ReplacedEntityRecord> replacedEntityRecords;
};

struct WorldEditor {
  // Flags
  bool enabled = false;
  bool useRange = false;
  bool rangeFromSelected = false;
  bool deleting = false;

  // Placeable entity selection
  StaticEntityType currentSelectedEntityType = GROUND;  // @todo deprecate
  u8 currentSelectedEntityIndex = 0;                    // @todo use as new value

  // Default orientation for placed entities
  Gamma::Orientation currentEntityOrientation;

  // Default world orientation target for World Orientation Change triggers
  WorldOrientation currentSelectedWorldOrientation = POSITIVE_Y_UP;

  // Ranged actions
  GridCoordinates rangeFrom;
  GridCoordinates rangeTo;

  // Edit action history/undo management
  EditAction editActions[MAX_EDIT_ACTIONS];
  u8 totalEditActions = 0;
};

#if DEVELOPMENT == 1
  void setCurrentSelectedEntityType(GmContext* context, GameState& state, StaticEntityType type);
  void adjustCurrentEntityOrientation(GmContext* context, GameState& state, const Gamma::Orientation& adjustment);
  void selectRangeFrom(GmContext* context, GameState& state);
  void showStaticEntityPlacementPreview(GmContext* context, GameState& state);
  void showRangeFromSelectionPreview(GmContext* context, GameState& state);
  void showRangedEntityPlacementPreview(GmContext* context, GameState& state);
  void handleEditorSingleTileClickAction(GmContext* context, GameState& state);
  void handleEditorRangedClickAction(GmContext* context, GameState& state);
  void undoPreviousEditAction(GmContext* context, GameState& state);
  void placeCameraAtClosestWalkableTile(GmContext* context, GameState& state);
#endif