#include <fstream>
#include <map>
#include <string>
#include <vector>

#include "Gamma.h"

#include "grid_utilities.h"
#include "editor_system.h"
#include "world_system.h"
#include "object_system.h"
#include "game_state.h"
#include "game_macros.h"
#include "build_flags.h"

using namespace Gamma;

// @todo derive these from game_world.cpp -> meshBuilders instead
static const std::vector<std::string> placeableMeshNames = {
  // Lunar Garden
  "dirt-floor",
  "dirt-wall",
  "water",
  "rock",
  "arch",
  "tulips",
  "grass",
  "hedge",
  "stone-tile",
  "rosebush",
  "gate-column",
  "gate",
  // Palace of the Moon
  "tile-1",
  "column"
};

#if DEVELOPMENT == 1
  static const std::map<std::string, Vec3f> meshPlacementOffsetMap = {
    { "dirt-floor", Vec3f(0, -HALF_TILE_SIZE, 0) },
    { "tile-1", Vec3f(0, -HALF_TILE_SIZE, 0) },
    { "rock", Vec3f(0, -HALF_TILE_SIZE, 0) },
    { "rosebush", Vec3f(0, -HALF_TILE_SIZE, 0) },
    { "column", Vec3f(0, HALF_TILE_SIZE, 0) }
  };

  static void removeObjectAtPosition(Globals, ObjectPool& objects, const Vec3f& position) {
    auto* object = findObjectByPosition(objects, position);

    if (object != nullptr) {
      removeObject(*object);
    }
  }

  static void removeGridEntityObjectAtGridCoordinates(Globals, const GridCoordinates& coordinates) {
    auto& grid = state.world.grid;
    auto* entity = grid.get(coordinates);

    if (entity == nullptr) {
      return;
    }

    auto objectPosition = gridCoordinatesToWorldPosition(coordinates);
    std::string meshName;

    // @todo define a map for this
    if (entity->type == GROUND) meshName = "ground";
    if (entity->type == STAIRCASE) meshName = "staircase";
    if (entity->type == SWITCH) meshName = "switch";
    if (entity->type == WORLD_ORIENTATION_CHANGE) meshName = "trigger-indicator";

    auto* mesh = mesh(meshName);

    removeObjectAtPosition(globals, mesh->objects, objectPosition);
  }

  static bool findGridEntityPlacementCoordinates(Globals, GridCoordinates& coordinates) {
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

  static bool findGridEntityDeletionCoordinates(Globals, GridCoordinates& coordinates) {
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

  static Object* findMeshObjectByDirection(Globals, const Vec3f& direction) {
    auto& camera = getCamera();
    float closestDistance = FLT_MAX;
    Object* found = nullptr;

    // @todo see if we need to optimize this once the number
    // of objects per mesh extends into the hundreds/thousands
    for (auto& meshName : placeableMeshNames) {
      for (auto& object : objects(meshName)) {
        auto cameraToObject = object.position - camera.position;
        auto objectDistance = cameraToObject.magnitude();
        auto unitCameraToObject = cameraToObject / objectDistance;
        auto dot = Vec3f::dot(unitCameraToObject, direction);
        auto angleRange = 1.f / (objectDistance + 1.f);

        if (
          dot > 1.f - angleRange &&
          objectDistance < closestDistance
        ) {
          closestDistance = objectDistance;
          found = &object;
          state.editor.currentMeshName = meshName;
        }
      }
    }

    return found;
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
          delete record.oldEntity;
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

  static GridEntity* copyGridEntity(const GridEntity* source) {
    if (source == nullptr) {
      return nullptr;
    }

    GridEntity* copy = nullptr;

    switch (source->type) {
      case GROUND:
        copy = new Ground;
        break;
      case STAIRCASE:
        copy = new Staircase((Staircase*)source);
        break;
      case SWITCH:
        copy = new Switch;
        break;
      case WORLD_ORIENTATION_CHANGE:
        copy = new WorldOrientationChange((WorldOrientationChange*)source);
        break;
      case TELEPORTER:
        copy = new Teleporter((Teleporter*)source);
      default:
        break;
    }

    return copy;
  }

  static void hideEntityPlacementPreview(Globals) {
    object("entity-preview").scale = 0.f;
    commit(object("entity-preview"));
  }

  static void stopPlacingMesh(Globals) {
    auto& editor = state.editor;

    removeObject(object("mesh-preview"));

    synchronizeCompoundMeshes(globals, editor.currentMeshName);

    editor.isPlacingMesh = false;
  }

  static void disableMeshFinder(Globals) {
    auto& editor = state.editor;
    auto* preview = findObject("mesh-preview");

    if (preview != nullptr) {
      // Commit the existing preview object, restoring its
      // color as set in showMeshFinderPreview()
      commit(*preview);
    }

    editor.isFindingMesh = false;
  }

  static void handleGridCellClickAction(Globals) {
    auto& grid = state.world.grid;
    auto& editor = state.editor;
    GridCoordinates targetCoordinates;
    EditAction editAction;
    GridEntity* newEntity = nullptr;

    if (editor.deleting) {
      if (!findGridEntityDeletionCoordinates(globals, targetCoordinates)) {
        return;
      }

      // Offset the placement preview object so it isn't deleted
      object("entity-preview").position += Vec3f(1.f);
    } else {
      if (!findGridEntityPlacementCoordinates(globals, targetCoordinates)) {
        return;
      }

      switch (editor.currentSelectedEntityType) {
        case GROUND:
          newEntity = new Ground;
          break;
        case STAIRCASE:
          newEntity = new Staircase;
          ((Staircase*)newEntity)->orientation = editor.currentEntityOrientation;
          break;
        case SWITCH:
          newEntity = new Switch;
          break;
        case WORLD_ORIENTATION_CHANGE:
          newEntity = new WorldOrientationChange;
          ((WorldOrientationChange*)newEntity)->targetWorldOrientation = editor.currentSelectedWorldOrientation;
          break;
        case TELEPORTER:
          newEntity = new Teleporter;
          ((Teleporter*)newEntity)->toCoordinates = editor.currentSelectedGridCoordinates;
          ((Teleporter*)newEntity)->toOrientation = editor.currentSelectedWorldOrientation;
        default:
          break;
      }
    }

    editAction.oldEntity = copyGridEntity(grid.get(targetCoordinates));
    editAction.coordinates = targetCoordinates;

    removeGridEntityObjectAtGridCoordinates(globals, targetCoordinates);
    grid.clear(targetCoordinates);

    if (newEntity != nullptr) {
      grid.set(targetCoordinates, newEntity);
      createGridObjectFromCoordinates(globals, targetCoordinates);
    }

    storeEditAction(globals, editAction);

    context->renderer->resetShadowMaps();
  }

  static void selectRangeFrom(Globals) {
    GridCoordinates rangeFrom = worldPositionToGridCoordinates(getCamera().position);

    findGridEntityPlacementCoordinates(globals, rangeFrom);

    state.editor.rangeFrom = rangeFrom;
    state.editor.rangeFromSelected = true;

    // Offset the placement preview object so it isn't
    // caught in ranged object replacement actions
    object("entity-preview").position += Vec3f(1.f);
    
    hideEntityPlacementPreview(globals);
  }

  static void handleRangedClickAction(Globals) {
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
          copyGridEntity(grid.get(coordinates))
        });
      }

      removeGridEntityObjectAtGridCoordinates(globals, coordinates);

      if (state.editor.deleting) {
        grid.clear(coordinates);
      } else {
        // @todo use entity corresponding to the current selected entity type
        grid.set(coordinates, new Ground);
        createGridObjectFromCoordinates(globals, coordinates);
      }
    });

    storeEditAction(globals, action);
    objects("range-preview").reset();

    state.editor.rangeFromSelected = false;

    context->renderer->resetShadowMaps();
  }

  static void handleMeshSelectionAction(Globals) {
    auto& camera =getCamera();
    auto& editor = state.editor;
    auto* object = findMeshObjectByDirection(globals, camera.orientation.getDirection());

    if (object != nullptr) {
      saveObject("mesh-preview", *object);
      // Commit the selected object, restoring its
      // color as set in showMeshFinderPreview()
      commit(*object);

      pointCameraAt(*object);

      editor.isFindingMesh = false;
      editor.isPlacingMesh = true;
      editor.selectedMeshDistance = (camera.position - object->position).magnitude();
    }
  }

  static void handleMeshPlacementAction(Globals) {
    if (object("mesh-preview").scale == 0.f) {
      return;
    }

    if (state.editor.snapMeshesToGrid) {
      // @todo check to see if any mesh objects exist at the
      // mesh preview position, and don't place it if so

      // Create a new mesh object to allow continual placement
      // until the editor is disabled
      createPlaceableMeshObjectFrom(globals, state.editor.currentMeshName);
    } else {
      // Stop moving the current preview mesh, setting it in place
      state.editor.isPlacingMesh = false;
      state.editor.enabled = false;
      state.editor.currentMeshName = "";
    }
  }

  static void resetLightIndicatorFocusStates(Globals) {
    auto& scene = context->scene;
    auto& lightIndicators = objects("light-indicator");
    u32 index = 0;

    for (u32 i = 0; i < scene.lights.size(); i++) {
      auto* light = scene.lights[i];

      if (light->serializable) {
        auto& indicator = lightIndicators[index++];

        indicator.color = light->color;
        indicator.scale = 1.5f;

        commit(indicator);
      }
    }
  }

  static void handleLightSelectionAction(Globals) {
    pointCameraAt(state.editor.selectedLight->position);

    state.editor.isFindingLight = false;
    state.editor.isPlacingLight = true;
  }

  static void handleLightPlacementAction(Globals) {
    state.editor.isFindingLight = true;
    state.editor.isPlacingLight = false;
  }

  static void handleLightDeletionAction(Globals) {
    auto& editor = state.editor;
    auto* indicator = findObjectByPosition(objects("light-indicator"), editor.selectedLight->position);

    if (indicator != nullptr) {
      removeObject(*indicator);
    }

    removeLight(editor.selectedLight);

    editor.selectedLight = nullptr;
    editor.isFindingLight = true;
    editor.isPlacingLight = false;

    saveLightData(globals);
  }

  void toggleEditor(Globals) {
    auto& editor = state.editor;

    editor.enabled = !editor.enabled;

    if (editor.enabled) {
      resetLightIndicatorFocusStates(globals);
    } else {
      hideEntityPlacementPreview(globals);

      if (editor.isPlacingMesh) {
        stopPlacingMesh(globals);
        saveMeshData(globals);
      }

      if (editor.isFindingMesh) {
        disableMeshFinder(globals);
      }

      // @todo disableLightPlacement()
      editor.isPlacingLight = false;
      editor.isFindingLight = false;

      context->renderer->resetShadowMaps();
    }
  }

  void toggleMeshFinder(Globals) {
    auto& editor = state.editor;

    editor.isFindingMesh = !editor.isFindingMesh;
    editor.enabled = editor.isFindingMesh;

    if (editor.isPlacingMesh) {
      stopPlacingMesh(globals);
    }

    if (!editor.isFindingMesh) {
      disableMeshFinder(globals);
    }

    hideEntityPlacementPreview(globals);

    // @todo disableLightPlacement()
    editor.isPlacingLight = false;
    editor.isFindingLight = false;
  }
  
  void toggleLightFinder(Globals) {
    auto& editor = state.editor;

    editor.isFindingLight = !editor.isFindingLight;
    editor.enabled = editor.isFindingLight;

    hideEntityPlacementPreview(globals);

    // @todo disableMeshPlacement()
    if (editor.isPlacingMesh) {
      stopPlacingMesh(globals);
    }

    if (editor.isFindingMesh) {
      disableMeshFinder(globals);
    }
  }

  void setCurrentSelectedEntityType(Globals, EntityType type) {
    state.editor.currentSelectedEntityType = type;
    state.editor.deleting = false;
    
    auto& existingPreview = object("entity-preview");
    auto existingPreviewPosition = existingPreview.position;
    auto existingPreviewRotation = existingPreview.rotation;
    std::string previewMeshName;

    removeObject(existingPreview);

    // @todo define a map for this
    if (type == GROUND) previewMeshName = "ground";
    if (type == STAIRCASE) previewMeshName = "staircase";
    if (type == SWITCH) previewMeshName = "switch";
    if (type == WORLD_ORIENTATION_CHANGE) previewMeshName = "trigger-indicator";
    if (type == TELEPORTER) previewMeshName = "trigger-indicator";

    auto& preview = createObjectFrom(previewMeshName);

    preview.position = existingPreviewPosition;
    preview.rotation = existingPreviewRotation;

    saveObject("entity-preview", preview);
  }

  // @todo switch roll/pitch depending on camera facing direction
  void adjustCurrentEntityOrientation(GmContext* context, GameState& state, const Orientation& adjustment) {
    state.editor.currentEntityOrientation += adjustment;
  }

  void showGridEntityPlacementPreview(Globals) {
    auto& preview = object("entity-preview");
    GridCoordinates previewCoordinates;

    preview.scale = 0.f;

    if (state.editor.deleting) {
      if (findGridEntityDeletionCoordinates(globals, previewCoordinates)) {
        auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);
        auto alpha = sinf(getRunningTime() * 3.f) * 0.5f + 0.5f;

        preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
        preview.scale = HALF_TILE_SIZE * 1.1f + alpha * 0.2f;
        preview.color = Vec3f::lerp(Vec3f(1.f, 0, 0), Vec3f(0.7f, 0, 0), alpha);
      }
    } else {
      if (findGridEntityPlacementCoordinates(globals, previewCoordinates)) {
        auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);
        auto& params = getGridObjectParameters(state.editor.currentSelectedEntityType);

        preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
        preview.rotation = Vec3f::lerp(preview.rotation, state.editor.currentEntityOrientation.toVec3f(), 0.5f);
        preview.scale = params.scale * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
        preview.color = params.color;
      }
    }

    commit(preview);
  }

  void showRangeFromSelectionPreview(GmContext* context, GameState& state) {
    auto& preview = object("entity-preview");
    GridCoordinates previewCoordinates;

    if (findGridEntityPlacementCoordinates(globals, previewCoordinates)) {
      auto fadeColor = state.editor.deleting ? Vec3f(1.f, 0, 0) : Vec3f(0, 1.f, 0);
      auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);
      auto& params = getGridObjectParameters(state.editor.currentSelectedEntityType);

      #define sinewave(speed) sinf(getRunningTime() * speed)

      preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
      preview.scale = params.scale * (0.9f + sinewave(3.f) * 0.1f);
      preview.color = Vec3f::lerp(params.color, fadeColor, sinewave(5.f) * 0.5f + 0.5f);
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

    findGridEntityPlacementCoordinates(globals, end);

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

  void showMeshPlacementPreview(Globals) {
    auto& editor = state.editor;
    auto& camera = getCamera();
    auto& preview = object("mesh-preview");

    if (editor.snapMeshesToGrid) {
      auto placementRay = camera.position + camera.orientation.getDirection() * TILE_SIZE * 4.f;
      auto targetMeshPosition = getGridAlignedWorldPosition(placementRay);

      if (meshPlacementOffsetMap.find(editor.currentMeshName) != meshPlacementOffsetMap.end()) {
        targetMeshPosition += meshPlacementOffsetMap.at(editor.currentMeshName);
      }

      preview.position = Vec3f::lerp(preview.position, targetMeshPosition, 0.5f);
    } else {
      preview.position = camera.position + camera.orientation.getDirection() * editor.selectedMeshDistance;
    }

    commit(preview);

    synchronizeCompoundMeshes(globals, editor.currentMeshName);
  }

  void showMeshFinderPreview(Globals) {
    #define FOCUS_COLOR Vec3f(1.f, 0, 0)

    auto& editor = state.editor;
    auto& camera = getCamera();
    auto* previousPreview = findObject("mesh-preview");
    auto* targetPreview = findMeshObjectByDirection(globals, camera.orientation.getDirection());

    if (previousPreview != nullptr) {
      // Commit the restored color state (see below)
      commit(*previousPreview);
    }

    if (targetPreview != nullptr) {
      auto originalColor = targetPreview->color;

      // Temporarily set and commit the preview object color to
      // a given focus color
      targetPreview->color = FOCUS_COLOR;

      commit(*targetPreview);
      saveObject("mesh-preview", *targetPreview);

      // Set the preview object color back to its original value,
      // but don't commit the change. We want its rendered color
      // to be the focus color, but we want to be able to commit
      // it again on a subsequent frame and restore its state.
      targetPreview->color = originalColor;
    }
  }

  void createPlaceableMeshObjectFrom(Globals, const std::string& meshName) {
    if (Gm_VectorContains(placeableMeshNames, meshName)) {
      auto& editor = state.editor;
      Vec3f defaultRotation = Vec3f(0.f);

      if (editor.isPlacingMesh) {
        auto& existingPreview = object("mesh-preview");

        if (meshName == editor.currentMeshName) {
          // Preserve existing rotation when spawning more of the same mesh
          defaultRotation = existingPreview.rotation;
        } else {
          // Remove the existing mesh preview when swapping it for a different mesh
          removeObject(existingPreview);
        }
      }

      editor.enabled = true;
      editor.currentMeshName = meshName;
      editor.isPlacingMesh = true;
      editor.snapMeshesToGrid = true;
      editor.selectedMeshDistance = TILE_SIZE * 2.f;

      editor.isPlacingLight = false;
      editor.isFindingLight = false;

      auto& object = createMeshObject(globals, meshName);

      object.rotation = defaultRotation;

      saveObject("mesh-preview", object);

      hideEntityPlacementPreview(globals);
    }
  }

  void showLightPlacementPreview(Globals) {
    auto& editor = state.editor;
    auto& camera = getCamera();
    auto& lightIndicators = objects("light-indicator");

    if (editor.selectedLight != nullptr) {
      auto* indicator = findObjectByPosition(lightIndicators, editor.selectedLight->position);

      editor.selectedLight->position = camera.position + camera.orientation.getDirection() * editor.selectedLightDistance;

      if (indicator != nullptr) {
        indicator->position = editor.selectedLight->position;
        indicator->color = editor.selectedLight->color;

        commit(*indicator);
      }
    }
  }

  void showLightFinderPreview(Globals) {
    #define MAX_LIGHT_FINDER_DISTANCE 100.f

    auto& editor = state.editor;
    auto& scene = context->scene;
    auto& camera = getCamera();
    auto& cameraDirection = camera.orientation.getDirection();
    auto closestDistance = MAX_LIGHT_FINDER_DISTANCE;
    auto& lightIndicators = objects("light-indicator");

    // Reset selected light values
    editor.selectedLight = nullptr;
    editor.selectedLightDistance = 0.f;

    resetLightIndicatorFocusStates(globals);

    // Find a point/spot light by camera direction
    for (auto* light : scene.lights) {
      if (light->serializable) {
        auto cameraToLight = light->position - camera.position;
        auto lightDistance = cameraToLight.magnitude();
        auto unitCameraToObject = cameraToLight / lightDistance;
        auto dot = Vec3f::dot(unitCameraToObject, cameraDirection);
        auto angleRange = 1.f / (lightDistance + 1.f);

        if (
          dot > 1.f - angleRange &&
          lightDistance < closestDistance
        ) {
          closestDistance = lightDistance;

          editor.selectedLight = light;
          editor.selectedLightDistance = lightDistance;
        }
      }
    }

    // Highlight the selected light's corresponding indicator
    if (editor.selectedLight != nullptr) {
      auto* indicator = findObjectByPosition(lightIndicators, editor.selectedLight->position);

      if (indicator != nullptr) {
        indicator->color = Vec3f(1.f, 0, 0);
        indicator->scale = 2.f;

        commit(*indicator);
      }
    }
  }

  void createPlaceableLight(Globals, Gamma::LightType lightType) {
    auto& editor = state.editor;
    auto& camera = getCamera();
    auto& light = createLight(lightType);

    light.position = camera.position + camera.orientation.getDirection() * 40.f;

    auto& indicator = createObjectFrom("light-indicator");

    indicator.scale = 1.5f;
    indicator.position = light.position;

    commit(indicator);

    editor.selectedLight = &light;
    editor.selectedLightDistance = TILE_SIZE * 2.f;

    editor.enabled = true;
    editor.isPlacingLight = true;
    editor.isFindingLight = false;
  }

  void handleEditorClickAction(Globals) {
    auto& editor = state.editor;

    if (editor.isFindingLight) {
      handleLightSelectionAction(globals);
    } else if (editor.isPlacingLight) {
      handleLightPlacementAction(globals);
      saveLightData(globals);
    } else if (editor.isFindingMesh) {
      handleMeshSelectionAction(globals);
    } else if (editor.isPlacingMesh) {
      handleMeshPlacementAction(globals);
      saveMeshData(globals);
    } else if (editor.useRange) {
      if (editor.rangeFromSelected) {
        handleRangedClickAction(globals);
        saveWorldGridData(globals);
      } else {
        selectRangeFrom(globals);
      }
    } else {
      handleGridCellClickAction(globals);
      saveWorldGridData(globals);
    }
  }

  void handleEditorDeletionAction(Globals) {
    auto& editor = state.editor;

    if (editor.isPlacingMesh) {
      stopPlacingMesh(globals);
      saveMeshData(globals);

      editor.isFindingMesh = true;
    } else if (editor.isPlacingLight) {
      handleLightDeletionAction(globals);
      saveLightData(globals);
    }
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

        removeGridEntityObjectAtGridCoordinates(globals, coordinates);
        grid.clear(coordinates);
      });

      // Restore the entities/objects replaced by the edit action
      for (auto& record : lastEditAction.replacedEntityRecords) {
        grid.set(record.coordinates, record.oldEntity);
        createGridObjectFromCoordinates(globals, record.coordinates);
      }

      lastEditAction.replacedEntityRecords.clear();
    } else {
      auto& coordinates = lastEditAction.coordinates;
      auto* oldEntity = lastEditAction.oldEntity;

      // Undo the last entity/object placement
      removeGridEntityObjectAtGridCoordinates(globals, coordinates);
      grid.clear(coordinates);

      if (oldEntity != nullptr) {
        // Restore the entity/object which existed before
        grid.set(coordinates, oldEntity);
        createGridObjectFromCoordinates(globals, coordinates);
      }
    }

    editor.totalEditActions--;

    context->renderer->resetShadowMaps();
  }

  void placeCameraAtClosestWalkableTile(Globals) {
    auto& camera = getCamera();
    auto& editor = state.editor;
    auto& grid = state.world.grid;
    auto upGridCoordinates = getUpGridCoordinates(state.worldOrientationState.worldOrientation);
    float offset = 0.f;

    while (offset < 500.f) {
      auto ray = camera.position + camera.orientation.getDirection() * offset;
      auto testCoordinates = worldPositionToGridCoordinates(ray);

      if (grid.has(testCoordinates)) {
        state.cameraStartPosition = camera.position;
        state.cameraTargetPosition = gridCoordinatesToWorldPosition(testCoordinates + upGridCoordinates + upGridCoordinates);
        state.cameraTargetStartTime = getRunningTime();

        break;
      }

      offset += HALF_TILE_SIZE;
    }
  }

  #define serialize3Vector(value) std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z)

  // Wrap a value to within the [0, TAU] range
  #define wrap(n) n = n > Gm_TAU ? n - Gm_TAU : n < 0.f ? n + Gm_TAU : n

  void saveWorldGridData(Globals) {
    std::string serialized;

    serialized += "ground\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == GROUND) {
        serialized += serialize3Vector(coordinates) + "\n";
      }
    }

    serialized += "staircase\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == STAIRCASE) {
        auto orientationAsVec3f = ((Staircase*)entity)->orientation.toVec3f();

        wrap(orientationAsVec3f.x);
        wrap(orientationAsVec3f.y);
        wrap(orientationAsVec3f.z);

        serialized += serialize3Vector(coordinates) + ",";
        serialized += serialize3Vector(orientationAsVec3f) + "\n";
      }
    }

    serialized += "switch\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == SWITCH) {
        serialized += serialize3Vector(coordinates) + "\n";
      }
    }

    serialized += "woc\n";

    // @todo define in orientation_system
    // @todo use in game_update -> addDebugMessages
    static std::map<WorldOrientation, std::string> worldOrientationToString = {
      { POSITIVE_Y_UP, "+Y" },
      { NEGATIVE_Y_UP, "-Y" },
      { POSITIVE_X_UP, "+X" },
      { NEGATIVE_X_UP, "-X" },
      { POSITIVE_Z_UP, "+Z" },
      { NEGATIVE_Z_UP, "-Z" }
    };

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == WORLD_ORIENTATION_CHANGE) {
        auto worldOrientation = ((WorldOrientationChange*)entity)->targetWorldOrientation;

        serialized += serialize3Vector(coordinates) + ",";
        serialized += worldOrientationToString[worldOrientation] + "\n";
      }
    }

    serialized += "teleporter\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == TELEPORTER) {
        auto toCoordinates = ((Teleporter*)entity)->toCoordinates;
        auto toOrientation = ((Teleporter*)entity)->toOrientation;

        serialized += serialize3Vector(coordinates) + ",";
        serialized += serialize3Vector(toCoordinates) + ",";
        serialized += worldOrientationToString[toOrientation] + "\n";
      }
    }

    Gm_WriteFileContents("./game/world/grid_data.txt", serialized);
  }

  void saveMeshData(Globals) {
    std::stringstream serialized;
    auto* previewMesh = findObject("mesh-preview");

    for (auto& meshName : placeableMeshNames) {
      serialized << meshName + "\n";

      for (auto& object : objects(meshName)) {
        if (&object != previewMesh || !state.editor.isPlacingMesh) {
          auto& p = object.position;
          auto& r = object.rotation;
          auto& s = object.scale;
          auto& c = object.color;

          serialized << p.x << "," << p.y << "," << p.z << ",";
          serialized << r.x << "," << r.y << "," << r.z << ",";
          serialized << (u32)c.r << "," << (u32)c.g << "," << (u32)c.b << ",";
          // Only serialize a single scale component, assuming uniform scaling
          serialized << s.x << "\n";
        }
      }
    }

    Gm_WriteFileContents("./game/world/mesh_data.txt", serialized.str());
  }

  void saveLightData(Globals) {
    std::stringstream serialized;
    auto& scene = context->scene;

    serialized << "point\n";

    for (auto* light : scene.lights) {
      if (light->serializable) {
        auto& p = light->position;
        auto& c = light->color;
        auto radius = light->radius;

        serialized << p.x << "," << p.y << "," << p.z << ",";
        serialized << c.x << "," << c.y << "," << c.z << ",";
        serialized << radius << "\n";
      }
    }

    Gm_WriteFileContents("./game/world/light_data.txt", serialized.str());
  }
