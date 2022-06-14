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

template<typename T>
struct GridMap : std::unordered_map<GridCoordinates, T*, GridCoordinatesHasher> {
  T* get(const GridCoordinates& coordinates) {
    if (find(coordinates) != end()) {
      return at(coordinates);
    }

    return nullptr; 
  }
};

struct DynamicEntityManager {
  DynamicEntity* entities = nullptr;
};

struct World {
  GridMap<StaticEntity> grid;
  GridMap<TriggerEntity> triggers;
  DynamicEntityManager entities;
};