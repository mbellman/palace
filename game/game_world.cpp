#include "Gamma.h"

#include "game_world.h"
#include "game_state.h"
#include "build_flags.h"

using namespace Gamma;

void addMeshes(Globals) {
  // Grid entity objects
  addMesh("ground", 0xffff, Mesh::Cube());
  mesh("ground")->canCastShadows = false;
  addMesh("staircase", 0xffff, Mesh::Model("./game/models/staircase.obj"));
  addMesh("switch", 1000, Mesh::Model("./game/models/switch.obj"));

  // Lunar Garden
  addMesh("dirt-floor", 0xffff, Mesh::Plane(2));
  mesh("dirt-floor")->texture = "./game/textures/dirt-floor.png";
  mesh("dirt-floor")->normalMap = "./game/textures/dirt-normals.png";

  addMesh("dirt-wall", 0xffff, Mesh::Model("./game/models/wall.obj"));
  mesh("dirt-wall")->texture = "./game/textures/dirt-wall.png";
  mesh("dirt-wall")->normalMap = "./game/textures/dirt-normals.png";

  addMesh("water", 1000, Mesh::Plane(2));
  mesh("water")->type = MeshType::WATER;

  addMesh("rock", 1000, Mesh::Model("./game/models/rock.obj"));

  addMesh("arch", 1000, Mesh::Model("./game/models/arch.obj"));
  mesh("arch")->texture = "./game/textures/wood.png";
  mesh("arch")->normalMap = "./game/textures/wood-normals.png";

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

  addMesh("rosebush", 1000, Mesh::Model("./game/models/rosebush-leaves.obj"));
  mesh("rosebush")->type = MeshType::FOLIAGE;
  mesh("rosebush")->foliage.type = FoliageType::FLOWER;
  mesh("rosebush")->texture = "./game/textures/rose-leaves.png";
  mesh("rosebush")->normalMap = "./game/textures/rose-leaves-normals.png";

  addMesh("rosebush-flowers", 1000, Mesh::Model("./game/models/rosebush-flowers.obj"));
  mesh("rosebush-flowers")->type = MeshType::FOLIAGE;
  mesh("rosebush-flowers")->foliage.type = FoliageType::FLOWER;
  mesh("rosebush-flowers")->texture = "./game/textures/rose-petals.png";
  mesh("rosebush-flowers")->normalMap = "./game/textures/rose-petals-normals.png";
  mesh("rosebush-flowers")->emissivity = 0.25f;

  addMesh("gate-column", 250, Mesh::Model("./game/models/gate-column.obj"));
  mesh("gate-column")->texture = "./game/textures/brick.png";
  mesh("gate-column")->normalMap = "./game/textures/brick-normals.png";

  addMesh("gate", 250, Mesh::Model("./game/models/gate.obj"));

  // Palace of the Moon
  addMesh("tile-1", 0xffff, Mesh::Plane(2));
  mesh("tile-1")->texture = "./game/textures/tile-1.png";
  mesh("tile-1")->normalMap = "./game/textures/tile-1-normals.png";

  addMesh("column", 1000, Mesh::Model("./game/models/column.obj"));

  // Static world structures
  addMesh("potm-facade", 1, Mesh::Model("./game/models/potm-facade.obj"));

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

void addZones(Globals) {
  Zone z1 = {
    "Lunar Garden",
    { -5, -5, -20 },
    { 10, 7, 26 },
    {
      "dirt-floor",
      "dirt-wall",
      "water",
      "rock",
      "arch",
      "arch-vines",
      "tulips",
      "tulip-petals",
      "grass",
      "hedge",
      "stone-tile",
      "rosebush",
      "rosebush-flowers",
      "gate-column",
      "gate",
      "potm-facade"
    }
  };

  Zone z2 = {
    "Palace of the Moon",
    { -17, -32, 6 },
    { 17, 27, 67 },
    {
      "tile-1",
      "column"
    }
  };

  state.world.zones.push_back(z1);
  state.world.zones.push_back(z2);
}