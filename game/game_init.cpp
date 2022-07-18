#include "Gamma.h"

#include "game_init.h"
#include "orientation_system.h"
#include "world_system.h"
#include "object_system.h"
#include "editor_system.h"
#include "grid_utilities.h"
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

// @todo move to a separate file
static void addOrientationTestLayout(Globals) {
  auto& grid = state.world.grid;

  // @todo define elsewhere
  auto createWorldOrientationChange = [context, &state](const GridCoordinates& coordinates, WorldOrientation target) {
    auto* trigger = new WorldOrientationChange;

    trigger->targetWorldOrientation = target;

    state.world.grid.set(coordinates, trigger);
  };

  // Bottom area
  setGridEntityOverRange<Ground>(globals, { -4, -1, -4 }, { 4, -1, 4 });

  // Left area
  setGridEntityOverRange<Ground>(globals, { -5, -1, -4 }, { -5, 8, 4 });

  // Right area
  setGridEntityOverRange<Ground>(globals, { 5, -1, -4 }, { 5, 8, 4 });
  
  // Top area
  setGridEntityOverRange<Ground>(globals, { -5, 9, -4 }, { 5, 9, 4 });

  // Back area
  setGridEntityOverRange<Ground>(globals, { -5, -1, -5 }, { 5, 9, -5 });

  // Front area
  setGridEntityOverRange<Ground>(globals, { -5, -1, 5 }, { 5, 9, 5 });

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
  particles.medianSpeed = 0.3f;
  particles.speedVariation = 0.15f;
  particles.deviation = 10.f;
  particles.sizeVariation = 3.f;
  particles.medianSize = 5.f;
  particles.spawn = gridCoordinatesToWorldPosition({ 2, -2, -7 });
}

static void addMeshes(Globals) {
  // Grid entity objects
  addMesh("ground", 0xffff, Mesh::Cube());
  addMesh("staircase", 0xffff, Mesh::Model("./game/models/staircase.obj"));
  addMesh("switch", 1000, Mesh::Model("./game/models/switch.obj"));

  // Dirt floor mesh
  addMesh("dirt-floor", 0xffff, Mesh::Plane(2));
  mesh("dirt-floor")->texture = "./game/textures/dirt-floor.png";
  mesh("dirt-floor")->normalMap = "./game/textures/dirt-normals.png";

  // Dirt wall mesh
  addMesh("dirt-wall", 0xffff, Mesh::Model("./game/models/wall.obj"));
  mesh("dirt-wall")->texture = "./game/textures/dirt-wall.png";
  mesh("dirt-wall")->normalMap = "./game/textures/dirt-normals.png";

  // Water plane mesh
  addMesh("water", 1000, Mesh::Plane(2));
  mesh("water")->type = MeshType::WATER;

  // Decorative mesh objects
  addMesh("rock", 1000, Mesh::Model("./game/models/rock.obj"));

  addMesh("arch", 1000, Mesh::Model("./game/models/arch.obj"));
  mesh("arch")->texture = "./game/textures/arch.png";

  addMesh("arch-vines", 1000, Mesh::Model("./game/models/arch-vines.obj"));
  mesh("arch-vines")->type = MeshType::FOLIAGE;
  mesh("arch-vines")->foliage.type = FoliageType::LEAF;
  mesh("arch-vines")->foliage.speed = 3.f;

  addMesh("tulips", 1000, Mesh::Model("./game/models/tulips.obj"));
  mesh("tulips")->type = MeshType::FOLIAGE;
  mesh("tulips")->foliage.type = FoliageType::FLOWER;

  addMesh("tulip-petals", 1000, Mesh::Model("./game/models/tulip-petals.obj"));
  mesh("tulip-petals")->type = MeshType::FOLIAGE;
  mesh("tulip-petals")->foliage.type = FoliageType::FLOWER;

  addMesh("grass", 1000, Mesh::Model("./game/models/grass.obj"));

  addMesh("hedge", 1000, Mesh::Model("./game/models/hedge.obj"));
  mesh("hedge")->texture = "./game/textures/hedge.png";
  mesh("hedge")->normalMap = "./game/textures/hedge-normals.png";

  addMesh("stone-tile", 1000, Mesh::Model("./game/models/stone-tile.obj"));

  #if DEVELOPMENT == 1
    // Trigger entity indicators
    addMesh("trigger-indicator", 0xffff, Mesh::Cube());
    mesh("trigger-indicator")->disabled = true;

    // Light indicators
    addMesh("light-indicator", 1000, Mesh::Cube());  // @todo Mesh::Sphere()
    mesh("light-indicator")->type = MeshType::EMISSIVE;
    mesh("light-indicator")->disabled = true;

    // Ranged placement preview
    addMesh("range-preview", 0xffff, Mesh::Cube());
  #endif
}

