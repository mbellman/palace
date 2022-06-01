#include "Gamma.h"

using namespace Gamma;

static void addKeyHandlers(_ctx) {
  auto& input = getInput();

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
  addMesh("plane", 100, Mesh::Plane(2));

  for (int i = -5; i < 5; i++) {
    for (int j = -5; j < 5; j++) {
      auto& plane = createObjectFrom("plane");

      plane.position = Vec3f(i * 15.0f, 0, j * 15.0f);
      plane.scale = 14.0f;

      plane.color = Vec3f(
        Gm_Random(0.f, 0.3f),
        Gm_Random(0.7f, 1.f),
        Gm_Random(0.f, 0.5f)
      );

      commit(plane);
    }
  }
}

static void addRocks(_ctx) {
  addMesh("rock", 5, Mesh::Model("./game/models/rock.obj"));

  auto randomPosition = []() {
    return std::roundf(Gm_Random(-5.f, 4.f)) * 15.f;
  };

  for (uint8 i = 0; i < 5; i++) {
    auto& rock = createObjectFrom("rock");

    rock.position = Vec3f(
      randomPosition(),
      1.f,
      randomPosition()
    );

    rock.rotation.y = Gm_Random(0.f, Gm_TAU);
    rock.scale = 7.f;

    commit(rock);
  }
}

static void initGame(_ctx) {
  auto& input = getInput();
  auto& camera = getCamera();

  input.on<MouseMoveEvent>("mousemove", [&](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      camera.orientation.pitch += event.deltaY / 1000.0f;
      camera.orientation.yaw += event.deltaX / 1000.0f;
    }
  });

  addKeyHandlers(context);
  addGroundTiles(context);
  addRocks(context);

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.0f, 1.0f);
  light.color = Vec3f(1.0f, 1.0f, 0);

  camera.position = objects("plane")[0].position + Vec3f(0, 20.0f, 0);
}