#include <string>

#include "SDL.h"
#include "SDL_ttf.h"
#include "SDL_image.h"

#include "opengl/OpenGLRenderer.h"
#include "performance/benchmark.h"
#include "performance/tools.h"
#include "system/console.h"
#include "system/context.h"
#include "system/flags.h"
#include "system/scene.h"

#define String(value) std::to_string(value)

static void Gm_DisplayDevtools(GmContext* context) {
  using namespace Gamma;

  auto& renderer = *context->renderer;
  auto& resolution = renderer.getInternalResolution();
  auto& renderStats = renderer.getRenderStats();
  auto& sceneStats = Gm_GetSceneStats(context);
  auto& fpsAverager = context->fpsAverager;
  auto& frameTimeAverager = context->frameTimeAverager;
  auto& commander = context->commander;
  auto& window = context->window;
  auto* font_sm = window.font_sm;
  auto* font_lg = window.font_lg;
  uint64 averageFrameTime = frameTimeAverager.average();
  uint32 frameTimeBudget = uint32(100.0f * (float)averageFrameTime / 16667.0f);

  auto fpsLabel = "FPS: "
    + String(fpsAverager.average())
    + ", low "
    + String(fpsAverager.low())
    + " (V-Sync " + (renderStats.isVSynced ? "ON" : "OFF") + ")";

  auto frameTimeLabel = "Frame time: "
    + String(averageFrameTime)
    + "us, high "
    + String(frameTimeAverager.high())
    + " ("
    + String(frameTimeBudget)
    + "%)";

  auto resolutionLabel = "Resolution: " + String(resolution.width) + " x " + String(resolution.height);
  auto vertsLabel = "Verts: " + String(sceneStats.verts);
  auto trisLabel = "Tris: " + String(sceneStats.tris);
  auto memoryLabel = "GPU Memory: " + String(renderStats.gpuMemoryUsed) + "MB / " + String(renderStats.gpuMemoryTotal) + "MB";

  renderer.renderText(font_sm, fpsLabel.c_str(), 25, 25);
  renderer.renderText(font_sm, frameTimeLabel.c_str(), 25, 50);
  renderer.renderText(font_sm, resolutionLabel.c_str(), 25, 75);
  renderer.renderText(font_sm, vertsLabel.c_str(), 25, 100);
  renderer.renderText(font_sm, trisLabel.c_str(), 25, 125);
  renderer.renderText(font_sm, memoryLabel.c_str(), 25, 150);

  // Display command line
  if (commander.isOpen()) {
    std::string caret = SDL_GetTicks() % 1000 < 500 ? "_" : "  ";
    std::string command = "> " + commander.getCommand() + caret;
    const Vec3f fgColor = Vec3f(0.0f, 1.0f, 0.0f);
    const Vec4f bgColor = Vec4f(0.0f, 0.0f, 0.0f, 0.8f);

    renderer.renderText(font_lg, command.c_str(), 25, window.size.height - 200, fgColor, bgColor);
  }

  // Display console messages
  auto* message = Console::getFirstMessage();
  uint8 messageIndex = 0;

  // @todo clear messages after a set duration
  while (message != nullptr) {
    renderer.renderText(font_sm, message->text.c_str(), 25, window.size.height - 150 + (messageIndex++) * 25);

    message = message->next;
  }
}

GmContext* Gm_CreateContext() {
  auto* context = new GmContext();

  SDL_Init(SDL_INIT_EVERYTHING);
  TTF_Init();
  IMG_Init(IMG_INIT_PNG);

  // @todo Gm_OpenWindow(const char* title, const Area<uint32>& size)
  context->window.sdl_window = SDL_CreateWindow(
    "Gamma",
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    640, 480,
    SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
  );

  context->window.font_sm = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 16);
  context->window.font_lg = TTF_OpenFont("./fonts/OpenSans-Regular.ttf", 22);
  context->window.size = { 640, 480 };

  return context;
}

void Gm_SetRenderMode(GmContext* context, GmRenderMode mode) {
  if (context->renderer != nullptr) {
    context->renderer->destroy();

    delete context->renderer;

    context->renderer = nullptr;
  }

  switch (mode) {
    case GmRenderMode::OPENGL:
      context->renderer = new Gamma::OpenGLRenderer(context);
      break;
    case GmRenderMode::VULKAN:
      // @todo
      break;
  }

  if (context->renderer != nullptr) {
    context->renderer->init();
  }
}

float Gm_GetDeltaTime(GmContext* context) {
  Gamma::uint32 ticks = SDL_GetTicks();
  float dt = float(ticks - context->lastTick) / 1000.0f;

  context->lastTick = ticks;

  return dt;
}

void Gm_LogFrameStart(GmContext* context) {
  context->frameStartMicroseconds = Gm_GetMicroseconds();
}

void Gm_HandleEvents(GmContext* context) {
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
      case SDL_QUIT:
        context->window.closed = true;
        break;
      case SDL_WINDOWEVENT:
        if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
          context->window.size = {
            (Gamma::uint32)event.window.data1,
            (Gamma::uint32)event.window.data2
          };
        }

        break;
      default:
        break;
    }

    if (!context->commander.isOpen()) {
      context->scene.input.handleEvent(event);
    }

    #if GAMMA_DEVELOPER_MODE
      context->commander.input.handleEvent(event);
    #endif
  }
}

void Gm_RenderScene(GmContext* context) {
  context->renderer->render();

  #if GAMMA_DEVELOPER_MODE
    Gm_DisplayDevtools(context);
  #endif

  context->renderer->present();

  // @todo move this elsewhere
  Gamma::Gm_SavePreviousFlags();
}

void Gm_LogFrameEnd(GmContext* context) {
  using namespace Gamma;

  uint64 frameTimeInMicroseconds = Gm_GetMicroseconds() - context->frameStartMicroseconds;
  uint32 fps = (uint32)(1000000.0f / (float)frameTimeInMicroseconds);

  context->fpsAverager.add(fps);
  context->frameTimeAverager.add(frameTimeInMicroseconds);

  context->scene.runningTime += frameTimeInMicroseconds / 1000000.0f;
  context->scene.frame++;
}

void Gm_DestroyContext(GmContext* context) {
  // @todo clear scene

  IMG_Quit();

  TTF_CloseFont(context->window.font_sm);
  TTF_CloseFont(context->window.font_lg);
  TTF_Quit();

  SDL_DestroyWindow(context->window.sdl_window);
  SDL_Quit();
}