static void createGridEntityObjects(Globals) {
  auto& grid = state.world.grid;

  for (auto& [ coordinates, entity ] : grid) {
    createGridObjectFromCoordinates(globals, coordinates);
  }

  #if DEVELOPMENT == 1
    // @todo move to editor_system
    auto& preview = createObjectFrom("ground");
    
    preview.scale = 0.f;

    saveObject("entity-preview", preview);
    commit(preview);
  #endif
}

static void createLightIndicatorObjects(Globals) {
  for (auto* light : context->scene.lights) {
    if (light->type != DIRECTIONAL && light->type != DIRECTIONAL_SHADOWCASTER) {
      auto& indicator = createObjectFrom("light-indicator");

      indicator.scale = 1.5f;
      indicator.position = light->position;

      commit(indicator);
    }
  }
}

static void addSwitchEntityEffects(Globals) {
  addMesh("switch-particles", 500, Mesh::Particles());

  auto& switchParticles = mesh("switch-particles")->particleSystem;
  auto& switchLight = createLight(POINT);

  switchLight.color = Vec3f(1.f, 0.2f, 0.1f);
  switchLight.power = 0.f;

  switchParticles.spread = TILE_SIZE;
  switchParticles.medianSpeed = 0.5f;
  switchParticles.deviation = 2.f;
  switchParticles.path.push_back(Vec3f(0));
  switchParticles.path.push_back(Vec3f(0));
  switchParticles.isCircuit = false;

  saveLight("switch-light", &switchLight);
}

