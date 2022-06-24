#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "world_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

#if DEVELOPMENT == 1
  static void removeObjectAtPosition(Globals, ObjectPool& objects, const Vec3f& position) {
    auto* object = queryObjectByPosition(globals, objects, position);

    if (object != nullptr) {
      remove(*object);
    }
  }

  static void removeStaticEntityObjectAtGridCoordinates(Globals, const GridCoordinates& coordinates) {
    auto& grid = state.world.grid;
    auto* entity = grid.get(coordinates);

    if (entity == nullptr) {
      return;
    }

    auto objectPosition = gridCoordinatesToWorldPosition(coordinates);
    Mesh* mesh;

    switch (entity->type) {
      default:
      case GROUND:
        mesh = mesh("ground-tile");
        break;
      case STAIRCASE:
        mesh = mesh("staircase");
        break;
      case WALKABLE_SPACE:
        mesh = mesh("entity-indicator");
        break;
    }

    removeObjectAtPosition(globals, mesh->objects, objectPosition);
  }

  static bool findStaticEntityPlacementCoordinates(Globals, GridCoordinates& coordinates) {
    auto& camera = getCamera();
    auto& grid = state.world.grid;
    float offset = TILE_SIZE * 2.f;
    bool canPlaceEntity = false;

    while (offset < 80.f) {
      auto ray = camera.position + camera.orientation.getDirection() * offset;
      auto testCoordinates = worldPositionToGridCoordinates(ray);

      if (
        grid.has(testCoordinates) &&
        grid.get(testCoordinates)->type == state.editor.currentSelectedEntityType
      ) {
        break;
      }

      canPlaceEntity = true;
      coordinates = testCoordinates;
      offset += HALF_TILE_SIZE / 4.f;
    }

    return canPlaceEntity;
  }

  static void storeEditAction(Globals, EditAction& action) {
    auto& editor = state.editor;

    if (editor.totalEditActions == MAX_EDIT_ACTIONS) {
      if (editor.editActions[0].oldEntity != nullptr) {
        // When shifting the first edit action off the queue,
        // we delete the copy of the old entity which existed
        // prior to that action, since the action can no
        // longer be undone.
        delete editor.editActions[0].oldEntity;
      }

      for (u8 i = 0; i < MAX_EDIT_ACTIONS - 1; i++) {
        editor.editActions[i] = editor.editActions[i + 1];
      }

      editor.totalEditActions--;
    }

    editor.editActions[editor.totalEditActions++] = action;
  }

  static StaticEntity* copyStaticEntity(const StaticEntity* source) {
    if (source == nullptr) {
      return nullptr;
    }

    StaticEntity* copy = nullptr;

    switch (source->type) {
      case GROUND:
        copy = new Ground;
        break;
      case STAIRCASE:
        copy = new Staircase((Staircase*)source);
        break;
      case WALKABLE_SPACE:
        copy = new WalkableSpace;
        break;
      default:
        break;
    }

    return copy;
  }

  void selectRangeFrom(Globals) {
    GridCoordinates rangeFrom = worldPositionToGridCoordinates(getCamera().position);

    findStaticEntityPlacementCoordinates(globals, rangeFrom);

    state.editor.rangeFrom = rangeFrom;
    state.editor.rangeFromSelected = true;

    // Hide the single tile placement preview
    // and ensure it isn't removed when placing
    // new entities over the range
    object("tile-preview").scale = 0.f;
    object("tile-preview").position += Vec3f(1.f);
    commit(object("tile-preview"));
  }

  void showStaticEntityPlacementPreview(Globals) {
    auto& preview = object("tile-preview");
    GridCoordinates previewCoordinates;

    if (findStaticEntityPlacementCoordinates(globals, previewCoordinates)) {
      auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

      // @todo use proper scale/color based on entity type
      preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.color = Vec3f(1.f, 0.7f, 0.3f);
    } else {
      preview.scale = 0.f;
    }

    commit(preview);
  }

  void showRangeFromSelectionPreview(GmContext* context, GameState& state) {
    auto& preview = object("tile-preview");
    GridCoordinates previewCoordinates;

    if (findStaticEntityPlacementCoordinates(globals, previewCoordinates)) {
      // @todo use proper color based on entity type
      const static auto defaultColor = Vec3f(1.f, 0.7f, 0.3f);
      const static auto fadeColor = Vec3f(0, 1.f, 0);
      auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

      #define sinewave(speed) sinf(getRunningTime() * speed)

      // @todo use proper scale based on entity type
      preview.scale = HALF_TILE_SIZE * (0.9f + sinewave(3.f) * 0.1f);
      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.color = Vec3f::lerp(defaultColor, fadeColor, sinewave(5.f) * 0.5f + 0.5f);
    } else {
      preview.scale = 0.f;
    }

    commit(preview);
  }

  void showRangedEntityPlacementPreview(Globals) {
    if (!state.editor.rangeFromSelected) {
      return;
    }

    auto start = state.editor.rangeFrom;
    GridCoordinates end = worldPositionToGridCoordinates(getCamera().position);

    findStaticEntityPlacementCoordinates(globals, end);

    if (end != state.editor.rangeTo) {
      state.editor.rangeTo = end;

      objects("range-preview").reset();

      checkRange(start, end);

      overRange(start, end, {
        auto& preview = createObjectFrom("range-preview");

        preview.scale = HALF_TILE_SIZE / 4.f;
        preview.color = Vec3f(0, 1.f, 0);
        preview.position = gridCoordinatesToWorldPosition({ x, y, z });

        commit(preview);
      });
    }

    for (auto& preview : objects("range-preview")) {
      auto baseSize = HALF_TILE_SIZE / 4.f;
      auto sizeRange = baseSize * 0.1f;

      preview.scale = baseSize + sinf(getRunningTime() * 3.f) * sizeRange;

      commit(preview);
    }
  }

  void tryPlacingStaticEntity(Globals) {
    GridCoordinates targetCoordinates;

    if (!findStaticEntityPlacementCoordinates(globals, targetCoordinates)) {
      return;
    }

    StaticEntity* entity;

    switch (state.editor.currentSelectedEntityType) {
      default:
      case GROUND:
        entity = new Ground;
        break;
      case STAIRCASE:
        entity = new Staircase;
        // @todo set staircase orientation
        break;
      case WALKABLE_SPACE:
        entity = new WalkableSpace;
        break;
    }

    auto& grid = state.world.grid;
    EditAction action;

    action.oldEntity = copyStaticEntity(grid.get(targetCoordinates));
    action.newEntity = entity;
    action.coordinates = targetCoordinates;

    removeStaticEntityObjectAtGridCoordinates(globals, targetCoordinates);
    grid.set(targetCoordinates, entity);
    createObjectFromCoordinates(globals, targetCoordinates);

    storeEditAction(globals, action);
  }

  void placeStaticEntitiesOverCurrentRange(Globals) {
    auto& grid = state.world.grid;
    auto start = state.editor.rangeFrom;
    auto end = state.editor.rangeTo;

    checkRange(start, end);

    EditAction action;

    action.isRangedPlacementAction = true;
    action.rangeFrom = start;
    action.rangeTo = end;

    overRange(start, end, {
      GridCoordinates coordinates = { x, y, z };

      removeStaticEntityObjectAtGridCoordinates(globals, coordinates);
      // @todo use entity corresponding to the current selected entity type
      grid.set(coordinates, new Ground);
      createObjectFromCoordinates(globals, coordinates);
    });

    storeEditAction(globals, action);

    objects("range-preview").reset();

    state.editor.rangeFromSelected = false;
  }

  void undoPreviousEditAction(Globals) {
    auto& editor = state.editor;

    if (editor.totalEditActions == 0) {
      return;
    }

    auto& grid = state.world.grid;
    auto& lastEditAction = editor.editActions[editor.totalEditActions - 1];

    if (lastEditAction.isRangedPlacementAction) {
      overRange(lastEditAction.rangeFrom, lastEditAction.rangeTo, {
        GridCoordinates coordinates = { x, y, z };

        removeStaticEntityObjectAtGridCoordinates(globals, coordinates);
        grid.clear(coordinates);
      });
    } else {
      auto& coordinates = lastEditAction.coordinates;
      auto* oldEntity = lastEditAction.oldEntity;

      // Undo the last entity/object placement
      removeStaticEntityObjectAtGridCoordinates(globals, coordinates);
      grid.clear(coordinates);

      if (oldEntity != nullptr) {
        // Restore the object/entity which existed before
        grid.set(coordinates, oldEntity);
        createObjectFromCoordinates(globals, coordinates);
      }
    }

    editor.totalEditActions--;
  }
#endif