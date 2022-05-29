#pragma once

#include <functional>
#include <string>
#include <vector>

#include "math/geometry.h"
#include "math/matrix.h"
#include "math/vector.h"
#include "system/ObjectPool.h"
#include "system/ObjLoader.h"
#include "system/packed_data.h"
#include "system/type_aliases.h"

#define UNUSED_LIGHT_INDEX -1

namespace Gamma {
  enum LightType {
    POINT,
    DIRECTIONAL,
    SPOT,
    POINT_SHADOWCASTER,
    DIRECTIONAL_SHADOWCASTER,
    SPOT_SHADOWCASTER
  };

  /**
   * Light
   * -----
   *
   * Defines a light source, which affects scene illumination
   * and color/reflective properties of illuminated surfaces.
   */
  struct Light {
    Vec3f position;
    float radius = 100.0f;
    Vec3f color = Vec3f(1.0f);
    float power = 1.0f;
    Vec3f direction = Vec3f(0.0f, -1.0f, 0.0f);
    float fov = 90.0f;
    uint32 type = LightType::POINT;
    bool isStatic = false;
    int id = UNUSED_LIGHT_INDEX;
    // @todo std::vector<uint32> shadowMapMeshes (?)
  };

  /**
   * ObjectRecord
   * ------------
   *
   * A unique identifier for an object which allows it
   * to be looked up on the appropriate Mesh and at its
   * corresponding index, with ID checks for referential
   * integrity.
   *
   * @size 8 bytes
   */
  struct ObjectRecord {
    uint16 meshIndex = 0;
    uint16 meshId = 0;
    // @todo uint32 id with 24 bits for id and 8 for generation,
    // allowing up to ~16.77 million objects per pool
    uint16 id = 0;
    uint16 generation = 0;
  };

  /**
   * Object
   * ------
   *
   * Objects are derived from Meshes, defining individual
   * instances of a Mesh distributed throughout a scene,
   * each with its own transformations.
   *
   * @size 48 bytes
   */
  struct Object {
    ObjectRecord _record;
    Vec3f position;
    Vec3f scale;
    Vec3f rotation;
    pVec4 color;
  };

  /**
   * A set of Mesh types, particular to all instances of a Mesh,
   * controlling the rendering priority and properties of those
   * instances.
   */
  enum MeshType {
    /**
     * @todo description
     */
    SKYBOX = 0x00,
    /**
     * @todo description
     */
    EMISSIVE = 0x50,
    /**
     * Defines a set of GPU-accelerated particles, with custom
     * parameters for controlling particle behavior.
     */
    PARTICLE_SYSTEM = 0xA0,
    /**
     * Defines translucent Meshes which refract and lens the
     * objects and geometry behind them.
     */
    REFRACTIVE = 0xF0,
    /**
     * Defines Meshes which reflect surrounding geometry in
     * screen space, or the sky where geometry is not reflected.
     */
    REFLECTIVE = 0xFA,
    /**
     * @todo description
     */
    PROBE_REFLECTOR = 0xFB,
    /**
     * Defines standard Meshes without any unique rendering properties.
     */
    NON_EMISSIVE = 0xFF
  };

  /**
   * MeshLod
   * -------
   *
   * Defines vertices/face elements and instances particular
   * to specific Mesh levels of detail.
   */
  struct MeshLod {
    /**
     * Defines the starting face element in the LOD model.
     */
    uint32 elementOffset = 0;
    /**
     * Defines the number of face elements in the LOD model.
     */
    uint32 elementCount = 0;
    /**
     * Defines the starting instance in the LOD group.
     */
    uint32 instanceOffset = 0;
    /**
     * Defines the number of instances in the LOD group.
     */
    uint32 instanceCount = 0;
    /**
     * Defines the starting vertex in the LOD model.
     */
    uint32 vertexOffset = 0;
    /**
     * Defines the number of vertices in the LOD model.
     */
    uint32 vertexCount = 0;
  };

  /**
   * ParticleSystem
   * --------------
   *
   * @todo description
   */
  struct ParticleSystem {
    Vec3f spawn;
    float spread;
    float minimumRadius;
    float medianSpeed;
    float speedVariation;
    float medianSize;
    float sizeVariation;
    float deviation;
    std::vector<Vec3f> path;
    bool isCircuit;
  };

  /**
   * Mesh
   * ----
   *
   * A Mesh serves as a static reference model from which
   * individual Objects can be created, where Objects
   * only contain transformation properties and other
   * instance-specific attributes.
   */
  struct Mesh {
    /**
     * The index of the mesh in a scene's Mesh array,
     * used for efficient mesh lookups.
     */
    uint16 index = 0;
    /**
     * A unique ID for the mesh. If a mesh retrieved
     * at a specific index in a scene's Mesh array
     * does not match an expected ID (e.g., if the
     * mesh structure has been recycled), the reference
     * should be considered stale.
     */
    uint16 id = 0;
    /**
     * Static mesh vertices in model space.
     */
    std::vector<Vertex> vertices;
    /**
     * Dynamic mesh vertices, based on the static vertices.
     * Remains empty unless transformGeometry() is used.
     */
    std::vector<Vertex> transformedVertices;
    /**
     * Vertex indices for each triangle face of the mesh,
     * defined in groups of three.
     */
    std::vector<uint32> faceElements;
    /**
     * The LOD groups for the Mesh, if applicable.
     *
     * @see MeshLod
     */
    std::vector<MeshLod> lods;
    /**
     * A collection of objects representing unique instances
     * of the mesh.
     */
    ObjectPool objects;
    /**
     * An optional albedo texture for the mesh.
     */
    std::string texture = "";
    /**
     * An optional normal map texture for the mesh.
     */
    std::string normalMap = "";
    /**
     * An optional specularity map texture for the mesh.
     */
    std::string specularityMap = "";
    /**
     * The probe name to use for probe reflectors.
     */
    std::string probe = "";
    /**
     * Defines the mesh type.
     *
     * @see MeshType
     */
    uint8 type = MeshType::NON_EMISSIVE;
    /**
     * Controls the maximum directional cascaded shadow
     * map that the mesh objects should be rendered to.
     */
    uint8 maxCascade = 3;
    /**
     * Controls whether the mesh's instances are rendered
     * to shadow maps, enabling them to cast shadows.
     */
    bool canCastShadows = true;
    /**
     * Configuration parameters for particle system meshes.
     */
    ParticleSystem particleSystem;

    static Mesh* Cube();
    static Mesh* Model(const char* path);
    static Mesh* Model(const std::vector<std::string>& paths);
    static Mesh* Particles();
    static Mesh* Plane(uint32 size, bool useLoopingTexture = false);
    // @todo Sphere(uint32 divisions)
    // @todo Cylinder(uint32 divisions)

    void transformGeometry(std::function<void(const Vertex&, Vertex&)> handler);
  };

  /**
   * Gm_FreeMesh
   * -----------
   */
  void Gm_FreeMesh(Mesh* mesh);
}