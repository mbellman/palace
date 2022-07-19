#include <map>
#include <string>
#include <vector>

#include "object_system.h"
#include "game_entities.h"
#include "grid_utilities.h"
#include "game_state.h"
#include "game_macros.h"

#define scale(...) Vec3f(__VA_ARGS__)
#define color(...) Vec3f(__VA_ARGS__)

using namespace Gamma;

const static std::map<EntityType, ObjectParameters> gridObjectParametersMap = {
  {GROUND, {
    scale(HALF_TILE_SIZE * 0.99f),
    color(1.f, 0.7f, 0.3f)
  }},
  {STAIRCASE, {
    scale(HALF_TILE_SIZE),
    color(0.5f)
  }},
  {SWITCH, {
    scale(HALF_TILE_SIZE),
    color(1.f, 0, 0)
  }},
  {WORLD_ORIENTATION_CHANGE, {
    scale(TILE_SIZE * 0.075f),
    color(1.f, 0, 0)
  }}
};

const static std::map<std::string, ObjectParameters> meshObjectParametersMap = {
  {"tulips", {
    scale(HALF_TILE_SIZE),
    color(0.1f, 0.6f, 0.2f)
  }},
  {"tulip-petals", {
    scale(HALF_TILE_SIZE),
    color(1.f, 0.4f, 0.2f)
  }},
  {"arch-vines", {
    scale(HALF_TILE_SIZE),
    color(0.1f, 1.f, 0.4f)
  }},
  {"stone-tile", {
    scale(HALF_TILE_SIZE),
    color(0.75f)
  }}
};

const static std::map<std::string, std::vector<std::string>> compoundMeshMap = {
  {"tulips", { "tulip-petals" }},
  {"arch", { "arch-vines" }}
};

static void createGroundObject(Globals, const GridCoordinates& coordinates) {
  auto& object = createObjectFrom("ground");
  auto& params = getGridObjectParameters(GROUND);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.scale = params.scale;
  object.color = params.color;

  commit(object);
}

static void createStaircaseObject(Globals, const GridCoordinates& coordinates, const Orientation& orientation) {
  auto& object = createObjectFrom("staircase");
  auto& params = getGridObjectParameters(STAIRCASE);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;
  object.rotation.x = orientation.pitch;
  object.rotation.y = orientation.yaw;
  object.rotation.z = -orientation.roll;

  commit(object);
}

static void createSwitchObject(Globals, const GridCoordinates& coordinates) {
  auto& object = createObjectFrom("switch");
  auto& params = getGridObjectParameters(SWITCH);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;

  commit(object);
}

static void createWorldOrientationChangeObject(Globals, const GridCoordinates& coordinates, WorldOrientation worldOrientation) {
  auto& object = createObjectFrom("trigger-indicator");
  auto& params = getGridObjectParameters(WORLD_ORIENTATION_CHANGE);

  object.position = gridCoordinatesToWorldPosition(coordinates);
  object.color = params.color;
  object.scale = params.scale;

  commit(object);
}

const ObjectParameters& getGridObjectParameters(EntityType entityType) {
  return gridObjectParametersMap.at(entityType);
}

const ObjectParameters& getMeshObjectParameters(const std::string& meshName) {
  const static ObjectParameters defaultParameters = {
    scale(HALF_TILE_SIZE),
    color(1.f)
  };
  
  if (meshObjectParametersMap.find(meshName) != meshObjectParametersMap.end()) {
    return meshObjectParametersMap.at(meshName);
  }

  return defaultParameters;
}

void createGridObjectFromCoordinates(Globals, const GridCoordinates& coordinates) {
  auto* entity = state.world.grid.get(coordinates);

  if (entity == nullptr) {
    return;
  }

  switch (entity->type) {
    case GROUND:
      #if DEVELOPMENT == 1
        createGroundObject(globals, coordinates);
      #endif
      break;
    case STAIRCASE:
      createStaircaseObject(globals, coordinates, ((Staircase*)entity)->orientation);        
      break;
    case SWITCH:
      createSwitchObject(globals, coordinates);
      break;
    case WORLD_ORIENTATION_CHANGE:
      #if DEVELOPMENT == 1
        createWorldOrientationChangeObject(globals, coordinates, ((WorldOrientationChange*)entity)->targetWorldOrientation);
      #endif
      break;
    default:
      break;
  }
}

Object& createMeshObject(Globals, const std::string& meshName) {
  auto& object = createObjectFrom(meshName);
  auto& params = getMeshObjectParameters(meshName);

  object.scale = params.scale;
  object.color = params.color;

  commit(object);

  if (compoundMeshMap.find(meshName) != compoundMeshMap.end()) {
    auto& extensionMeshNames = compoundMeshMap.at(meshName);

    for (auto& extensionMeshName : extensionMeshNames) {
      createMeshObject(globals, extensionMeshName);
    }
  }

  return object;
}

void synchronizeCompoundMeshes(Globals) {
  for (auto& [ meshName, _ ] : compoundMeshMap) {
    synchronizeCompoundMeshes(globals, meshName);
  }
}

void synchronizeCompoundMeshes(Globals, const std::string& meshName) {
  if (compoundMeshMap.find(meshName) != compoundMeshMap.end()) {
    auto& baseObjects = objects(meshName);
    auto& extensionMeshNames = compoundMeshMap.at(meshName);

    for (auto& extensionMeshName : extensionMeshNames) {
      auto& extensions = objects(extensionMeshName);

      // Remove any residual extension mesh objects
      // if their base mesh objects were removed
      while (extensions.totalActive() > baseObjects.totalActive()) {
        removeObject(extensions[extensions.totalActive() - 1]);
      }

      for (u8 i = 0; i < baseObjects.totalActive(); i++) {
        auto& object = baseObjects[i];
        auto& extension = extensions[i];

        extension.position = object.position;
        extension.rotation = object.rotation;
        extension.scale = object.scale;

        commit(extension);
      }
    }
  }
}

Object* findObjectByPosition(ObjectPool& objects, const Vec3f& position) {
  for (auto& object : objects) {
    if (object.position == position) {
      return &object;
    }
  }

  return nullptr;
}