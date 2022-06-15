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

template<typename E>
static void addEntityOverRange(args(), const GridCoordinates& start, const GridCoordinates& end) {
  auto& grid = state.world.grid;

  for (s16 x = start.x; x <= end.x; x++) {
    for (s16 y = start.y; y <= end.y; y++) {
      for (s16 z = start.z; z <= end.z; z++) {
        grid.set({ x, y, z }, new E);
      }
    }
  }
}

static void addOrientationTestLayout(args()) {
  auto& grid = state.world.grid;

  // Bottom area
  addEntityOverRange<Ground>(params(), { -4, -1, -4 }, { 4, -1, 4 });

  // Left area
  addEntityOverRange<Ground>(params(), { -5, -1, -4 }, { -5, 8, 4 });

  // Right area
  addEntityOverRange<Ground>(params(), { 5, -1, -4 }, { 5, 8, 4 });
  
  // Top area
  addEntityOverRange<Ground>(params(), { -5, 9, -4 }, { 5, 9, 4 });

  // Back area
  addEntityOverRange<Ground>(params(), { -5, -1, -5 }, { 5, 9, -5 });

  // Front area
  addEntityOverRange<Ground>(params(), { -5, -1, 5 }, { 5, 9, 5 });

  // Bottom to left staircase
  grid.set({ -2, 0, 0 }, new Staircase);
  grid.get<Staircase>({ -2, 0, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.set({ -3, 1, 0 }, new Staircase);
  grid.get<Staircase>({ -3, 1, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.set({ -4, 2, 0 }, new Staircase);
  grid.get<Staircase>({ -4, 2, 0 })->orientation.yaw = -Gm_PI / 2.f;

  // Bottom to front staircase
  grid.set({ 0, 0, 2 }, new Staircase);
  grid.set({ 0, 1, 3 }, new Staircase);
  grid.set({ 0, 2, 4 }, new Staircase);

  // Bottom to right staircase
  grid.set({ 2, 0, 0 }, new Staircase);
  grid.get<Staircase>({ 2, 0, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.set({ 3, 1, 0 }, new Staircase);
  grid.get<Staircase>({ 3, 1, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.set({ 4, 2, 0 }, new Staircase);
  grid.get<Staircase>({ 4, 2, 0 })->orientation.yaw = Gm_PI / 2.f;

  // Bottom to back staircase
  grid.set({ 0, 0, -2 }, new Staircase);
  grid.get<Staircase>({ 0, 0, -2 })->orientation.yaw = Gm_PI;
  grid.set({ 0, 1, -3 }, new Staircase);
  grid.get<Staircase>({ 0, 1, -3 })->orientation.yaw = Gm_PI;
  grid.set({ 0, 2, -4 }, new Staircase);
  grid.get<Staircase>({ 0, 2, -4 })->orientation.yaw = Gm_PI;

  // Left to front staircase
  grid.set({ -4, 4, 2 }, new Staircase);
  grid.get<Staircase>({ -4, 4, 2 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -3, 4, 3 }, new Staircase);
  grid.get<Staircase>({ -3, 4, 3 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -2, 4, 4 }, new Staircase);
  grid.get<Staircase>({ -2, 4, 4 })->orientation.roll = Gm_PI / 2.f;

  // Left to back staircase
  grid.set({ -4, 4, -2 }, new Staircase);
  grid.get<Staircase>({ -4, 4, -2 })->orientation.roll = Gm_PI / 2.f;
  grid.get<Staircase>({ -4, 4, -2 })->orientation.yaw = Gm_PI;
  grid.set({ -3, 4, -3 }, new Staircase);
  grid.get<Staircase>({ -3, 4, -3 })->orientation.roll = Gm_PI / 2.f;
  grid.get<Staircase>({ -3, 4, -3 })->orientation.yaw = Gm_PI;
  grid.set({ -2, 4, -4 }, new Staircase);
  grid.get<Staircase>({ -2, 4, -4 })->orientation.roll = Gm_PI / 2.f;
  grid.get<Staircase>({ -2, 4, -4 })->orientation.yaw = Gm_PI;

  // Left to top staircase
  grid.set({ -4, 6, 0 }, new Staircase);
  grid.get<Staircase>({ -4, 6, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.get<Staircase>({ -4, 6, 0 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -3, 7, 0 }, new Staircase);
  grid.get<Staircase>({ -3, 7, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.get<Staircase>({ -3, 7, 0 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -2, 8, 0 }, new Staircase);
  grid.get<Staircase>({ -2, 8, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.get<Staircase>({ -2, 8, 0 })->orientation.roll = Gm_PI / 2.f;

  // Front to right staircase
  grid.set({ 2, 4, 4 }, new Staircase);
  grid.get<Staircase>({ 2, 4, 4 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 3, 4, 3 }, new Staircase);
  grid.get<Staircase>({ 3, 4, 3 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 4, 4, 2 }, new Staircase);
  grid.get<Staircase>({ 4, 4, 2 })->orientation.roll = -Gm_PI / 2.f;

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.triggers.set(coordinates, trigger);
  };
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
        staircase.rotation.x = stairs->orientation.pitch;
        // @todo use proper model orientation to avoid the -PI/2 offset here
        staircase.rotation.y = -Gm_PI / 2.f + stairs->orientation.yaw;
        staircase.rotation.z = -stairs->orientation.roll;

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
  addOrientationTestLayout(params());
  addParticles(params());

  addEntityObjects(params());
  addInvisibleEntityIndicators(params());

  auto& light = createLight(LightType::POINT_SHADOWCASTER);

  light.color = Vec3f(1.f, 0.8f, 0.4f);
  light.position = gridCoordinatesToWorldPosition({ 0, 4, 0 });
  light.radius = 500.f;
  light.isStatic = true;

  camera.position = gridCoordinatesToWorldPosition({ 0, 4, 0 });
}