#pragma once

#include <cmath>

enum EasingType {
  EASE_IN_OUT,
  LINEAR,
  EASE_OUT
};

inline float easeOut(float a, float b, float alpha) {
  const float t = 1.f - (1.f - alpha) * (1.f - alpha);

  return a + (b - a) * t;
}

inline float easeInOut(float a, float b, float alpha) {
  const float t = alpha < 0.5f
    ? 2.f * alpha * alpha
    : 1.f - powf(-2.f * alpha + 2.f, 2) / 2.f;

  return a + (b - a) * t;
}