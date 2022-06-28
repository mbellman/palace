#pragma once

#include "Gamma.h"

#include "game_entities.h"

struct GmContext;
struct GameState;

struct ObjectParameters {
  Gamma::Vec3f scale;
  Gamma::Vec3f color;
};

const ObjectParameters& getObjectParameters(EntityType entityType);
Gamma::Object* queryObjectByPosition(GmContext* context, GameState& state, Gamma::ObjectPool& objects, const Gamma::Vec3f& position);
void createObjectFromCoordinates(GmContext* context, GameState& state, const GridCoordinates& coordinates);