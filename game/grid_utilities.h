#pragma once

#include <cmath>

#include "Gamma.h"

#include "game_state.h"

constexpr static float TILE_SIZE = 15.f;

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