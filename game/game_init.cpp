#include "Gamma.h"

#include "game_init.h"
#include "orientation_system.h"
#include "grid_utilities.h"
#include "game_macros.h"
#include "game_state.h"

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
  addMesh("plane", 196, Mesh::Plane(2));

  for (int i = -5; i < 5; i++) {
    for (int j = -5; j < 5; j++) {
      if (i == 0 && (j == 1 || j == 2)) {
        continue;
      }

      auto& plane = createObjectFrom("plane");

      plane.position = gridCoordinatesToWorldPosition({ i, 0, j }) + Vec3f(0, -HALF_TILE_SIZE, 0);
      plane.scale = TILE_SIZE;

      plane.color = Vec3f(
        Gm_Random(0.f, 0.3f),
        Gm_Random(0.5f, 1.f),
        Gm_Random(0.f, 0.7f)
      );

      commit(plane);
    }
  }

  for (int i = -5; i < 5; i++) {
    for (int j = -5; j < 5; j++) {
      if (i == 0 && (j == 3 || j == 4)) {
        continue;
      }

      auto& plane = createObjectFrom("plane");

      plane.position = gridCoordinatesToWorldPosition({ i, j - 5, 1 }) + Vec3f(0, 0, -HALF_TILE_SIZE);
      plane.scale = TILE_SIZE;
      plane.rotation.x = -Gm_PI / 2.f;

      plane.color = Vec3f(
        Gm_Random(0.5f, 1.f),
        Gm_Random(0.f, 0.3f),
        Gm_Random(0.1f, 0.4f)
      );

      commit(plane);
    }
  }
}

static void addStaircase(args()) {
  addMesh("stairs", 2, Mesh::Model("./game/models/staircase/model.obj"));

  auto& s1 = createObjectFrom("stairs");
  auto& s2 = createObjectFrom("stairs");

  s1.position = gridCoordinatesToWorldPosition({ 0, 0, 2 }) + Vec3f(0, -HALF_TILE_SIZE, 0);
  s1.position.y -= 7.5f;
  s1.scale = 7.5f;
  s1.color = Vec3f(0.5f);
  s1.rotation.y = -Gm_PI / 2.f;

  s2.position = gridCoordinatesToWorldPosition({ 0, -1, 1 }) + Vec3f(0, -HALF_TILE_SIZE, 0);
  s2.position.y -= 7.5f;
  s2.scale = 7.5f;
  s2.color = Vec3f(0.5f);
  s2.rotation.y = -Gm_PI / 2.f;

  commit(s1);
  commit(s2);

  // @todo define a proper way of creating entities
  // @todo in debug mode, render colored cubes to represent entity types
  state.staircaseMovers[0] = {{0,1,3}, {0,1,2}, Vec3f(0, -TILE_SIZE, 0)};
  state.staircaseMovers[1] = {{0,0,2}, {0,0,1}, Vec3f(0, -TILE_SIZE, 0)};
  state.staircaseMovers[2] = {{0,-1,1}, {0,-1,0}, Vec3f(0, -TILE_SIZE, 0)};
  state.staircaseMovers[3] = {{0,-2,0}, {0,-2,-1}, Vec3f(0, -TILE_SIZE, 0)};

  state.staircaseMovers[4] = {{0,-3,-1}, {0,-2,-1}, Vec3f(0, 0, TILE_SIZE)};
  state.staircaseMovers[5] = {{0,-2,0}, {0,-1,0}, Vec3f(0, 0, TILE_SIZE)};
  state.staircaseMovers[6] = {{0,-1,1}, {0,0,1}, Vec3f(0, 0, TILE_SIZE)};
  state.staircaseMovers[7] = {{0,0,2}, {0,1,2}, Vec3f(0, 0, TILE_SIZE)};

  state.worldOrientationChanges[0] = {{0,1,3}, POSITIVE_Y_UP};
  state.worldOrientationChanges[1] = {{0,-3,-1}, NEGATIVE_Z_UP};
}

