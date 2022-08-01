#include <functional>
#include <string>
#include <vector>

#include "Gamma.h"

#include "game_world.h"
#include "game_state.h"
#include "build_flags.h"

#define CREATE(m) []() { return m; }
#define CONFIGURE [](Mesh& m)

using namespace Gamma;

struct MeshConfig {
  std::string meshName;
  u16 maxInstances;
  std::function<Mesh*()> create;
  std::function<void(Mesh&)> configure = nullptr;
};

const static std::vector<MeshConfig> meshConfigs = {
  {
    "dirt-floor", 0xffff, CREATE(Mesh::Plane(2)),
    CONFIGURE {
      m.texture = "./game/textures/dirt-floor.png";
      m.normalMap = "./game/textures/dirt-normals.png";
    }
  },
  {
    "dirt-wall", 0xffff, CREATE(Mesh::Model("./game/models/wall.obj")),
    CONFIGURE {
      m.texture = "./game/textures/dirt-wall.png";
      m.normalMap = "./game/textures/dirt-normals.png";
    }
  },
  {
    "water", 1000, CREATE(Mesh::Plane(2)),
    CONFIGURE {
      m.type = MeshType::WATER;
    }
  },
  {
    "rock", 1000, CREATE(Mesh::Model("./game/models/rock.obj"))
  },
  {
    "arch", 1000, CREATE(Mesh::Model("./game/models/arch.obj")),
    CONFIGURE {
      m.texture = "./game/textures/wood.png";
      m.normalMap = "./game/textures/wood-normals.png";
    }
  },
  {
    "arch-vines", 1000, CREATE(Mesh::Model("./game/models/arch-vines.obj")),
    CONFIGURE {
      m.type = MeshType::FOLIAGE;
      m.foliage.type = FoliageType::LEAF;
      m.foliage.speed = 3.f;
    }
  }
};

void addMeshes(Globals) {
  // Grid entity objects
  addMesh("ground", 0xffff, Mesh::Cube());
  mesh("ground")->canCastShadows = false;
  addMesh("staircase", 0xffff, Mesh::Model("./game/models/staircase.obj"));
  addMesh("switch", 1000, Mesh::Model("./game/models/switch.obj"));

  // Lunar Garden
  for (auto& config : meshConfigs) {
    addMesh(config.meshName, config.maxInstances, config.create());

    if (config.configure != nullptr) {
      config.configure(*mesh(config.meshName));
    }
  }

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