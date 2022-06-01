#pragma once

#include <math.h>
#include <random>

static std::default_random_engine randomEngine;
static std::uniform_real_distribution<float> randomRange(0.f, 1.f);

float Gm_Random(float low, float high);