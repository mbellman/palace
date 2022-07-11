#pragma once

#include <string>

#include "Gamma.h"

#include "game_entities.h"

struct GmContext;
struct GameState;

struct ObjectParameters {
  Gamma::Vec3f scale;
  Gamma::Vec3f color;
};

const ObjectParameters& getGridObjectParameters(EntityType entityType);
const ObjectParameters& getMeshObjectParameters(const std::string& meshName);
void createGridObjectFromCoordinates(GmContext* context, GameState& state, const GridCoordinates& coordinates);
Gamma::Object* findObjectByPosition(Gamma::ObjectPool& objects, const Gamma::Vec3f& position);