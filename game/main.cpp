#include "Gamma.h"

#include "game_state.h"
#include "game_init.h"
#include "game_update.h"
#include "game_macros.h"

int main(int argc, char* argv[]) {
  GameState state;
  auto* context = Gm_CreateContext();

  Gm_SetRenderMode(context, GmRenderMode::OPENGL);

  initializeGame(params());

  while (!context->window.closed) {
    float dt = Gm_GetDeltaTime(context);

    Gm_LogFrameStart(context);
    Gm_HandleEvents(context);

    updateGame(params(), dt);

    Gm_RenderScene(context);
    Gm_LogFrameEnd(context);
  }

  // @todo free game memory

  Gm_DestroyContext(context);

  return 0;
}