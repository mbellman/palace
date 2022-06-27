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

static void addKeyHandlers(Globals) {
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

#if DEVELOPMENT == 1
  // @todo move this stuff to editor_system
  #include <fstream>

  #define serialize3Vector(value) std::to_string(value.x) + "," + std::to_string(value.y) + "," + std::to_string(value.z)

  // Wrap a value to within the [0, TAU] range
  #define wrap(n) n = n > Gm_TAU ? n - Gm_TAU : n < 0.f ? n + Gm_TAU : n

  static void saveEditorWorldData(Globals) {
    std::string serialized;

    serialized += "ground\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == GROUND) {
        serialized += serialize3Vector(coordinates) + "\n";
      }
    }

    serialized += "staircase\n";

    for (auto& [ coordinates, entity ] : state.world.grid) {
      if (entity->type == STAIRCASE) {
        auto orientationAsVec3f = ((Staircase*)entity)->orientation.toVec3f();

        wrap(orientationAsVec3f.x);
        wrap(orientationAsVec3f.y);
        wrap(orientationAsVec3f.z);

        serialized += serialize3Vector(coordinates) + ",";
        serialized += serialize3Vector(orientationAsVec3f) + "\n";
      }
    }

    Gm_WriteFileContents("./game/world/raw_data.txt", serialized);
  }

  static void loadEditorWorldData(Globals) {
    auto& grid = state.world.grid;
    StaticEntityType currentEntityType;
    std::ifstream file("./game/world/raw_data.txt");

    if (file.fail()) {
      return;
    }

    std::string line;

    while (std::getline(file, line)) {
      if (line == "ground") {
        currentEntityType = GROUND;
      } else if (line == "staircase") {
        currentEntityType = STAIRCASE;
      } else {
        auto coords = Gm_SplitString(line, ",");
        auto x = (s16)stoi(coords[0]);
        auto y = (s16)stoi(coords[1]);
        auto z = (s16)stoi(coords[2]);

        switch (currentEntityType) {
          case GROUND:
            grid.set({ x, y, z }, new Ground);
            break;
          case STAIRCASE: {
            auto pitch = (float)stof(coords[3]);
            auto yaw = (float)stof(coords[4]);
            auto roll = (float)stof(coords[5]);
            auto* staircase = new Staircase;

            staircase->orientation = { roll, pitch, yaw };

            grid.set({ x, y, z }, staircase);
            break;
          }
        }
      }
    }

    file.close();
  }
#endif

// @todo move to a separate file
static void addOrientationTestLayout(Globals) {
  auto& grid = state.world.grid;
  auto& triggers = state.world.triggers;

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.triggers.set(coordinates, trigger);
  };

  // Bottom area
  setStaticEntityOverRange<Ground>(globals, { -4, -1, -4 }, { 4, -1, 4 });

  // Left area
  setStaticEntityOverRange<Ground>(globals, { -5, -1, -4 }, { -5, 8, 4 });

  // Right area
  setStaticEntityOverRange<Ground>(globals, { 5, -1, -4 }, { 5, 8, 4 });
  
  // Top area
  setStaticEntityOverRange<Ground>(globals, { -5, 9, -4 }, { 5, 9, 4 });

  // Back area
  setStaticEntityOverRange<Ground>(globals, { -5, -1, -5 }, { 5, 9, -5 });

  // Front area
  setStaticEntityOverRange<Ground>(globals, { -5, -1, 5 }, { 5, 9, 5 });

  // Pathway outdoors
  grid.clear({ 3, 1, -5 });
  grid.clear({ 3, 0, -5 });
  grid.set({ 3, -1, -6 }, new Ground);
  grid.set({ 3, -1, -7 }, new Ground);
  grid.set({ 2, -1, -7 }, new Ground);

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

  auto& light = createLight(POINT_SHADOWCASTER);

  light.color = Vec3f(1.f, 0.8f, 0.4f);
  light.position = gridCoordinatesToWorldPosition({ 0, 4, 0 });
  light.radius = 500.f;
  light.isStatic = true;
}

static void addParticles(Globals) {
  addMesh("particles", 1000, Mesh::Particles());

  auto& particles = mesh("particles")->particleSystem;

  particles.spread = 100.f;
  particles.speedVariation = 5.f;
  particles.deviation = 10.f;
  particles.sizeVariation = 3.f;
  particles.medianSize = 5.f;
  particles.spawn = gridCoordinatesToWorldPosition({ 0, 4, 0 });
}

static void addMeshes(Globals) {
  // Static entity objects
  addMesh("ground", 0xffff, Mesh::Cube());
  addMesh("staircase", 0xffff, Mesh::Model("./game/models/staircase/model.obj"));

  #if DEVELOPMENT == 1
    // Entity/trigger indicators
    auto totalTriggers = (s16)state.world.triggers.size();

    addMesh("trigger-indicator", totalTriggers, Mesh::Cube());

    // Ranged placement preview
    addMesh("range-preview", 0xffff, Mesh::Cube());
  #endif
}