static void addRocks(args()) {
  addMesh("rock", 5, Mesh::Model("./game/models/rock/model.obj"));

  auto randomPosition = []() {
    return std::roundf(Gm_Random(-5.f, 4.f)) * TILE_SIZE + HALF_TILE_SIZE;
  };

  for loop(uint8, 0, 5) {
    auto& rock = createObjectFrom("rock");

    rock.position = Vec3f(
      randomPosition(),
      1.f,
      randomPosition()
    );

    rock.rotation.y = Gm_Random(0.f, Gm_TAU);
    rock.scale = Gm_Random(6.f, 9.f);
    rock.scale.y *= 1.5f;

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

  for loop(uint8, 0, 200) {
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

  for loop(uint8, 0, 100) {
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

static void addCacti(args()) {
  addMesh("cactus", 5, Mesh::Model("./game/models/cactus/model.obj"));

  auto randomPosition = [](float low, float high) {
    return std::roundf(Gm_Random(low, high)) * TILE_SIZE + HALF_TILE_SIZE;
  };

  for loop(uint8, 0, 5) {
    auto& cactus = createObjectFrom("cactus");

    cactus.color = Vec3f(0.6f, 1.f, 0.3f);
    cactus.scale = Gm_Random(8.f, 12.f);

    cactus.position = Vec3f(
      randomPosition(-5.f, 4.f),
      randomPosition(-10.f, -3.f),
      1.f * TILE_SIZE
    );

    cactus.rotation.x = -Gm_PI / 2.f;
    cactus.rotation.z = Gm_Random(0, Gm_PI);

    commit(cactus);
  }
}

static void addLamps(args()) {
  addMesh("lamp", 5, Mesh::Model("./game/models/lamp/model.obj"));

  mesh("lamp")->texture = "./game/models/lamp/texture.png";
  mesh("lamp")->normalMap = "./game/models/lamp/normals.png";

  auto randomPosition = []() {
    return std::roundf(Gm_Random(-5.f, 4.f)) * TILE_SIZE + HALF_TILE_SIZE;
  };

  for loop(uint8, 0, 5) {
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
    light.position = lamp.position - Vec3f(0, TILE_SIZE, 0);
    light.power = 2.f;
    light.radius = 20.f;
    light.isStatic = true;
  }
}

static void addStatues(args()) {
  addMesh("statue", 1, Mesh::Model("./game/models/statue/model.obj"));
  mesh("statue")->type = MeshType::REFRACTIVE;

  auto& statue = createObjectFrom("statue");

  statue.position = gridCoordinatesToWorldPosition({ 0, 0, 0 }) + Vec3f(0, -HALF_TILE_SIZE, 0);
  statue.scale = HALF_TILE_SIZE;
  statue.color = pVec4(200, 220, 255);
  statue.rotation.y = Gm_PI;

  commit(statue);

  addMesh("anubis", 1, Mesh::Model("./game/models/anubis/model.obj"));
  mesh("anubis")->type = MeshType::PROBE_REFLECTOR;
  mesh("anubis")->probe = "anubis-probe";

  auto& anubis = createObjectFrom("anubis");

  anubis.position = gridCoordinatesToWorldPosition({ 0, -10, 0 }) + Vec3f(0, 0, HALF_TILE_SIZE);
  anubis.scale = TILE_SIZE;
  anubis.rotation = Vec3f(-Gm_PI / 2.f, 0, Gm_PI / 2.f);
  anubis.color = Vec3f(0.4f, 0.6f, 1.f);

  commit(anubis);

  addProbe("anubis-probe", anubis.position + Vec3f(0, 0, -10.f));
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
  addStaircase(params());
  addRocks(params());
  addParticles(params());
  addPlants(params());
  addCacti(params());
  addLamps(params());
  addStatues(params());

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.3f, 0.5f, -1.0f).invert();
  light.color = Vec3f(1.f, 0.8f, 0.4f);

  camera.position = gridCoordinatesToWorldPosition({ 0, 1, -5 });
}