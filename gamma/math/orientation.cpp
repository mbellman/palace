#include <cmath>

#include "math/constants.h"
#include "math/orientation.h"
#include "math/vector.h"

namespace Gamma {
  Vec3f Orientation::getDirection() const {
    return Vec3f(
      sinf(yaw) * cosf(pitch),
      -sinf(pitch),
      cosf(yaw) * cosf(pitch)
    ).unit();
  }

  Vec3f Orientation::getLeftDirection() const {
    return Vec3f::cross(getDirection(), getUpDirection()).unit();
  }

  Vec3f Orientation::getRightDirection() const {
    return Vec3f::cross(getUpDirection(), getDirection()).unit();
  }

  Vec3f Orientation::getUpDirection() const {
    return Orientation(roll, pitch - PI / 2.0f, yaw).getDirection();
  }

  void Orientation::face(const Vec3f& forward, const Vec3f& up) {
    const static Vec3f Y_UP(0.0f, 1.0f, 0.0f);
    float uDotY = Vec3f::dot(up, Y_UP);

    // Calculate yaw as a function of forward z/x
    yaw = -1.0f * (atan2f(forward.z, forward.x) - HALF_PI);

    if (uDotY < 0.0f) {
      // If upside-down, flip the yaw by 180 degrees
      yaw -= PI;
    }

    Vec3f rUp = up;

    // Rotate the up vector back onto the y/z plane,
    // and calculate pitch as a function of y/z
    rUp.z = up.x * sinf(yaw) + up.z * cosf(yaw);

    pitch = -1.0f * (atan2f(rUp.y, rUp.z) - HALF_PI);
  }

  Orientation Orientation::invert() const {
    return Orientation(-roll, -pitch, -yaw);
  }

  Vec3f Orientation::toVec3f() const {
    return Vec3f(pitch, yaw, roll);
  }
}