static void addEntityObjects(Globals) {
  auto& grid = state.world.grid;

  for (auto& [ coordinates, entity ] : grid) {
    createObjectFromCoordinates(globals, coordinates);
  }

  #if DEVELOPMENT == 1
    // @todo move to editor_system
    auto& preview = createObjectFrom("ground");
    
    preview.scale = 0.f;

    save("placement-preview", preview);
    commit(preview);
  #endif
}

static void addTriggerEntityIndicators(Globals) {
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

void initializeGame(Globals) {
  Gm_EnableFlags(GammaFlags::VSYNC);

  auto& input = getInput();
  auto& camera = getCamera();

  input.on<MouseMoveEvent>("mousemove", [context, &state](const MouseMoveEvent& event) {
    if (SDL_GetRelativeMouseMode()) {
      updateCameraFromMouseMoveEvent(globals, event);
    }
  });

  #if DEVELOPMENT == 1
    input.on<KeyboardEvent>("keydown", [context, &state, &input](const KeyboardEvent& event) {
      if (context->commander.isOpen()) {
        return;
      }

      auto key = event.key;

      // Undo editor actions
      if (state.editor.enabled && input.isKeyHeld(Key::CONTROL) && key == Key::Z) {
        undoPreviousEditAction(globals);
        saveEditorWorldData(globals);
      }

      // Toggle free camera mode
      if (key == Key::C) {
        if (Gm_IsFlagEnabled(FREE_CAMERA_MODE)) {
          Gm_DisableFlags(FREE_CAMERA_MODE);
        } else {
          Gm_EnableFlags(FREE_CAMERA_MODE);
        }

        if (!Gm_IsFlagEnabled(FREE_CAMERA_MODE)) {
          placeCameraAtClosestWalkableTile(globals);
        }
      }
    });

    input.on<KeyboardEvent>("keyup", [context, &state](const KeyboardEvent& event) {
      if (context->commander.isOpen()) {
        return;
      }

      auto key = event.key;

      // Toggle world editor
      if (key == Key::E) {
        state.editor.enabled = !state.editor.enabled;

        if (!state.editor.enabled) {
          object("placement-preview").scale = 0.f;
          commit(object("placement-preview"));
        }
      }

      // Toggle entity/trigger indicators
      if (key == Key::I) {
        auto& triggerIndicators = *mesh("trigger-indicator");

        triggerIndicators.disabled = !triggerIndicators.disabled;

        context->renderer->resetShadowMaps();
      }

      // Editor controls
      // @todo allow staircase orientations to be controlled, or heuristically determined
      if (state.editor.enabled) {
        // Toggle ranged tile placement
        if (key == Key::R) {
          state.editor.useRange = !state.editor.useRange;

          if (!state.editor.useRange) {
            state.editor.rangeFromSelected = false;

            objects("range-preview").reset();
          }
        }

        if (key == Key::NUM_0) state.editor.deleting = true;
        if (key == Key::NUM_1) setCurrentSelectedEntityType(globals, GROUND);
        if (key == Key::NUM_2) setCurrentSelectedEntityType(globals, STAIRCASE);

        if (key == Key::ARROW_UP) adjustCurrentEntityOrientation(globals, { 0, Gm_HALF_PI, 0 });
        if (key == Key::ARROW_DOWN) adjustCurrentEntityOrientation(globals, { 0, -Gm_HALF_PI, 0 });
        if (key == Key::ARROW_LEFT) adjustCurrentEntityOrientation(globals, { 0, 0, -Gm_HALF_PI });
        if (key == Key::ARROW_RIGHT) adjustCurrentEntityOrientation(globals, { 0, 0, Gm_HALF_PI });
      }
    });

    input.on<MouseButtonEvent>("mousedown", [context, &state](const MouseButtonEvent& event) {
      if (state.editor.enabled && SDL_GetRelativeMouseMode()) {
        // @todo create a mousedown handler in editor_system
        if (state.editor.useRange) {
          if (state.editor.rangeFromSelected) {
            handleEditorRangedClickAction(globals);
            saveEditorWorldData(globals);
          } else {
            selectRangeFrom(globals);
          }
        } else {
          handleEditorSingleTileClickAction(globals);
          saveEditorWorldData(globals);
        }
      }
    });
  #endif

  addKeyHandlers(globals);
  // addOrientationTestLayout(globals);

  #if DEVELOPMENT == 1
    loadEditorWorldData(globals);
  #endif

  addParticles(globals);

  addMeshes(globals);
  addEntityObjects(globals);

  #if DEVELOPMENT == 1
    addTriggerEntityIndicators(globals);
  #endif

  auto& sunlight = createLight(DIRECTIONAL_SHADOWCASTER);

  sunlight.direction = Vec3f(0.3f, 0.5f, -1.f).invert();
  sunlight.color = Vec3f(1.f, 0.7f, 0.2f);

  camera.position = gridCoordinatesToWorldPosition({ 0, 1, 0 });
}