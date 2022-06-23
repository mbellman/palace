#pragma once

#include <unordered_map>

#include "grid_utilities.h"
#include "game_entities.h"

struct GmContext;
struct GameState;

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
  void operator[](const GridCoordinates& coordinates) = delete;

  void clear(const GridCoordinates& coordinates) {
    if (find(coordinates) != end()) {
      T* t = at(coordinates);

      delete t;

      erase(coordinates);
    }
  }

  template<typename E>
  u32 count() const {
    u32 total = 0;

    for (auto& [ coordinates, entity ] : *this) {
      if (dynamic_cast<E*>(entity) != nullptr) {
        total++;
      }
    }

    return total;
  }

  template<typename E = T>
  E* get(const GridCoordinates& coordinates) const {
    if (find(coordinates) != end()) {
      return (E*)at(coordinates);
    }

    return nullptr; 
  }

  bool has(const GridCoordinates& coordinates) const {
    return find(coordinates) != end();
  }

  void set(const GridCoordinates& coordinates, T* value) {
    clear(coordinates);
    insert({ coordinates, value });
  }
};

struct DynamicEntityManager {
  DynamicEntity* entities = nullptr;
};

struct Area {
  // @todo
};

struct World {
  GridMap<StaticEntity> grid;
  GridMap<TriggerEntity> triggers;
  DynamicEntityManager entities;
};

Gamma::Object* queryObjectByPosition(GmContext* context, GameState& state, Gamma::ObjectPool& objects, const Gamma::Vec3f& position);
void createObjectFromCoordinates(GmContext* context, GameState& state, const GridCoordinates& coordinates);

template<typename E>
void setStaticEntityOverRange(GmContext* context, GameState& state, const GridCoordinates& start, const GridCoordinates& end) {
  auto& grid = state.world.grid;

  for (s16 x = start.x; x <= end.x; x++) {
    for (s16 y = start.y; y <= end.y; y++) {
      for (s16 z = start.z; z <= end.z; z++) {
        grid.set({ x, y, z }, new E);
      }
    }
  }
}