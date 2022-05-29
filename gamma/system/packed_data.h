#pragma once

#include "math/vector.h"
#include "system/type_aliases.h"

namespace Gamma {
  /**
   * pVec4
   * -----
   *
   * A packed vector containing four 8-bit integer components.
   */
  struct pVec4 {
    uint8 r = 255;
    uint8 g = 255;
    uint8 b = 255;
    uint8 a = 255;

    pVec4() {};
    pVec4(uint8 r, uint8 g, uint8 b) : r(r), g(g), b(b), a(255) {};
    pVec4(uint8 r, uint8 g, uint8 b, uint8 a) : r(r), g(g), b(b), a(a) {};
    pVec4(const Vec3f& value);
    pVec4(const Vec4f& value);
  };
}