#include "system/packed_data.h"
#include "system/type_aliases.h"

#define CLAMP(f) (f < 0.0f ? 0.0f : f > 1.0f ? 1.0f : f)

namespace Gamma {
  /**
   * pVec4
   * -----
   */
  pVec4::pVec4(const Vec3f& value) {
    r = uint8(CLAMP(value.x) * 255.0f);
    g = uint8(CLAMP(value.y) * 255.0f);
    b = uint8(CLAMP(value.z) * 255.0f);
    a = 255;
  }

  pVec4::pVec4(const Vec4f& value) {
    r = uint8(CLAMP(value.x) * 255.0f);
    g = uint8(CLAMP(value.y) * 255.0f);
    b = uint8(CLAMP(value.z) * 255.0f);
    a = uint8(CLAMP(value.w) * 255.0f);
  }
}