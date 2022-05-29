#include <cmath>

#include "math/matrix.h"
#include "math/Quaternion.h"

namespace Gamma {
  Quaternion Quaternion::fromAxisAngle(float angle, float x, float y, float z) {
    float sa = sinf(angle / 2.0f);

    return {
      cosf(angle / 2.0f),
      x * sa,
      y * sa,
      z * sa
    };
  }

  Matrix4f Quaternion::toMatrix4f() const {
    return {
      1 - 2 * y * y - 2 * z * z, 2 * x * y - 2 * z * w, 2 * x * z + 2 * y * w, 0.0f,
      2 * x * y + 2 * z * w, 1 - 2 * x * x - 2 * z * z, 2 * y * z - 2 * x * w, 0.0f,
      2 * x * z - 2 * y * w, 2 * y * z + 2 * x * w, 1 - 2 * x * x - 2 * y * y, 0.0f,
      0.0f, 0.0f, 0.0f, 1.0f
    };
  }

  Quaternion Quaternion::operator*(const Quaternion& q2) const {
    return {
      w * q2.w - x * q2.x - y * q2.y - z * q2.z,
      w * q2.x + x * q2.w + y * q2.z - z * q2.y,
      w * q2.y - x * q2.z + y * q2.w + z * q2.x,
      w * q2.z + x * q2.y - y * q2.x + z * q2.w
    };
  }
}