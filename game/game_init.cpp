#include "Gamma.h"

#include "game_init.h"
#include "orientation_system.h"

using namespace Gamma;

static void addKeyHandlers(args()) {
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

static void addGroundTiles(args()) {
  addMesh("plane", 100, Mesh::Plane(2));

  for (int i = -5; i < 5; i++) {
    for (int j = -5; j < 5; j++) {
      auto& plane = createObjectFrom("plane");

      plane.position = Vec3f(i * 15.0f, 0, j * 15.0f);
      plane.scale = 14.0f;

      plane.color = Vec3f(
        Gm_Random(0.f, 0.3f),
        Gm_Random(0.5f, 1.f),
        Gm_Random(0.f, 0.7f)
      );

      commit(plane);
    }
  }
}

static void addRocks(args()) {
  addMesh("rock", 5, Mesh::Model("./game/models/rock/model.obj"));

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
    rock.scale = Gm_Random(6.f, 9.f);

    commit(rock);
  }
}

static void addParticles(args()) {
  addMesh("particles", 1000, Mesh::Particles());

  auto& particles = mesh("particles")->particleSystem;

  particles.spread = 100.f;
  particles.speedVariation = 5.f;
  particles.deviation = 10.f;
  particles.sizeVariation = 3.f;
  particles.medianSize = 5.f;
}

static void addPlants(args()) {
  addMesh("grass", 200, Mesh::Model("./game/models/grass/model.obj"));
  addMesh("flower-stalk", 100, Mesh::Model("./game/models/flower/stalk.obj"));
  addMesh("flower-petals", 100, Mesh::Model("./game/models/flower/petals.obj"));

  mesh("flower-petals")->normalMap = "./game/models/flower/petals-normals.png";

  for (uint8 i = 0; i < 200; i++) {
    auto& grass = createObjectFrom("grass");

    grass.position = Vec3f(
      Gm_Random(-70.f, 70.f),
      0.f,
      Gm_Random(-70.f, 70.f)
    );

    grass.scale = Gm_Random(1.f, 3.f);
    grass.color = pVec4(0, 255, 0);
    grass.rotation.y = Gm_Random(0.f, Gm_TAU);

    commit(grass);
  }

  for (uint8 i = 0; i < 100; i++) {
    auto& stalk = createObjectFrom("flower-stalk");
    auto& petals = createObjectFrom("flower-petals");

    Vec3f position = Vec3f(
      Gm_Random(-70.f, 70.f),
      0.f,
      Gm_Random(-70.f, 70.f)
    );

    stalk.position = petals.position = position;
    stalk.scale = petals.scale = Gm_Random(1.f, 2.f);
    stalk.rotation.y = petals.rotation.y = Gm_Random(0.f, Gm_TAU);

    stalk.color = pVec4(0, 255, 0);
    petals.color = Vec3f(Gm_Random(0.f, 1.f), Gm_Random(0.f, 1.f), Gm_Random(0.f, 1.f));

    commit(stalk);
    commit(petals);
  }
}

static void addLamps(args()) {
  addMesh("lamp", 5, Mesh::Model("./game/models/lamp/model.obj"));

  mesh("lamp")->texture = "./game/models/lamp/texture.png";
  mesh("lamp")->normalMap = "./game/models/lamp/normals.png";

  auto randomPosition = []() {
    return std::roundf(Gm_Random(-5.f, 4.f)) * 15.f;
  };

  for (uint8 i = 0; i < 5; i++) {
    auto& lamp = createObjectFrom("lamp");

    lamp.position = Vec3f(
      randomPosition(),
      25.f,
      randomPosition()
    );

    lamp.rotation.y = Gm_Random(0.f, Gm_TAU);
    lamp.scale = 5.f;

    commit(lamp);

    auto& light = createLight(LightType::POINT_SHADOWCASTER);

    light.color = Vec3f(1.f, 0.3f, 0.1f);
    light.position = lamp.position - Vec3f(0, 15.f, 0);
    light.power = 2.f;
    light.radius = 20.f;
    light.isStatic = true;
  }
}

static void addStatue(args()) {
  addMesh("statue", 1, Mesh::Model("./game/models/statue/model.obj"));
  mesh("statue")->type = MeshType::REFRACTIVE;

  auto& statue = createObjectFrom("statue");

  statue.scale = 7.5f;
  statue.color = pVec4(200, 220, 255);

  commit(statue);
}

void initializeGame(args()) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  auto& input = getInput();
  auto& camera = getCamera();

  input.on<MouseMoveEvent>("mousemove", [context, &state](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      updateCameraFromMouseMoveEvent(params(), event);
    }
  });

  // @temporary
  input.on<KeyboardEvent>("keydown", [context, &state](const KeyboardEvent& event) {
    auto key = event.key;

    if (key == Key::NUM_1) {
      setWorldOrientation(params(), POSITIVE_Y_UP);
    } else if (key == Key::NUM_2) {
      setWorldOrientation(params(), NEGATIVE_Y_UP);
    } else if (key == Key::NUM_3) {
      setWorldOrientation(params(), POSITIVE_Z_UP);
    } else if (key == Key::NUM_4) {
      setWorldOrientation(params(), NEGATIVE_Z_UP);
    } else if (key == Key::NUM_5) {
      setWorldOrientation(params(), POSITIVE_X_UP);
    } else if (key == Key::NUM_6) {
      setWorldOrientation(params(), NEGATIVE_X_UP);
    }
  });

  addKeyHandlers(params());
  addGroundTiles(params());
  addRocks(params());
  addParticles(params());
  addPlants(params());
  addLamps(params());
  addStatue(params());

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.5f, -1.f, 1.f);
  light.color = Vec3f(1.f, 0.8f, 0.4f);

  camera.position = objects("plane")[0].position + Vec3f(0, 15.f, 0);
}