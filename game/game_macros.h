#pragma once

#include "Gamma.h"

#define Globals GmContext* context, GameState& state
#define globals context, state

#define loop(type, begin, end) (type i = begin; i < end; i++)