#endif

// @todo move everything below to world_system
static void createObjectFromData(Globals, const std::vector<std::string>& data, const std::string& meshName) {
  Vec3f position = {
    stof(data[0]),
    stof(data[1]),
    stof(data[2])
  };

  Vec3f rotation = {
    stof(data[3]),
    stof(data[4]),
    stof(data[5])
  };

  pVec4 color = {
    (u8)stoi(data[6]),
    (u8)stoi(data[7]),
    (u8)stoi(data[8])
  };

  Vec3f scale = stof(data[9]);

  auto& object = createMeshObject(globals, meshName);

  if (meshName == "rosebush") {
    // @todo manage this deterministically
    rotation.y = Gm_Random(0.f, Gm_TAU);
  }

  object.position = position;
  object.rotation = rotation;
  object.color = color;
  object.scale = scale;

  commit(object);
}

void loadWorldGridData(Globals) {
  auto& grid = state.world.grid;
  EntityType currentEntityType;
  std::ifstream file("./game/world/grid_data.txt");

  if (file.fail()) {
    return;
  }

  defer(file.close());

  std::string line;

  // @todo define in orientation_system
  static std::map<std::string, WorldOrientation> stringToWorldOrientation = {
    { "+Y", POSITIVE_Y_UP },
    { "-Y", NEGATIVE_Y_UP },
    { "+X", POSITIVE_X_UP },
    { "-X", NEGATIVE_X_UP },
    { "+Z", POSITIVE_Z_UP },
    { "-Z", NEGATIVE_Z_UP }
  };

  while (std::getline(file, line)) {
    if (line == "ground") {
      currentEntityType = GROUND;
    } else if (line == "staircase") {
      currentEntityType = STAIRCASE;
    } else if (line == "switch") {
      currentEntityType = SWITCH;
    } else if (line == "woc") {
      currentEntityType = WORLD_ORIENTATION_CHANGE;
    } else if (line == "teleporter") {
      currentEntityType = TELEPORTER;
    } else {
      auto data = Gm_SplitString(line, ",");
      auto x = (s16)stoi(data[0]);
      auto y = (s16)stoi(data[1]);
      auto z = (s16)stoi(data[2]);

      switch (currentEntityType) {
        case GROUND:
          grid.set({ x, y, z }, new Ground);
          break;
        case STAIRCASE: {
          auto pitch = stof(data[3]);
          auto yaw = stof(data[4]);
          auto roll = stof(data[5]);
          auto* staircase = new Staircase;

          staircase->orientation = { roll, pitch, yaw };

          grid.set({ x, y, z }, staircase);
          break;
        }
        case SWITCH:
          grid.set({ x, y, z }, new Switch);
          break;
        case WORLD_ORIENTATION_CHANGE: {
          auto worldOrientation = stringToWorldOrientation[data[3]];
          auto* worldOrientationChange = new WorldOrientationChange;

          worldOrientationChange->targetWorldOrientation = worldOrientation;

          grid.set({ x, y, z }, worldOrientationChange);
          break;
        }
        case TELEPORTER: {
          auto* teleporter = new Teleporter;

          teleporter->toCoordinates = {
            (s16)stoi(data[3]),
            (s16)stoi(data[4]),
            (s16)stoi(data[5])
          };

          teleporter->toOrientation = stringToWorldOrientation[data[6]];

          grid.set({ x, y, z }, teleporter);
          break;
        }
      }
    }
  }
}

