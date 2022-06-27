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
        mesh = mesh("ground");
        break;
      case STAIRCASE:
        mesh = mesh("staircase");
        break;
    }

    removeObjectAtPosition(globals, mesh->objects, objectPosition);
  }

  static bool findStaticEntityPlacementCoordinates(Globals, GridCoordinates& coordinates) {
    auto& camera = getCamera();
    auto& editor = state.editor;
    auto& grid = state.world.grid;
    float offset = TILE_SIZE * 2.f;
    bool canPlaceEntity = false;

    while (offset < 80.f) {
      auto ray = camera.position + camera.orientation.getDirection() * offset;
      auto testCoordinates = worldPositionToGridCoordinates(ray);

      if (grid.has(testCoordinates)) {
        break;
      }

      canPlaceEntity = true;
      coordinates = testCoordinates;
      offset += HALF_TILE_SIZE / 4.f;
    }

    return canPlaceEntity;
  }

  static bool findStaticEntityDeletionCoordinates(Globals, GridCoordinates& coordinates) {
    auto& camera = getCamera();
    auto& editor = state.editor;
    auto& grid = state.world.grid;
    float offset = TILE_SIZE * 2.f;

    while (offset < 150.f) {
      auto ray = camera.position + camera.orientation.getDirection() * offset;
      auto testCoordinates = worldPositionToGridCoordinates(ray);

      if (grid.has(testCoordinates)) {
        coordinates = testCoordinates;
        return true;
      }

      offset += HALF_TILE_SIZE / 4.f;
    }

    return false;
  }

  static void storeEditAction(Globals, EditAction& action) {
    auto& editor = state.editor;

    if (editor.totalEditActions == MAX_EDIT_ACTIONS) {
      auto& firstEditAction = editor.editActions[0];

      // When shifting the first edit action off the queue,
      // delete the old entity/replaced entity copies, since
      // the action can no longer be undone, and those entities
      // cannot be restored
      if (firstEditAction.oldEntity != nullptr) {
        delete firstEditAction.oldEntity;
      } else if (firstEditAction.replacedEntityRecords.size() > 0) {
        for (auto& record : firstEditAction.replacedEntityRecords) {
          delete record.entity;
        }

        firstEditAction.replacedEntityRecords.clear();
      }

      // Move existing edit actions back one
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
      default:
        break;
    }

    return copy;
  }

  void setCurrentSelectedEntityType(Globals, StaticEntityType type) {
    state.editor.currentSelectedEntityType = type;
    state.editor.deleting = false;
    
    auto& existingPreview = object("placement-preview");
    auto existingPreviewPosition = existingPreview.position;
    auto existingPreviewRotation = existingPreview.rotation;
    std::string previewMeshName;

    remove(existingPreview);

    if (state.editor.deleting) {
      previewMeshName = "ground";
    } else {
      // @todo define a map for this
      if (type == GROUND) previewMeshName = "ground";
      if (type == STAIRCASE) previewMeshName = "staircase";
    }

    auto& preview = createObjectFrom(previewMeshName);

    preview.position = existingPreviewPosition;
    preview.rotation = existingPreviewRotation;

    save("placement-preview", preview);
  }

  // @todo switch roll/pitch depending on camera facing direction
  void adjustCurrentEntityOrientation(GmContext* context, GameState& state, const Orientation& adjustment) {
    state.editor.currentEntityOrientation += adjustment;
  }

  void selectRangeFrom(Globals) {
    GridCoordinates rangeFrom = worldPositionToGridCoordinates(getCamera().position);

    findStaticEntityPlacementCoordinates(globals, rangeFrom);

    state.editor.rangeFrom = rangeFrom;
    state.editor.rangeFromSelected = true;

    // Hide the single tile placement preview
    object("placement-preview").scale = 0.f;
    // Offset the placement preview object so it isn't
    // caught in ranged object replacement actions
    object("placement-preview").position += Vec3f(1.f);
    commit(object("placement-preview"));
  }

  void showStaticEntityPlacementPreview(Globals) {
    auto& preview = object("placement-preview");
    GridCoordinates previewCoordinates;

    preview.scale = 0.f;

    if (state.editor.deleting) {
      if (findStaticEntityDeletionCoordinates(globals, previewCoordinates)) {
        auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);
        auto alpha = sinf(getRunningTime() * 3.f) * 0.5f + 0.5f;

        preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
        preview.scale = HALF_TILE_SIZE * 1.1f + alpha * 0.2f;
        preview.color = Vec3f::lerp(Vec3f(1.f, 0, 0), Vec3f(0.7f, 0, 0), alpha);
      }
    } else {
      if (findStaticEntityPlacementCoordinates(globals, previewCoordinates)) {
        auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

        preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
        preview.rotation = Vec3f::lerp(preview.rotation, state.editor.currentEntityOrientation.toVec3f(), 0.5f);
        // @todo use proper scale/color based on entity type
        preview.scale = HALF_TILE_SIZE * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
        preview.color = Vec3f(1.f, 0.7f, 0.3f);
      }
    }

    commit(preview);
  }

  void showRangeFromSelectionPreview(GmContext* context, GameState& state) {
    auto& preview = object("placement-preview");
    GridCoordinates previewCoordinates;

    if (findStaticEntityPlacementCoordinates(globals, previewCoordinates)) {
      // @todo use proper color based on entity type
      const static auto defaultColor = Vec3f(1.f, 0.7f, 0.3f);
      auto fadeColor = state.editor.deleting ? Vec3f(1.f, 0, 0) : Vec3f(0, 1.f, 0);
      auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);

      #define sinewave(speed) sinf(getRunningTime() * speed)

      // @todo use proper scale based on entity type
      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.scale = HALF_TILE_SIZE * (0.9f + sinewave(3.f) * 0.1f);
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

        preview.position = gridCoordinatesToWorldPosition({ x, y, z });

        commit(preview);
      });
    }

    for (auto& preview : objects("range-preview")) {
      auto baseSize = HALF_TILE_SIZE / 4.f;
      auto sizeRange = baseSize * 0.1f;

      preview.scale = baseSize + sinf(getRunningTime() * 3.f) * sizeRange;
      preview.color = state.editor.deleting ? Vec3f(1.f, 0, 0) : Vec3f(0, 1.f, 0);

      commit(preview);
    }
  }

  void handleEditorSingleTileClickAction(Globals) {
    auto& grid = state.world.grid;
    GridCoordinates targetCoordinates;
    EditAction editAction;
    StaticEntity* newEntity = nullptr;

    if (state.editor.deleting) {
      if (!findStaticEntityDeletionCoordinates(globals, targetCoordinates)) {
        return;
      }

      // Offset the placement preview object so it isn't deleted
      object("placement-preview").position += Vec3f(1.f);
    } else {
      if (!findStaticEntityPlacementCoordinates(globals, targetCoordinates)) {
        return;
      }

      switch (state.editor.currentSelectedEntityType) {
        default:
        case GROUND:
          newEntity = new Ground;
          break;
        case STAIRCASE:
          newEntity = new Staircase;
          ((Staircase*)newEntity)->orientation = state.editor.currentEntityOrientation;
          break;
      }
    }

    editAction.oldEntity = copyStaticEntity(grid.get(targetCoordinates));
    editAction.coordinates = targetCoordinates;

    removeStaticEntityObjectAtGridCoordinates(globals, targetCoordinates);
    grid.clear(targetCoordinates);

    if (newEntity != nullptr) {
      grid.set(targetCoordinates, newEntity);
      createObjectFromCoordinates(globals, targetCoordinates);
    }

    storeEditAction(globals, editAction);

    context->renderer->resetShadowMaps();
  }

  void handleEditorRangedClickAction(Globals) {
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

      // Keep track of any replaced entities so we can potentially restore them
      if (grid.has(coordinates)) {
        action.replacedEntityRecords.push_back({
          coordinates,
          copyStaticEntity(grid.get(coordinates))
        });
      }

      removeStaticEntityObjectAtGridCoordinates(globals, coordinates);

      if (state.editor.deleting) {
        grid.clear(coordinates);
      } else {
        // @todo use entity corresponding to the current selected entity type
        grid.set(coordinates, new Ground);
        createObjectFromCoordinates(globals, coordinates);
      }
    });

    storeEditAction(globals, action);
    objects("range-preview").reset();

    state.editor.rangeFromSelected = false;

    context->renderer->resetShadowMaps();
  }

  void undoPreviousEditAction(Globals) {
    auto& editor = state.editor;

    if (editor.totalEditActions == 0) {
      return;
    }

    auto& grid = state.world.grid;
    auto& lastEditAction = editor.editActions[editor.totalEditActions - 1];

    if (lastEditAction.isRangedPlacementAction) {
      // Remove objects/entities placed over the range
      overRange(lastEditAction.rangeFrom, lastEditAction.rangeTo, {
        GridCoordinates coordinates = { x, y, z };

        removeStaticEntityObjectAtGridCoordinates(globals, coordinates);
        grid.clear(coordinates);
      });

      // Restore the entities/objects replaced by the edit action
      for (auto& record : lastEditAction.replacedEntityRecords) {
        grid.set(record.coordinates, record.entity);
        createObjectFromCoordinates(globals, record.coordinates);
      }

      lastEditAction.replacedEntityRecords.clear();
    } else {
      auto& coordinates = lastEditAction.coordinates;
      auto* oldEntity = lastEditAction.oldEntity;

      // Undo the last entity/object placement
      removeStaticEntityObjectAtGridCoordinates(globals, coordinates);
      grid.clear(coordinates);

      if (oldEntity != nullptr) {
        // Restore the entity/object which existed before
        grid.set(coordinates, oldEntity);
        createObjectFromCoordinates(globals, coordinates);
      }
    }

    editor.totalEditActions--;

    context->renderer->resetShadowMaps();
  }

  void placeCameraAtClosestWalkableTile(Globals) {
    auto& camera = getCamera();
    auto& editor = state.editor;
    auto& grid = state.world.grid;
    float offset = 0.f;

    while (offset < 500.f) {
      auto ray = camera.position + camera.orientation.getDirection() * offset;
      auto testCoordinates = worldPositionToGridCoordinates(ray);

      if (grid.has(testCoordinates)) {
        // @todo smoothly interpolate to new camera position
        camera.position = gridCoordinatesToWorldPosition(testCoordinates + GridCoordinates(0, 2, 0));

        break;
      }

      offset += HALF_TILE_SIZE;
    }
  }
#endif