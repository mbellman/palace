#pragma once

#include "Gamma.h"

struct ObjectParameters {
  Gamma::Vec3f scale;
  Gamma::Vec3f color;
};

const ObjectParameters& getObjectParameters(u8 entityTypeTableIndex);