#include "Gamma.h"

#include "game_init.h"
#include "orientation_system.h"
#include "grid_utilities.h"
#include "world_system.h"
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
  addMesh("ground-tile", 196, Mesh::Cube());

  for (s16 i = -5; i < 5; i++) {
    for (s16 j = -5; j < 5; j++) {
      if (i == 0 && (j == 1 || j == 2)) {
        continue;
      }

      state.world.grid.set({i,1,j}, new WalkableSpace);

      // @todo create ground entities
      auto& cube = createObjectFrom("ground-tile");

      cube.position = gridCoordinatesToWorldPosition({ i, -1, j });
      cube.scale = HALF_TILE_SIZE * 0.98f;
      cube.color = Vec3f(1.f, 0.7f, 0.3f);

      commit(cube);
    }
  }

  for (s16 i = -5; i < 5; i++) {
    for (s16 j = -5; j < 5; j++) {
      if (i == 0 && (j == 3 || j == 4)) {
        continue;
      }

      state.world.grid.set({i,j-5,-1}, new WalkableSpace);

      // @todo create ground entities
      auto& cube = createObjectFrom("ground-tile");

      cube.position = gridCoordinatesToWorldPosition({ i, j - 5, 1 });
      cube.scale = HALF_TILE_SIZE * 0.98f;
      cube.color = Vec3f(1.f, 0.7f, 0.3f);

      commit(cube);
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

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.triggers.set(coordinates, trigger);
  };

  state.world.grid.set({0,0,2}, new StaircaseMover);
  state.world.grid.set({0,-1,1}, new StaircaseMover);
  state.world.grid.set({0,-2,0}, new StaircaseMover);

  createWorldOrientationChange({0,0,2}, POSITIVE_Y_UP);
  createWorldOrientationChange({0,-2,0}, NEGATIVE_Z_UP);
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

    auto& coords = worldPositionToGridCoordinates(rock.position);

    state.world.grid.remove({ coords.x, 1, coords.z });
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

static void addCacti(args()) {
  addMesh("cactus", 5, Mesh::Model("./game/models/cactus/model.obj"));

  auto randomPosition = [](float low, float high) {
    return std::roundf(Gm_Random(low, high)) * TILE_SIZE + HALF_TILE_SIZE;
  };

  for loop(uint8, 0, 5) {
    auto& cactus = createObjectFrom("cactus");

    cactus.color = Vec3f(0.6f, 1.f, 0.3f);
    cactus.scale = Gm_Random(8.f, 15.f);

    cactus.position = Vec3f(
      randomPosition(-5.f, 4.f),
      randomPosition(-10.f, -3.f),
      TILE_SIZE
    );

    cactus.rotation.x = -Gm_PI / 2.f;
    cactus.rotation.z = Gm_Random(0, Gm_PI);

    commit(cactus);

    auto& coords = worldPositionToGridCoordinates(cactus.position);

    state.world.grid.remove({ coords.x, coords.y, -1 });
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

  auto& coords = worldPositionToGridCoordinates(statue.position);

  state.world.grid.remove({ coords.x, 1, coords.z });

  addMesh("anubis", 1, Mesh::Model("./game/models/anubis/model.obj"));
  mesh("anubis")->type = MeshType::PROBE_REFLECTOR;
  mesh("anubis")->probe = "anubis-probe";

  auto& anubis = createObjectFrom("anubis");

  anubis.position = gridCoordinatesToWorldPosition({ 0, -9, 0 }) + Vec3f(0, 0, HALF_TILE_SIZE);
  anubis.scale = TILE_SIZE;
  anubis.rotation = Vec3f(-Gm_PI / 2.f, 0, Gm_PI / 2.f);
  anubis.color = Vec3f(0.4f, 0.6f, 1.f);

  commit(anubis);

  addProbe("anubis-probe", anubis.position + Vec3f(0, 0, -TILE_SIZE));

  auto& coords2 = worldPositionToGridCoordinates(anubis.position);

  state.world.grid.remove({ coords2.x, coords2.y, -1 });
  state.world.grid.remove({ coords2.x, coords2.y + 1, -1 });
  state.world.grid.remove({ coords2.x, coords2.y - 1, -1 });
}

// @todo add entity indicators toggle
static void addEntityIndicators(args()) {
  auto totalEntities = (s16)state.world.grid.size() + (s16)state.world.triggers.size();

  addMesh("indicator", totalEntities, Mesh::Cube());

  for (auto& [ coordinates, entity ] : state.world.grid) {
    auto& cube = createObjectFrom("indicator");

    cube.position = gridCoordinatesToWorldPosition(coordinates);
    cube.scale = 0.5f;

    switch (entity->type) {
      case WALKABLE_SPACE:
        cube.color = pVec4(0,0,255);
        break;
    }

    commit(cube);
  }

  // @todo use spheres
  for (auto& [ coordinates, trigger ] : state.world.triggers) {
    auto& cube = createObjectFrom("indicator");

    cube.position = gridCoordinatesToWorldPosition(coordinates) + Vec3f(1.f);
    cube.scale = 0.5f;

    switch (trigger->type) {
      case WORLD_ORIENTATION_CHANGE:
        cube.color = pVec4(255,0,255);
        break;
    }

    commit(cube);
  }
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
  addCacti(params());
  addLamps(params());
  addStatues(params());
  addEntityIndicators(params());

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.3f, 0.5f, -1.0f).invert();
  light.color = Vec3f(1.f, 0.8f, 0.4f);

  camera.position = gridCoordinatesToWorldPosition({ 0, 1, -5 });
}