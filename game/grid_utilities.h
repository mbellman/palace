#pragma once

#include <cmath>

#include "Gamma.h"

constexpr static float TILE_SIZE = 15.f;
constexpr static float HALF_TILE_SIZE = TILE_SIZE / 2.f;

struct GridCoordinates {
  s16 x = 0;
  s16 y = 0;
  s16 z = 0;

  GridCoordinates(): x(0), y(0), z(0) {};
  GridCoordinates(s16 x, s16 y, s16 z): x(x), y(y), z(z) {};

  bool GridCoordinates::operator==(const GridCoordinates& comparison) const {
    return x == comparison.x && y == comparison.y && z == comparison.z;
  }

  bool GridCoordinates::operator!=(const GridCoordinates& comparison) const {
    return x != comparison.x || y != comparison.y || z != comparison.z;
  }

  GridCoordinates GridCoordinates::operator+(const GridCoordinates& coordinates) const {
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
    s16(floorf(position.x / TILE_SIZE)),
    s16(floorf(position.y / TILE_SIZE)),
    s16(floorf(position.z / TILE_SIZE))
  };
}

inline Gamma::Vec3f getGridAlignedWorldPosition(const Gamma::Vec3f& position) {
  auto coordinates = worldPositionToGridCoordinates(position);

  return gridCoordinatesToWorldPosition(coordinates);
}