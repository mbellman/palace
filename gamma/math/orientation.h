#pragma once

#include "math/vector.h"

namespace Gamma {
  struct Orientation {
    float roll = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;

    Orientation() {};
    Orientation(float roll, float pitch, float yaw): roll(roll), pitch(pitch), yaw(yaw) {};

    Vec3f getDirection() const;
    Vec3f getLeftDirection() const;
    Vec3f getRightDirection() const;
    Vec3f getUpDirection() const;
    void face(const Vec3f& forward, const Vec3f& up);
    Orientation invert() const;
    Vec3f toVec3f() const;
  };
}