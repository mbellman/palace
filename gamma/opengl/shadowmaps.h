#pragma once

#include "math/matrix.h"
#include "math/vector.h"
#include "opengl/framebuffer.h"
#include "opengl/shader.h"
#include "system/camera.h"
#include "system/entities.h"

namespace Gamma {
  struct OpenGLBaseShadowMap {
    // @todo store a pointer to the light instead of the index
    uint32 lightId = UNUSED_LIGHT_INDEX;
    bool isRendered = false;
  };

  struct OpenGLDirectionalShadowMap : public OpenGLBaseShadowMap {
    OpenGLFrameBuffer buffer;

    OpenGLDirectionalShadowMap(const Light* light);
  };

  struct OpenGLPointShadowMap : public OpenGLBaseShadowMap {
    OpenGLCubeMap buffer;

    OpenGLPointShadowMap(const Light* light);
  };

  struct OpenGLSpotShadowMap : public OpenGLBaseShadowMap {
    OpenGLFrameBuffer buffer;

    OpenGLSpotShadowMap(const Light* light);
  };

  Matrix4f Gm_CreateCascadedLightViewMatrixGL(uint8 cascade, const Vec3f& lightDirection, const Camera& camera);
}