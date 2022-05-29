#pragma once

#include "math/plane.h"
#include "performance/tools.h"
#include "system/AbstractRenderer.h"
#include "system/Commander.h"
#include "system/entities.h"
#include "system/macros.h"
#include "system/scene.h"
#include "system/traits.h"
#include "system/type_aliases.h"

#define _ctx GmContext* context

enum GmRenderMode {
  OPENGL,
  VULKAN
};

struct GmContext {
  GmScene scene;
  Gamma::AbstractRenderer* renderer = nullptr;
  Gamma::uint32 lastTick = 0;
  Gamma::uint64 frameStartMicroseconds = 0;
  Gamma::Averager<5, Gamma::uint32> fpsAverager;
  Gamma::Averager<5, Gamma::uint64> frameTimeAverager;
  Gamma::Commander commander;

  struct GmWindow {
    bool closed = false;
    TTF_Font* font_sm = nullptr;
    TTF_Font* font_lg = nullptr;
    SDL_Window* sdl_window = nullptr;
    Gamma::Area<Gamma::uint32> size;
  } window;
};

GmContext* Gm_CreateContext();
void Gm_OpenWindow(GmContext* context, const Gamma::Area<Gamma::uint32>& size);  // @todo
void Gm_SetRenderMode(GmContext* context, GmRenderMode mode);
float Gm_GetDeltaTime(GmContext* context);
void Gm_LogFrameStart(GmContext* context);
void Gm_HandleEvents(GmContext* context);
void Gm_RenderScene(GmContext* context);
void Gm_LogFrameEnd(GmContext* context);
void Gm_DestroyContext(GmContext* context);