void loadMeshData(Globals) {
  std::ifstream file("./game/world/mesh_data.txt");

  if (file.fail()) {
    return;
  }

  defer(file.close());

  std::string line;
  std::string currentMeshName;

  while (std::getline(file, line)) {
    if (Gm_VectorContains(placeableMeshNames, line)) {
      currentMeshName = line;
    } else {
      auto data = Gm_SplitString(line, ",");

      createObjectFromData(globals, data, currentMeshName);
    }
  }

  synchronizeCompoundMeshes(globals);
}

void loadStaticStructureData(Globals) {
  std::ifstream file("./game/world/static_structure_data.txt");

  if (file.fail()) {
    return;
  }

  defer(file.close());

  std::string line;
  std::string currentMeshName;

  while (std::getline(file, line)) {
    if (line[0] == '@') {
      currentMeshName = line.substr(1);
    } else {
      auto data = Gm_SplitString(line, ",");

      createObjectFromData(globals, data, currentMeshName);
    }
  }
}

void loadLightData(Globals) {
  std::ifstream file("./game/world/light_data.txt");

  if (file.fail()) {
    return;
  }

  defer(file.close());

  std::string line;
  LightType currentLightType;

  while (std::getline(file, line)) {
    if (line == "point") {
      currentLightType = LightType::POINT;
    } else {
      auto data = Gm_SplitString(line, ",");
      auto& light = createLight(currentLightType);

      light.position = {
        stof(data[0]),
        stof(data[1]),
        stof(data[2])
      };

      light.color = {
        stof(data[3]),
        stof(data[4]),
        stof(data[5])
      };

      light.radius = stof(data[6]);
    }
  }
}