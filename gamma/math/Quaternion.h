#pragma once

namespace Gamma {
  struct Matrix4f;

  /**
   * Quaternion
   * ----------
   *
   * A struct implementing basic quaternion math,
   * used for managing rotations without degeneracies
   * like gimbal lock.
   */
  struct Quaternion {
    float w;
    float x;
    float y;
    float z;

    static Quaternion fromAxisAngle(float angle, float x, float y, float z);

    Matrix4f toMatrix4f() const;
    Quaternion operator*(const Quaternion& q2) const;
  };
}