void initializeGame(Globals) {
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
        saveWorldGridData(globals);
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

    input.on<MouseWheelEvent>("mousewheel", [context, &state](const MouseWheelEvent& event) {
      if (
        !state.editor.enabled ||
        // Debounce wheel events since multiple sometimes fire in rapid succession
        getRunningTime() - state.editor.lastEntityChangeTime < 0.1f
      ) {
        return;
      }

      auto& editor = state.editor;
      auto maxEntityIndex = u8(editorEntityCycle.size() - 1);

      if (event.direction == MouseWheelEvent::UP) {
        if (editor.currentSelectedEntityIndex == 0) {
          editor.currentSelectedEntityIndex = maxEntityIndex;
        } else {
          editor.currentSelectedEntityIndex--;
        }
      }

      if (event.direction == MouseWheelEvent::DOWN) {
        if (editor.currentSelectedEntityIndex >= maxEntityIndex) {
          editor.currentSelectedEntityIndex = 0;
        } else {
          editor.currentSelectedEntityIndex++;
        }
      }

      auto currentSelectedEntityType = (EntityType)editorEntityCycle[editor.currentSelectedEntityIndex];

      setCurrentSelectedEntityType(globals, currentSelectedEntityType);

      editor.lastEntityChangeTime = getRunningTime();
    });

    input.on<KeyboardEvent>("keyup", [context, &state](const KeyboardEvent& event) {
      if (context->commander.isOpen()) {
        return;
      }

      auto key = event.key;
      auto& editor = state.editor;

      if (key == Key::E) {
        toggleEditor(globals);
      }

      if (key == Key::M) {
        mesh("ground")->disabled = !mesh("ground")->disabled;

        context->renderer->resetShadowMaps();
      }

      // Reset world orientation to +Y
      if (key == Key::O) {
        setWorldOrientation(globals, POSITIVE_Y_UP);
      }

      // Editor controls
      #define bindEditorWorldOrientationKey(keyCode, worldOrientation) if (key == keyCode) editor.currentSelectedWorldOrientation = worldOrientation

      bindEditorWorldOrientationKey(Key::NUM_1, POSITIVE_Y_UP);
      bindEditorWorldOrientationKey(Key::NUM_2, NEGATIVE_Y_UP);
      bindEditorWorldOrientationKey(Key::NUM_3, POSITIVE_X_UP);
      bindEditorWorldOrientationKey(Key::NUM_4, NEGATIVE_X_UP);
      bindEditorWorldOrientationKey(Key::NUM_5, POSITIVE_Z_UP);
      bindEditorWorldOrientationKey(Key::NUM_6, NEGATIVE_Z_UP);

      if (key == Key::F) {
        toggleMeshFinder(globals);
      }

      if (key == Key::L) {
        toggleLightFinder(globals);
      }

      // Editor controls while enabled
      if (editor.enabled) {
        // Toggle ranged tile placement
        if (key == Key::R) {
          editor.useRange = !editor.useRange;

          if (!editor.useRange) {
            editor.rangeFromSelected = false;

            objects("range-preview").reset();
          }
        }

        if (key == Key::G) {
          editor.snapMeshesToGrid = !editor.snapMeshesToGrid;
        }

        if (key == Key::BACKSPACE) {
          handleEditorDeletionAction(globals);
        }

        if (key == Key::NUM_0) {
          // Use the ground tile object for the deletion preview,
          // since a plain cube will do
          setCurrentSelectedEntityType(globals, GROUND);

          editor.deleting = true;
        }

        if (key == Key::ARROW_UP) adjustCurrentEntityOrientation(globals, { 0, Gm_HALF_PI, 0 });
        if (key == Key::ARROW_DOWN) adjustCurrentEntityOrientation(globals, { 0, -Gm_HALF_PI, 0 });
        if (key == Key::ARROW_LEFT) adjustCurrentEntityOrientation(globals, { 0, 0, -Gm_HALF_PI });
        if (key == Key::ARROW_RIGHT) adjustCurrentEntityOrientation(globals, { 0, 0, Gm_HALF_PI });
      }
    });

    input.on<MouseButtonEvent>("mousedown", [context, &state](const MouseButtonEvent& event) {
      if (state.editor.enabled && SDL_GetRelativeMouseMode()) {
        handleEditorClickAction(globals);
      }
    });

    context->commander.on<std::string>("command", [context, &state](const std::string& command) {
      #define parseVec3f(name, input) \
        auto components = Gm_SplitString(input, ",");\
        auto name = Vec3f(\
          stof(components[0]),\
          stof(components[1]),\
          stof(components[2])\
        )\

      // @todo handleCommand()
      if (Gm_StringStartsWith(command, "mesh")) {
        // @todo handleMeshCommand()
        auto parts = Gm_SplitString(command, " ");

        if (parts.size() > 1) {
          auto meshName = parts[1];

          createPlaceableMeshObjectFrom(globals, meshName);
        }
      } else if (Gm_StringStartsWith(command, "light")) {
        // @todo handleLightCommand()
        auto parts = Gm_SplitString(command, " ");
        auto lightTypeString = parts[1];
        LightType lightType = POINT;

        // @todo create a map for this
        if (lightTypeString == "point") {
          lightType = POINT;
        } else if (lightTypeString == "point_shadow") {
          // @todo
        } else if (lightTypeString == "spot") {
          // @todo
        } else if (lightTypeString == "spot_shadow") {
          // @todo
        }

        createPlaceableLight(globals, lightType);
      } else if (Gm_StringStartsWith(command, "radius")) {
        // @todo handleRadiusCommand()
        if (!state.editor.isPlacingLight) {
          return;
        }

        auto parts = Gm_SplitString(command, " ");

        if (parts.size() > 1) {
          auto radius = stof(parts[1]);

          state.editor.selectedLight->radius = radius;
        }
      } else if (Gm_StringStartsWith(command, "color")) {
        // @todo handleColorCommand()
        auto parts = Gm_SplitString(command, " ");

        if (parts.size() > 1) {
          parseVec3f(color, parts[1]);

          if (state.editor.isPlacingLight) {
            state.editor.selectedLight->color = color;
          } else if (state.editor.isPlacingMesh) {
            object("mesh-preview").color = color;

            commit(object("mesh-preview"));
          }
        }
      } else if (Gm_StringStartsWith(command, "rotate")) {
        // @todo handleRotateCommand()
        auto parts = Gm_SplitString(command, " ");

        if (parts.size() > 1) {
          parseVec3f(rotation, parts[1]);

          if (state.editor.isPlacingMesh) {
            object("mesh-preview").rotation = rotation;

            commit(object("mesh-preview"));
          }
        }
      } else if (Gm_StringStartsWith(command, "scale")) {
        // @todo handleScaleCommand()
        auto parts = Gm_SplitString(command, " ");

        if (parts.size() > 1) {
          auto scale = stof(parts[1]);

          if (state.editor.isPlacingMesh) {
            object("mesh-preview").scale = scale;

            commit(object("mesh-preview"));
          }
        }
      }
    });
  #endif

  addMeshes(globals);
  addSwitchEntityEffects(globals);
  addParticles(globals);  // @temporary
  addKeyHandlers(globals);
  // addOrientationTestLayout(globals);

  #if DEVELOPMENT == 1
    loadWorldGridData(globals);
    loadMeshData(globals);
    loadLightData(globals);

    createLightIndicatorObjects(globals);
  #else
    // @todo
  #endif

  createGridEntityObjects(globals);

  auto& moonlight = createLight(DIRECTIONAL_SHADOWCASTER);

  moonlight.direction = Vec3f(0.3f, 0.5f, 1.f).invert();
  moonlight.color = Vec3f(0.8f, 0.8f, 1.f);

  camera.position = gridCoordinatesToWorldPosition({ 2, -2, -14 });

  state.cameraStartPosition = camera.position;
  state.cameraTargetPosition = camera.position;

  Gm_EnableFlags(GammaFlags::VSYNC);
}