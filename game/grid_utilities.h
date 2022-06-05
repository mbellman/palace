#pragma once

#include <cmath>

#include "Gamma.h"

constexpr static float TILE_SIZE = 15.f;

// @todo We can encode the x/y/z position with a smaller
// data type if we use a single-dimensional array for
// the world grid, or chunking or something. This will
// decrease the memory size of entities which define
// their grid coordinates as well.
struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;
};

inline Gamma::Vec3f gridCoordinatesToWorldPosition(const GridCoordinates& coordinates) {
  return {
    coordinates.x * TILE_SIZE,
    coordinates.y * TILE_SIZE,
    coordinates.z * TILE_SIZE
  };
}

inline GridCoordinates worldPositionToGridCoordinates(const Gamma::Vec3f& position) {
  return {
    int(roundf(position.x / TILE_SIZE)),
    int(roundf(position.y / TILE_SIZE)),
    int(roundf(position.z / TILE_SIZE))
  };
}