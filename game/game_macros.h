#pragma once

#include "Gamma.h"

#define args() GmContext* context, GameState& state
#define params() context, state

#define loop(type, begin, end) (type i = begin; i < end; i++)