#include "Gamma.h"

static void addKeyHandlers(_ctx) {
  using namespace Gamma;

  auto& input = context->scene.input;

  input.on<MouseButtonEvent>("mousedown", [&](const MouseButtonEvent& event) {
    if (!SDL_GetRelativeMouseMode()) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
    }
  });

  input.on<Key>("keyup", [&](Key key) {
    if (key == Key::ESCAPE) {
      SDL_SetRelativeMouseMode(SDL_FALSE);
    }

    if (key == Key::V) {
      if (Gm_IsFlagEnabled(GammaFlags::VSYNC)) {
        Gm_DisableFlags(GammaFlags::VSYNC);
      } else {
        Gm_EnableFlags(GammaFlags::VSYNC);
      }
    }
  });
}

static void addGroundTiles(_ctx) {
  using namespace Gamma;

  addMesh("plane", 100, Mesh::Plane(2));

  for (int i = -5; i < 5; i++) {
    for (int j = -5; j < 5; j++) {
      auto& plane = createObjectFrom("plane");

      plane.position = Vec3f(i * 15.0f, 0, j * 15.0f);
      plane.scale = 10.0f;

      commit(plane);
    }
  }
}

static void initScene(_ctx) {
  using namespace Gamma;

  // Default camera control/window focus
  auto& input = context->scene.input;
  auto& camera = context->scene.camera;

  input.on<MouseMoveEvent>("mousemove", [&](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      camera.orientation.pitch += event.deltaY / 1000.0f;
      camera.orientation.yaw += event.deltaX / 1000.0f;
    }
  });

  addKeyHandlers(context);
  addGroundTiles(context);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.0f, 1.0f);
  light.color = Vec3f(1.0f, 1.0f, 0);

  camera.position = objects("plane")[0].position + Vec3f(0, 20.0f, 0);
}

static void updateScene(_ctx, float dt) {
  Gm_HandleFreeCameraMode(context, dt);
}

int main(int argc, char* argv[]) {
  auto* context = Gm_CreateContext();

  Gm_SetRenderMode(context, GmRenderMode::OPENGL);

  initScene(context);

  while (!context->window.closed) {
    float dt = Gm_GetDeltaTime(context);

    Gm_LogFrameStart(context);
    Gm_HandleEvents(context);

    updateScene(context, dt);

    Gm_RenderScene(context);
    Gm_LogFrameEnd(context);
  }

  Gm_DestroyContext(context);

  return 0;
}