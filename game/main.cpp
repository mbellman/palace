#include "Gamma.h"

#include "game_init.cpp"
#include "game_update.cpp"

int main(int argc, char* argv[]) {
  auto* context = Gm_CreateContext();

  Gm_SetRenderMode(context, GmRenderMode::OPENGL);

  initGame(context);

  while (!context->window.closed) {
    float dt = Gm_GetDeltaTime(context);

    Gm_LogFrameStart(context);
    Gm_HandleEvents(context);

    updateGame(context, dt);

    Gm_RenderScene(context);
    Gm_LogFrameEnd(context);
  }

  Gm_DestroyContext(context);

  return 0;
}