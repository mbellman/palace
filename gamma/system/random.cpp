#include "system/random.h"

float Gm_Random(float low, float high) {
  return low + randomRange(randomEngine) * (high - low);
}