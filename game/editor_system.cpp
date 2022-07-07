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

#if DEVELOPMENT == 1
  static const std::vector<std::string> meshNames = {
    "dirt-floor",
    "rock",
    "arch",
    "hedge"
  };

  static const std::map<std::string, Vec3f> meshPlacementOffsetMap = {
    { "dirt-floor", Vec3f(0, -HALF_TILE_SIZE, 0) },
    { "rock", Vec3f(0, -HALF_TILE_SIZE, 0) }
  };

  static void removeObjectAtPosition(Globals, ObjectPool& objects, const Vec3f& position) {
    auto* object = queryObjectByPosition(globals, objects, position);

    if (object != nullptr) {
      remove(*object);
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
    for (auto& meshName : meshNames) {
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
      default:
        break;
    }

    return copy;
  }

  void setCurrentSelectedEntityType(Globals, EntityType type) {
    state.editor.currentSelectedEntityType = type;
    state.editor.deleting = false;
    
    auto& existingPreview = object("placement-preview");
    auto existingPreviewPosition = existingPreview.position;
    auto existingPreviewRotation = existingPreview.rotation;
    std::string previewMeshName;

    remove(existingPreview);

    // @todo define a map for this
    if (type == GROUND) previewMeshName = "ground";
    if (type == STAIRCASE) previewMeshName = "staircase";
    if (type == SWITCH) previewMeshName = "switch";
    if (type == WORLD_ORIENTATION_CHANGE) previewMeshName = "trigger-indicator";

    auto& preview = createObjectFrom(previewMeshName);

    preview.position = existingPreviewPosition;
    preview.rotation = existingPreviewRotation;

    saveObject("placement-preview", preview);
  }

  void createPlaceableMeshObjectFrom(Globals, const std::string& meshName) {
    auto& meshMap = context->scene.meshMap;

    if (meshMap.find(meshName) != meshMap.end()) {
      // Remove the existing mesh preview when swapping it for a different mesh
      if (
        state.editor.isPlacingMesh &&
        meshName != state.editor.currentMeshName
      ) {
        remove(object("mesh-preview"));
      }

      state.editor.enabled = true;
      state.editor.currentMeshName = meshName;
      state.editor.isPlacingMesh = true;
      state.editor.snapMeshesToGrid = true;

      auto& object = createObjectFrom(meshName);

      object.scale = HALF_TILE_SIZE;

      saveObject("mesh-preview", object);

      object("placement-preview").scale = 0.f;
      commit(object("placement-preview"));
    }
  }

  // @todo switch roll/pitch depending on camera facing direction
  void adjustCurrentEntityOrientation(GmContext* context, GameState& state, const Orientation& adjustment) {
    state.editor.currentEntityOrientation += adjustment;
  }

  void selectRangeFrom(Globals) {
    GridCoordinates rangeFrom = worldPositionToGridCoordinates(getCamera().position);

    findGridEntityPlacementCoordinates(globals, rangeFrom);

    state.editor.rangeFrom = rangeFrom;
    state.editor.rangeFromSelected = true;

    // Hide the single tile placement preview
    object("placement-preview").scale = 0.f;
    // Offset the placement preview object so it isn't
    // caught in ranged object replacement actions
    object("placement-preview").position += Vec3f(1.f);
    commit(object("placement-preview"));
  }

  void showGridEntityPlacementPreview(Globals) {
    auto& preview = object("placement-preview");
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
        auto& params = getObjectParameters(state.editor.currentSelectedEntityType);

        preview.position = Vec3f::lerp(preview.position, targetPosition, 0.5f);
        preview.rotation = Vec3f::lerp(preview.rotation, state.editor.currentEntityOrientation.toVec3f(), 0.5f);
        preview.scale = params.scale * (0.9f + sinf(getRunningTime() * 3.f) * 0.1f);
        preview.color = params.color;
      }
    }

    commit(preview);
  }

  void showRangeFromSelectionPreview(GmContext* context, GameState& state) {
    auto& preview = object("placement-preview");
    GridCoordinates previewCoordinates;

    if (findGridEntityPlacementCoordinates(globals, previewCoordinates)) {
      auto fadeColor = state.editor.deleting ? Vec3f(1.f, 0, 0) : Vec3f(0, 1.f, 0);
      auto targetPosition = gridCoordinatesToWorldPosition(previewCoordinates);
      auto& params = getObjectParameters(state.editor.currentSelectedEntityType);

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
    auto& camera = getCamera();
    auto& preview = object("mesh-preview");
    auto& editor = state.editor;
    auto placementPosition = camera.position + camera.orientation.getDirection() * TILE_SIZE * 4.f;

    if (editor.snapMeshesToGrid) {
      auto gridCoordinates = worldPositionToGridCoordinates(placementPosition);
      auto targetMeshPosition = gridCoordinatesToWorldPosition(gridCoordinates);

      if (meshPlacementOffsetMap.find(editor.currentMeshName) != meshPlacementOffsetMap.end()) {
        targetMeshPosition += meshPlacementOffsetMap.at(editor.currentMeshName);
      }

      preview.position = Vec3f::lerp(preview.position, targetMeshPosition, 0.5f);
    } else {
      preview.position = placementPosition;
    }

    commit(preview);
  }

  void showMeshFinderPreview(Globals) {
    auto& camera = getCamera();
    auto* object = findMeshObjectByDirection(globals, camera.orientation.getDirection());

    // Reset the mesh preview highlight color
    if (hasObject("mesh-preview")) {
      object("mesh-preview").color = Vec3f(1.f);

      commit(object("mesh-preview"));
    }

    if (object != nullptr) {
      object->color = Vec3f(1.f, 0, 0);

      commit(*object);
      saveObject("mesh-preview", *object);
    }
  }

  void handleEditorSingleTileClickAction(Globals) {
    auto& grid = state.world.grid;
    GridCoordinates targetCoordinates;
    EditAction editAction;
    GridEntity* newEntity = nullptr;

    if (state.editor.deleting) {
      if (!findGridEntityDeletionCoordinates(globals, targetCoordinates)) {
        return;
      }

      // Offset the placement preview object so it isn't deleted
      object("placement-preview").position += Vec3f(1.f);
    } else {
      if (!findGridEntityPlacementCoordinates(globals, targetCoordinates)) {
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
        case SWITCH:
          newEntity = new Switch;
          break;
        case WORLD_ORIENTATION_CHANGE:
          newEntity = new WorldOrientationChange;
          ((WorldOrientationChange*)newEntity)->targetWorldOrientation = state.editor.currentSelectedWorldOrientation;
          break;
      }
    }

    editAction.oldEntity = copyGridEntity(grid.get(targetCoordinates));
    editAction.coordinates = targetCoordinates;

    removeGridEntityObjectAtGridCoordinates(globals, targetCoordinates);
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
          copyGridEntity(grid.get(coordinates))
        });
      }

      removeGridEntityObjectAtGridCoordinates(globals, coordinates);

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

  void handleEditorMeshPlacementAction(Globals) {
    if (object("mesh-preview").scale == 0.f) {
      return;
    }

    // @todo check to see if any mesh objects exist at the
    // mesh preview position, and don't place it if so

    if (state.editor.snapMeshesToGrid) {
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

  void handleEditorMeshSelectionAction(Globals) {
    auto* object = findMeshObjectByDirection(globals, getCamera().orientation.getDirection());

    if (object != nullptr) {
      saveObject("mesh-preview", *object);

      object("mesh-preview").color = Vec3f(1.f);
      commit(object("mesh-preview"));

      state.editor.isFindingMesh = false;
      state.editor.isPlacingMesh = true;
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
        createObjectFromCoordinates(globals, record.coordinates);
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

    Gm_WriteFileContents("./game/world/grid_data.txt", serialized);
  }

  // @todo save rotation + scale
  void saveMeshData(Globals) {
    std::string serialized;
    auto* previewMesh = findObject("mesh-preview");

    for (auto& meshName : meshNames) {
      serialized += meshName + "\n";

      for (auto& object : objects(meshName)) {
        if (&object != previewMesh) {
          serialized += serialize3Vector(object.position) + "\n";
        }
      }
    }

    Gm_WriteFileContents("./game/world/mesh_data.txt", serialized);
  }

  void loadWorldGridData(Globals) {
    auto& grid = state.world.grid;
    EntityType currentEntityType;
    std::ifstream file("./game/world/grid_data.txt");

    if (file.fail()) {
      return;
    }

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
        }
      }
    }

    file.close();
  }

  void loadMeshData(Globals) {
    std::ifstream file("./game/world/mesh_data.txt");

    if (file.fail()) {
      return;
    }

    std::string line;
    std::string currentMeshName;

    while (std::getline(file, line)) {
      if (Gm_VectorContains(meshNames, line)) {
        currentMeshName = line;
      } else {
        auto data = Gm_SplitString(line, ",");

        Vec3f position = {
          stof(data[0]),
          stof(data[1]),
          stof(data[2])
        };

        auto& object = createObjectFrom(currentMeshName);

        object.position = position;
        object.scale = HALF_TILE_SIZE;

        commit(object);
      }
    }
  }
#endif