#include "world_system.h"

void storeEntityAtCoordinates(World& world, const GridCoordinates& coordinates, StaticEntity* entity) {
  world.grid[coordinates] = entity;
}

StaticEntity* getEntityByCoordinates(const World& world, const GridCoordinates& coordinates) {
  if (world.grid.find(coordinates) != world.grid.end()) {
    return world.grid.at(coordinates);
  }

  return nullptr;
}