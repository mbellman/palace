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

static void addGroundEntities(args()) {
  auto& grid = state.world.grid;

  // Top area
  for (s16 x = -5; x < 5; x++) {
    for (s16 z = -5; z < 5; z++) {
      if (x == 0 && (z == 1 || z == 2)) {
        continue;
      }

      grid.set({x,1,z}, new WalkableSpace);
      grid.set({x,-1,z}, new Ground);
    }
  }

  // Left area
  for (s16 z = -5; z < 5; z++) {
    for (s16 y = -1; y < 10; y++) {
      grid.set({-6,y,z}, new Ground);

      if (y >= 0) {
        grid.set({-4,y,z}, new WalkableSpace);
      }
    }
  }

  grid.remove({-4,2,0});

  // Lower area
  for (s16 x = -5; x < 5; x++) {
    for (s16 y = -10; y < -1; y++) {
      if (x == 0 && (y == -2 || y == -1)) {
        continue;
      }

      grid.set({x,y,-1}, new WalkableSpace);
      grid.set({x,y,1}, new Ground);
    }
  }
}

static void addStaircaseEntities(args()) {
  auto& grid = state.world.grid;

  // Down to lower area
  grid.set({0,-1,2}, new Staircase);
  grid.set({0,-2,1}, new Staircase);

  // Up to left area
  grid.set({-3,0,0}, new Staircase);
  grid.get<Staircase>({-3,0,0})->orientation.yaw = -Gm_PI / 2.f;
  grid.set({-4,1,0}, new Staircase);
  grid.get<Staircase>({-4,1,0})->orientation.yaw = -Gm_PI / 2.f;
  grid.set({-5,2,0}, new Staircase);
  grid.get<Staircase>({-5,2,0})->orientation.yaw = -Gm_PI / 2.f;

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.triggers.set(coordinates, trigger);
  };

  createWorldOrientationChange({0,0,2}, POSITIVE_Y_UP);
  createWorldOrientationChange({0,-2,0}, NEGATIVE_Z_UP);

  // createWorldOrientationChange({-4,2,0}, POSITIVE_X_UP);
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

static void addEntityObjects(args()) {
  // @todo count entities by type
  auto& grid = state.world.grid;

  addMesh("ground-tile", grid.count<Ground>(), Mesh::Cube());
  addMesh("staircase", grid.count<Staircase>(), Mesh::Model("./game/models/staircase/model.obj"));

  for (auto& [ coordinates, entity ] : grid) {
    switch (entity->type) {
      case GROUND: {
        // @todo createGroundTileObject()
        auto& ground = createObjectFrom("ground-tile");

        ground.position = gridCoordinatesToWorldPosition(coordinates);
        ground.scale = HALF_TILE_SIZE * 0.98f;
        ground.color = Vec3f(1.f, 0.7f, 0.3f);

        commit(ground);
        break;
      }
      case STAIRCASE: {
        // @todo createStaircaseObject()
        auto& staircase = createObjectFrom("staircase");
        auto* stairs = (Staircase*)entity;

        staircase.position = gridCoordinatesToWorldPosition(coordinates);
        staircase.color = Vec3f(0.5f);
        staircase.scale = HALF_TILE_SIZE;
        // @todo use proper model orientation to avoid the -PI/2 offset here
        staircase.rotation.y = -Gm_PI / 2.f + stairs->orientation.yaw;

        commit(staircase);
        break;
      }
      default:
        break;
    }
  }
}

// @todo allow indicators to be toggled
static void addInvisibleEntityIndicators(args()) {
  auto totalEntities = (s16)state.world.grid.size() + (s16)state.world.triggers.size();

  addMesh("indicator", totalEntities, Mesh::Cube());

  for (auto& [ coordinates, entity ] : state.world.grid) {
    switch (entity->type) {
      case WALKABLE_SPACE: {
        auto& indicator = createObjectFrom("indicator");

        indicator.position = gridCoordinatesToWorldPosition(coordinates);
        indicator.scale = 0.5f;
        indicator.color = pVec4(0,0,255);

        commit(indicator);
        break;
      }
    }

  }

  // @todo use spheres
  for (auto& [ coordinates, trigger ] : state.world.triggers) {
    switch (trigger->type) {
      case WORLD_ORIENTATION_CHANGE: {
        auto& indicator = createObjectFrom("indicator");

        indicator.position = gridCoordinatesToWorldPosition(coordinates) + Vec3f(1.f);
        indicator.scale = 0.5f;
        indicator.color = pVec4(255,0,0);

        commit(indicator);
        break;
      }
    }
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
  addGroundEntities(params());
  addStaircaseEntities(params());
  // addRocks(params());
  addParticles(params());
  // addCacti(params());
  // addStatues(params());

  addEntityObjects(params());
  addInvisibleEntityIndicators(params());

  auto& light = createLight(LightType::DIRECTIONAL_SHADOWCASTER);

  light.direction = Vec3f(0.3f, 0.5f, -1.0f).invert();
  light.color = Vec3f(1.f, 0.8f, 0.4f);

  camera.position = gridCoordinatesToWorldPosition({ 0, 1, -5 });
}