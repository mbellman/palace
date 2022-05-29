#pragma once

#include <filesystem>
#include <functional>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

#include "system/camera.h"
#include "system/entities.h"
#include "system/InputSystem.h"
#include "system/Signaler.h"
#include "system/traits.h"
#include "system/type_aliases.h"

#define addMesh(meshName, maxInstances, mesh) Gm_AddMesh(context, meshName, maxInstances, mesh);
#define addProbe(probeName, position) Gm_AddProbe(context, probeName, position);
#define createLight(type) Gm_CreateLight(context, type);
#define createObjectFrom(meshName) Gm_CreateObjectFrom(context, meshName);
#define commit(object) Gm_Commit(context, object);
#define pointCamera(...) Gm_PointCamera(context, __VA_ARGS__);
#define useFrustumCulling(...) Gm_UseFrustumCulling(context, __VA_ARGS__);
#define useLodByDistance(distance, ...) Gm_UseLodByDistance(context, distance, __VA_ARGS__);

struct GmContext;

struct GmSceneStats {
  Gamma::uint32 verts = 0;
  Gamma::uint32 tris = 0;
};

struct GmScene {
  Gamma::Camera camera;
  Gamma::InputSystem input;
  std::vector<Gamma::Mesh*> meshes;
  std::vector<Gamma::Light> lights;
  std::map<std::string, Gamma::Mesh*> meshMap;
  std::map<std::string, Gamma::Vec3f> probeMap;
  std::map<std::string, Gamma::ObjectRecord> objectStore;
  Gamma::Vec3f freeCameraVelocity = Gamma::Vec3f(0.0f);
  Gamma::uint16 runningMeshId = 0;
  Gamma::uint32 frame = 0;
  float runningTime = 0.0f;
};

const GmSceneStats Gm_GetSceneStats(GmContext* context);
void Gm_AddMesh(GmContext* context, const std::string& meshName, Gamma::uint16 maxInstances, Gamma::Mesh* mesh);
void Gm_AddProbe(GmContext* context, const std::string& probeName, const Gamma::Vec3f& position);
Gamma::Light& Gm_CreateLight(GmContext* context, Gamma::LightType type);
void Gm_UseSceneFile(GmContext* context, const std::string& filename);
Gamma::Object& Gm_CreateObjectFrom(GmContext* context, const std::string& meshName);
void Gm_Commit(GmContext* context, const Gamma::Object& object);
void Gm_PointCamera(GmContext* context, const Gamma::Object& object, bool upsideDown = false);
void Gm_PointCamera(GmContext* context, const Gamma::Vec3f& position, bool upsideDown = false);
void Gm_HandleFreeCameraMode(GmContext* context, float dt);
void Gm_UseFrustumCulling(GmContext* context, const std::initializer_list<std::string>& meshNames);
void Gm_UseLodByDistance(GmContext* context, float distance, const std::initializer_list<std::string>& meshNames);