#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "world_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

#if DEVELOPMENT == 1
  static void removeObjectAtPosition(args(), ObjectPool& objects, const Vec3f& position) {
    auto* object = queryObjectByPosition(params(), objects, position);

    if (object != nullptr) {
      remove(*object);
    }
  }

  static void removeStaticEntityObjectAtGridCoordinates(args(), const GridCoordinates& coordinates) {
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

    removeObjectAtPosition(params(), mesh->objects, objectPosition);
  }

  static bool findStaticEntityPlacementCoordinates(args(), GridCoordinates& coordinates) {
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

  static void storeEditAction(args(), EditAction& action) {
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

  void showStaticEntityPlacementPreview(args()) {
    auto& preview = object("preview");
    GridCoordinates previewCoordinates;

    if (findStaticEntityPlacementCoordinates(params(), previewCoordinates)) {
      Vec3f targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

      // @todo use proper scale/color based on entity type
      preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.color = Vec3f(1.f, 0.7f, 0.3f);
    } else {
      preview.scale = 0.f;
    }

    commit(preview);
  }

  void showRangedEntityPlacementPreview(args()) {
    if (!state.editor.rangeFromSelected) {
      return;
    }

    auto start = state.editor.rangeFrom;
    GridCoordinates end = worldPositionToGridCoordinates(getCamera().position);

    findStaticEntityPlacementCoordinates(params(), end);

    if (end != state.editor.rangeTo) {
      state.editor.rangeTo = end;

      objects("range-preview").reset();

      // @todo define a helper for this
      if (start.x > end.x) {
        std::swap(start.x, end.x);
      }

      if (start.y > end.y) {
        std::swap(start.y, end.y);
      }

      if (start.z > end.z) {
        std::swap(start.z, end.z);
      }

      // @todo define a macro/helper for this
      for (s16 x = start.x; x <= end.x; x++) {
        for (s16 y = start.y; y <= end.y; y++) {
          for (s16 z = start.z; z <= end.z; z++) {
            auto& preview = createObjectFrom("range-preview");

            preview.scale = HALF_TILE_SIZE / 4.f;
            preview.color = Vec3f(0, 1.f, 0);
            preview.position = gridCoordinatesToWorldPosition({ x, y, z });

            commit(preview);
          }
        }
      }
    }
  }

  void tryPlacingStaticEntity(args()) {
    GridCoordinates targetCoordinates;

    if (!findStaticEntityPlacementCoordinates(params(), targetCoordinates)) {
      return;
    }

    auto& grid = state.world.grid;
    EditAction action;
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

    action.oldEntity = copyStaticEntity(grid.get(targetCoordinates));
    action.newEntity = entity;
    action.coordinates = targetCoordinates;

    removeStaticEntityObjectAtGridCoordinates(params(), targetCoordinates);

    grid.set(targetCoordinates, entity);
    createObjectFromCoordinates(params(), targetCoordinates);

    storeEditAction(params(), action);
  }

  void selectRangeFrom(args()) {
    GridCoordinates rangeFrom = worldPositionToGridCoordinates(getCamera().position);

    findStaticEntityPlacementCoordinates(params(), rangeFrom);

    state.editor.rangeFrom = rangeFrom;
    state.editor.rangeFromSelected = true;
  }

  void fillStaticEntitiesWithinCurrentRange(args()) {
    auto& grid = state.world.grid;
    auto start = state.editor.rangeFrom;
    auto end = state.editor.rangeTo;

    // @todo define a helper for this
    if (start.x > end.x) {
      std::swap(start.x, end.x);
    }

    if (start.y > end.y) {
      std::swap(start.y, end.y);
    }

    if (start.z > end.z) {
      std::swap(start.z, end.z);
    }

    // @todo use entity corresponding to the current selected entity type
    setStaticEntityOverRange<Ground>(params(), start, end);

    // @todo define a macro/helper for this
    for (s16 x = start.x; x <= end.x; x++) {
      for (s16 y = start.y; y <= end.y; y++) {
        for (s16 z = start.z; z <= end.z; z++) {
          createObjectFromCoordinates(params(), { x, y, z });
        }
      }
    }

    state.editor.rangeFromSelected = false;
  }

  void undoPreviousEditAction(args()) {
    auto& editor = state.editor;

    if (editor.totalEditActions == 0) {
      return;
    }

    auto& grid = state.world.grid;
    auto& lastEditAction = editor.editActions[editor.totalEditActions - 1];
    auto& coordinates = lastEditAction.coordinates;
    auto* oldEntity = lastEditAction.oldEntity;

    // Undo the last entity/object placement
    removeStaticEntityObjectAtGridCoordinates(params(), coordinates);
    grid.clear(coordinates);

    if (oldEntity != nullptr) {
      // Restore the object/entity which existed before
      grid.set(coordinates, oldEntity);
      createObjectFromCoordinates(params(), coordinates);
    }

    editor.totalEditActions--;
  }
#endif