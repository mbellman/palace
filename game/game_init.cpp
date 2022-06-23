#include "Gamma.h"

#include "game_init.h"
#include "orientation_system.h"
#include "grid_utilities.h"
#include "world_system.h"
#include "editor_system.h"
#include "game_macros.h"
#include "game_state.h"
#include "build_flags.h"

using namespace Gamma;

static void addKeyHandlers(args()) {
  auto& input = getInput();

  input.on<MouseButtonEvent>("mousedown", [&](const MouseButtonEvent& event) {
    if (!SDL_GetRelativeMouseMode()) {
      SDL_SetRelativeMouseMode(SDL_TRUE);
    }
  });

  input.on<Key>("keyup", [context, &state](Key key) {
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

static void addOrientationTestLayout(args()) {
  auto& grid = state.world.grid;
  auto& triggers = state.world.triggers;

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.triggers.set(coordinates, trigger);
  };

  // Bottom area
  setStaticEntityOverRange<Ground>(params(), { -4, -1, -4 }, { 4, -1, 4 });
  setStaticEntityOverRange<WalkableSpace>(params(), { -4, 1, -4 }, { 4, 1, 4 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { -5, -3, -5 }, { 5, -3, 5 });

  // Left area
  setStaticEntityOverRange<Ground>(params(), { -5, -1, -4 }, { -5, 8, 4 });
  setStaticEntityOverRange<WalkableSpace>(params(), { -3, 1, -4 }, { -3, 8, 4 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { -7, -1, -5 }, { -7, 9, 5 });

  // Right area
  setStaticEntityOverRange<Ground>(params(), { 5, -1, -4 }, { 5, 8, 4 });
  setStaticEntityOverRange<WalkableSpace>(params(), { 3, 1, -4 }, { 3, 8, 4 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { 7, -1, -5 }, { 7, 9, 5 });
  
  // Top area
  setStaticEntityOverRange<Ground>(params(), { -5, 9, -4 }, { 5, 9, 4 });
  setStaticEntityOverRange<WalkableSpace>(params(), { -4, 7, -4 }, { 4, 7, 4 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { -5, 11, -5 }, { 5, 11, 5 });

  // Back area
  setStaticEntityOverRange<Ground>(params(), { -5, -1, -5 }, { 5, 9, -5 });
  setStaticEntityOverRange<WalkableSpace>(params(), { -4, 0, -3 }, { 4, 8, -3 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { -5, -1, -7 }, { 5, 9, -7 });

  // Front area
  setStaticEntityOverRange<Ground>(params(), { -5, -1, 5 }, { 5, 9, 5 });
  setStaticEntityOverRange<WalkableSpace>(params(), { -4, 0, 3 }, { 4, 8, 3 });
  // Outdoors
  setStaticEntityOverRange<WalkableSpace>(params(), { -5, -1, 7 }, { 5, 9, 7 });

  // Pathway outdoors
  grid.clear({ 3, 1, -5 });
  grid.clear({ 3, 0, -5 });
  grid.set({ 3, 1, -5 }, new WalkableSpace);
  grid.set({ 3, -1, -6 }, new Ground);
  grid.set({ 3, -1, -7 }, new Ground);
  grid.set({ 2, -1, -7 }, new Ground);
  grid.set({ 3, 1, -6 }, new WalkableSpace);
  grid.set({ 3, 1, -7 }, new WalkableSpace);
  grid.set({ 2, 1, -7 }, new WalkableSpace);

  createWorldOrientationChange({ 3, 1, -7 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 2, 1, -7 }, NEGATIVE_Z_UP);

  // Back to bottom staircase outdoors
  grid.set({ 0, -1, -5 }, new Staircase);
  grid.get<Staircase>({ 0, -1, -5 })->orientation.pitch = Gm_PI + Gm_PI / 2.f;

  createWorldOrientationChange({ 0, -1, -6 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ 0, -2, -5 }, NEGATIVE_Y_UP);

  // Back to top staircase outdoors
  grid.set({ 0, 9, -5 }, new Staircase);

  createWorldOrientationChange({ 0, 9, -6 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ 0, 10, -5 }, POSITIVE_Y_UP);

  // Back to left staircase outdoors
  grid.set({ -5, 4, -5 }, new Staircase);
  grid.get<Staircase>({ -5, 4, -5 })->orientation.roll = -Gm_PI / 2.f;

  createWorldOrientationChange({ -5, 4, -6 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ -6, 4, -5 }, NEGATIVE_X_UP);

  // Back to right staircase outdoors
  grid.set({ 5, 4, -5 }, new Staircase);
  grid.get<Staircase>({ 5, 4, -5 })->orientation.roll = Gm_PI / 2.f;

  createWorldOrientationChange({ 5, 4, -6 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ 6, 4, -5 }, POSITIVE_X_UP);

  // Top to left staircase outdoors
  grid.set({ -5, 9, 0 }, new Staircase);
  grid.get<Staircase>({ -5, 9, 0 })->orientation.yaw = Gm_PI / 2.f;

  createWorldOrientationChange({ -5, 10, 0 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ -6, 9, 0 }, NEGATIVE_X_UP);

  // Top to right staircase outdoors
  grid.set({ 5, 9, 0 }, new Staircase);
  grid.get<Staircase>({ 5, 9, 0 })->orientation.yaw = -Gm_PI / 2.f;

  createWorldOrientationChange({ 5, 10, 0 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 6, 9, 0 }, POSITIVE_X_UP);

  // Top to front staircase outdoors
  grid.set({ 0, 9, 5 }, new Staircase);
  grid.get<Staircase>({ 0, 9, 5 })->orientation.pitch = Gm_PI / 2.f;

  createWorldOrientationChange({ 0, 10, 5 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 0, 9, 6 }, POSITIVE_Z_UP);

  // Front to bottom staircase outdoors
  grid.set({ 0, -1, 5 }, new Staircase);
  grid.get<Staircase>({ 0, -1, 5 })->orientation.pitch = Gm_PI;

  createWorldOrientationChange({ 0, -1, 6 }, POSITIVE_Z_UP);
  createWorldOrientationChange({ 0, -2, 5 }, NEGATIVE_Y_UP);

  // Bottom to right staircase outdoors
  grid.set({ 5, -1, 0 }, new Staircase);
  grid.get<Staircase>({ 5, -1, 0 })->orientation.roll = Gm_PI / 2.f;
  grid.get<Staircase>({ 5, -1, 0 })->orientation.yaw = -Gm_PI / 2.f;

  createWorldOrientationChange({ 5, -2, 0 }, NEGATIVE_Y_UP);
  createWorldOrientationChange({ 6, -1, 0 }, POSITIVE_X_UP);

  // Bottom to left staircase outdoors
  grid.set({ -5, -1, 0 }, new Staircase);
  grid.get<Staircase>({ -5, -1, 0 })->orientation.roll = -Gm_PI / 2.f;
  grid.get<Staircase>({ -5, -1, 0 })->orientation.yaw = Gm_PI / 2.f;

  createWorldOrientationChange({ -5, -2, 0 }, NEGATIVE_Y_UP);
  createWorldOrientationChange({ -6, -1, 0 }, NEGATIVE_X_UP);

  // Right to front staircase outdoors
  grid.set({ 5, 4, 5 }, new Staircase);
  grid.get<Staircase>({ 5, 4, 5 })->orientation.pitch = Gm_PI / 2.f;
  grid.get<Staircase>({ 5, 4, 5 })->orientation.yaw = -Gm_PI / 2.f;

  createWorldOrientationChange({ 6, 4, 5 }, POSITIVE_X_UP);
  createWorldOrientationChange({ 5, 4, 6 }, POSITIVE_Z_UP);

  // Left to front staircase outdoors
  grid.set({ -5, 4, 5 }, new Staircase);
  grid.get<Staircase>({ -5, 4, 5 })->orientation.pitch = Gm_PI / 2.f;
  grid.get<Staircase>({ -5, 4, 5 })->orientation.yaw = Gm_PI / 2.f;

  createWorldOrientationChange({ -6, 4, 5 }, NEGATIVE_X_UP);
  createWorldOrientationChange({ -5, 4, 6 }, POSITIVE_Z_UP);

  // Bottom to front staircase
  grid.set({ 0, 0, 2 }, new Staircase);
  grid.set({ 0, 1, 3 }, new Staircase);
  grid.set({ 0, 2, 4 }, new Staircase);

  createWorldOrientationChange({ 0, 1, 2 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 0, 2, 3 }, NEGATIVE_Z_UP);

  // Bottom to left staircase
  grid.set({ -2, 0, 0 }, new Staircase);
  grid.get<Staircase>({ -2, 0, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.set({ -3, 1, 0 }, new Staircase);
  grid.get<Staircase>({ -3, 1, 0 })->orientation.yaw = -Gm_PI / 2.f;
  grid.set({ -4, 2, 0 }, new Staircase);
  grid.get<Staircase>({ -4, 2, 0 })->orientation.yaw = -Gm_PI / 2.f;

  createWorldOrientationChange({ -2, 1, 0 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ -3, 2, 0 }, POSITIVE_X_UP);

  // Bottom to right staircase
  grid.set({ 2, 0, 0 }, new Staircase);
  grid.get<Staircase>({ 2, 0, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.set({ 3, 1, 0 }, new Staircase);
  grid.get<Staircase>({ 3, 1, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.set({ 4, 2, 0 }, new Staircase);
  grid.get<Staircase>({ 4, 2, 0 })->orientation.yaw = Gm_PI / 2.f;

  createWorldOrientationChange({ 2, 1, 0 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 3, 2, 0 }, NEGATIVE_X_UP);

  // Bottom to back staircase
  grid.set({ 0, 0, -2 }, new Staircase);
  grid.get<Staircase>({ 0, 0, -2 })->orientation.yaw = Gm_PI;
  grid.set({ 0, 1, -3 }, new Staircase);
  grid.get<Staircase>({ 0, 1, -3 })->orientation.yaw = Gm_PI;
  grid.set({ 0, 2, -4 }, new Staircase);
  grid.get<Staircase>({ 0, 2, -4 })->orientation.yaw = Gm_PI;

  createWorldOrientationChange({ 0, 1, -2 }, POSITIVE_Y_UP);
  createWorldOrientationChange({ 0, 2, -3 }, POSITIVE_Z_UP);

  // Left to front staircase
  grid.set({ -4, 4, 2 }, new Staircase);
  grid.get<Staircase>({ -4, 4, 2 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -3, 4, 3 }, new Staircase);
  grid.get<Staircase>({ -3, 4, 3 })->orientation.roll = Gm_PI / 2.f;
  grid.set({ -2, 4, 4 }, new Staircase);
  grid.get<Staircase>({ -2, 4, 4 })->orientation.roll = Gm_PI / 2.f;

  createWorldOrientationChange({ -2, 4, 3 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ -3, 4, 2 }, POSITIVE_X_UP);

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

  createWorldOrientationChange({ -3, 4, -2 }, POSITIVE_X_UP);
  createWorldOrientationChange({ -2, 4, -3 }, POSITIVE_Z_UP);

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

  createWorldOrientationChange({ -3, 6, 0 }, POSITIVE_X_UP);
  createWorldOrientationChange({ -2, 7, 0 }, NEGATIVE_Y_UP);

  // Front to right staircase
  grid.set({ 2, 4, 4 }, new Staircase);
  grid.get<Staircase>({ 2, 4, 4 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 3, 4, 3 }, new Staircase);
  grid.get<Staircase>({ 3, 4, 3 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 4, 4, 2 }, new Staircase);
  grid.get<Staircase>({ 4, 4, 2 })->orientation.roll = -Gm_PI / 2.f;

  createWorldOrientationChange({ 3, 4, 2 }, NEGATIVE_X_UP);
  createWorldOrientationChange({ 2, 4, 3 }, NEGATIVE_Z_UP);

  // Front to top staircase
  grid.set({ 0, 6, 4 }, new Staircase);
  grid.get<Staircase>({ 0, 6, 4 })->orientation.pitch = -Gm_PI / 2.f;
  grid.set({ 0, 7, 3 }, new Staircase);
  grid.get<Staircase>({ 0, 7, 3 })->orientation.pitch = -Gm_PI / 2.f;
  grid.set({ 0, 8, 2 }, new Staircase);
  grid.get<Staircase>({ 0, 8, 2 })->orientation.pitch = -Gm_PI / 2.f;

  createWorldOrientationChange({ 0, 6, 3 }, NEGATIVE_Z_UP);
  createWorldOrientationChange({ 0, 7, 2 }, NEGATIVE_Y_UP);

  // Right to top staircase
  grid.set({ 4, 6, 0 }, new Staircase);
  grid.get<Staircase>({ 4, 6, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.get<Staircase>({ 4, 6, 0 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 3, 7, 0 }, new Staircase);
  grid.get<Staircase>({ 3, 7, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.get<Staircase>({ 3, 7, 0 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 2, 8, 0 }, new Staircase);
  grid.get<Staircase>({ 2, 8, 0 })->orientation.yaw = Gm_PI / 2.f;
  grid.get<Staircase>({ 2, 8, 0 })->orientation.roll = -Gm_PI / 2.f;

  createWorldOrientationChange({ 3, 6, 0 }, NEGATIVE_X_UP);
  createWorldOrientationChange({ 2, 7, 0 }, NEGATIVE_Y_UP);

  // Back to top staircase
  grid.set({ 0, 6, -4 }, new Staircase);
  grid.get<Staircase>({ 0, 6, -4 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 0, 6, -4 })->orientation.pitch = Gm_PI / 2.f;
  grid.set({ 0, 7, -3 }, new Staircase);
  grid.get<Staircase>({ 0, 7, -3 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 0, 7, -3 })->orientation.pitch = Gm_PI / 2.f;
  grid.set({ 0, 8, -2 }, new Staircase);
  grid.get<Staircase>({ 0, 8, -2 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 0, 8, -2 })->orientation.pitch = Gm_PI / 2.f;

  createWorldOrientationChange({ 0, 7, -2 }, NEGATIVE_Y_UP);
  createWorldOrientationChange({ 0, 6, -3 }, POSITIVE_Z_UP);

  // Back to right staircase
  grid.set({ 2, 4, -4 }, new Staircase);
  grid.get<Staircase>({ 2, 4, -4 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 2, 4, -4 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 3, 4, -3 }, new Staircase);
  grid.get<Staircase>({ 3, 4, -3 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 3, 4, -3 })->orientation.roll = -Gm_PI / 2.f;
  grid.set({ 4, 4, -2 }, new Staircase);
  grid.get<Staircase>({ 4, 4, -2 })->orientation.yaw = Gm_PI;
  grid.get<Staircase>({ 4, 4, -2 })->orientation.roll = -Gm_PI / 2.f;

  createWorldOrientationChange({ 3, 4, -2 }, NEGATIVE_X_UP);
  createWorldOrientationChange({ 2, 4, -3 }, POSITIVE_Z_UP);
}

static void addParticles(args()) {
  addMesh("particles", 1000, Mesh::Particles());

  auto& particles = mesh("particles")->particleSystem;

  particles.spread = 100.f;
  particles.speedVariation = 5.f;
  particles.deviation = 10.f;
  particles.sizeVariation = 3.f;
  particles.medianSize = 5.f;
  particles.spawn = gridCoordinatesToWorldPosition({ 0, 4, 0 });
}

static void addMeshes(args()) {
  // Static entity objects
  addMesh("ground-tile", 0xffff, Mesh::Cube());
  addMesh("staircase", 0xffff, Mesh::Model("./game/models/staircase/model.obj"));

  // Entity/trigger indicators
  auto totalEntities = (s16)state.world.grid.size();
  auto totalTriggers = (s16)state.world.triggers.size();

  addMesh("entity-indicator", totalEntities, Mesh::Cube());
  addMesh("trigger-indicator", totalTriggers, Mesh::Cube());
}

static void addEntityObjects(args()) {
  auto& grid = state.world.grid;

  for (auto& [ coordinates, entity ] : grid) {
    createObjectFromCoordinates(params(), coordinates);
  }

  // @todo move to editor_system; replace preview object
  // whenever switching current selected entity type
  auto& preview = createObjectFrom("ground-tile");

  save("preview", preview);
}

static void addTriggerEntityIndicators(args()) {
  for (auto& [ coordinates, trigger ] : state.world.triggers) {
    switch (trigger->type) {
      case WORLD_ORIENTATION_CHANGE: {
        auto& indicator = createObjectFrom("trigger-indicator");

        indicator.position = gridCoordinatesToWorldPosition(coordinates);
        indicator.rotation = Vec3f(Gm_PI / 4.f, Gm_PI / 4.f, 0);
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

  #if DEVELOPMENT == 1
    input.on<KeyboardEvent>("keyup", [context, &state](const KeyboardEvent& event) {
      auto key = event.key;

      // Toggle free camera mode
      if (key == Key::C) {
        if (Gm_IsFlagEnabled(FREE_CAMERA_MODE)) {
          Gm_DisableFlags(FREE_CAMERA_MODE);
        } else {
          Gm_EnableFlags(FREE_CAMERA_MODE);
        }

        if (!Gm_IsFlagEnabled(FREE_CAMERA_MODE)) {
          // @todo do a sweep of neighboring coordinates and
          // place the camera at the closest walkable tile
          getCamera().position = gridCoordinatesToWorldPosition(state.lastGridCoordinates);
        }
      }

      // Toggle world editor
      if (key == Key::E) {
        state.editor.enabled = !state.editor.enabled;

        if (!state.editor.enabled) {
          object("preview").scale = 0.f;
          commit(object("preview"));
        }
      }

      // Toggle ranged tile placement
      if (key == Key::R) {
        state.editor.useRange = !state.editor.useRange;

        if (!state.editor.useRange) {
          state.editor.rangeFromSelected = false;
        }
      }

      // Toggle entity/trigger indicators
      if (key == Key::I) {
        auto& entityIndicators = *mesh("entity-indicator");
        auto& triggerIndicators = *mesh("trigger-indicator");

        entityIndicators.disabled = !entityIndicators.disabled;
        triggerIndicators.disabled = !triggerIndicators.disabled;

        context->renderer->resetShadowMaps();
      }
    });

    input.on<KeyboardEvent>("keydown", [context, &state, &input](const KeyboardEvent& event) {
      auto key = event.key;

      if (input.isKeyHeld(Key::CONTROL) && key == Key::Z && state.editor.enabled) {
        undoPreviousEditAction(params());
      }
    });

    input.on<MouseButtonEvent>("mousedown", [context, &state](const MouseButtonEvent& event) {
      if (SDL_GetRelativeMouseMode() && state.editor.enabled) {
        // @todo clean up
        if (state.editor.useRange) {
          if (state.editor.rangeFromSelected) {
            fillStaticEntitiesWithinCurrentRange(params());
          } else {
            selectRangeFrom(params());
          }
        } else {
          tryPlacingStaticEntity(params());
        }
      }
    });
  #endif

  addKeyHandlers(params());
  addOrientationTestLayout(params());
  addParticles(params());

  addMeshes(params());
  addEntityObjects(params());

  #if DEVELOPMENT == 1
    addTriggerEntityIndicators(params());
  #endif

  auto& light = createLight(POINT_SHADOWCASTER);

  light.color = Vec3f(1.f, 0.8f, 0.4f);
  light.position = gridCoordinatesToWorldPosition({ 0, 4, 0 });
  light.radius = 500.f;
  light.isStatic = true;

  auto& sunlight = createLight(DIRECTIONAL_SHADOWCASTER);

  sunlight.direction = Vec3f(0.3f, 0.5f, -1.f).invert();
  sunlight.color = Vec3f(1.f, 0.7f, 0.2f);

  camera.position = gridCoordinatesToWorldPosition({ 0, 1, 0 });
}