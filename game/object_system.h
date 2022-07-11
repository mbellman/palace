#pragma once

#include <string>

#include "Gamma.h"

#include "game_entities.h"
#include "game_macros.h"

struct GmContext;
struct GameState;

struct ObjectParameters {
  Gamma::Vec3f scale;
  Gamma::Vec3f color;
};

const ObjectParameters& getGridObjectParameters(EntityType entityType);
const ObjectParameters& getMeshObjectParameters(const std::string& meshName);
void createGridObjectFromCoordinates(Globals, const GridCoordinates& coordinates);
Gamma::Object& createMeshObject(Globals, const std::string& meshName, const Gamma::Vec3f& position);
void synchronizeCompoundMeshes(Globals);
Gamma::Object* findObjectByPosition(Gamma::ObjectPool& objects, const Gamma::Vec3f& position);