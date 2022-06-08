#pragma once

#include <cmath>

#include "Gamma.h"

constexpr static float TILE_SIZE = 15.f;
constexpr static float HALF_TILE_SIZE = TILE_SIZE / 2.f;

// @todo We can encode the x/y/z position with a smaller
// data type if we use a single-dimensional array for
// the world grid, or chunking or something. This will
// decrease the memory size of entities which define
// their grid coordinates as well.
struct GridCoordinates {
  int x = 0;
  int y = 0;
  int z = 0;

  GridCoordinates(): x(0), y(0), z(0) {};
  GridCoordinates(int x, int y, int z): x(x), y(y), z(z) {};

  bool GridCoordinates::operator==(const GridCoordinates& comparison) {
    return x == comparison.x && y == comparison.y && z == comparison.z;
  }

  bool GridCoordinates::operator!=(const GridCoordinates& comparison) {
    return x != comparison.x || y != comparison.y || z != comparison.z;
  }

  GridCoordinates GridCoordinates::operator+(const GridCoordinates& coordinates) {
    return {
      x + coordinates.x,
      y + coordinates.y,
      z + coordinates.z
    };
  }
};

inline Gamma::Vec3f gridCoordinatesToWorldPosition(const GridCoordinates& coordinates) {
  return {
    coordinates.x * TILE_SIZE + HALF_TILE_SIZE,
    coordinates.y * TILE_SIZE + HALF_TILE_SIZE,
    coordinates.z * TILE_SIZE + HALF_TILE_SIZE
  };
}

inline GridCoordinates worldPositionToGridCoordinates(const Gamma::Vec3f& position) {
  return {
    int(floorf(position.x / TILE_SIZE)),
    int(floorf(position.y / TILE_SIZE)),
    int(floorf(position.z / TILE_SIZE))
  };
}