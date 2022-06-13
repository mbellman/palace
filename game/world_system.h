#pragma once

#include <string>
#include <unordered_map>

#include "grid_utilities.h"
#include "entities.h"

struct GridCoordinatesHasher {
  std::size_t operator()(const GridCoordinates& coordinates) const {
    u32 ux = u32(coordinates.x + 32768) & 0xFFFF;
    u32 uy = u32(coordinates.y + 32768) & 0xFFFF;
    u32 uz = u32(coordinates.z + 32768) & 0xFFFF;

    // @todo check the frequency of hash collisions in actual level layouts
    return (ux << 16 | uy) + uz;
  }
};

typedef std::unordered_map<GridCoordinates, StaticEntity*, GridCoordinatesHasher> WorldGrid;

struct DynamicEntityManager {
  DynamicEntity* entities = nullptr;
};

struct World {
  WorldGrid grid;
  DynamicEntityManager entities;
};

void storeEntityAtCoordinates(World& world, const GridCoordinates& coordinates, StaticEntity* entity);
StaticEntity* getEntityByCoordinates(const World& world, const GridCoordinates& coordinates);