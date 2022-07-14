#version 460 core

uniform vec2 screenSize;
uniform sampler2D texColorAndDepth;
uniform mat4 matProjection;
uniform mat4 matView;
uniform mat4 matInverseProjection;
uniform mat4 matInverseView;
uniform vec3 cameraPosition;
uniform float time;

flat in vec3 fragColor;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;

#include "utils/gl.glsl";
#include "utils/conversion.glsl";
#include "utils/skybox.glsl";
#include "utils/helpers.glsl";

vec2 getPixelCoords() {
  return gl_FragCoord.xy / screenSize;
}

mat3 getTBNMatrix() {
  vec3 surfaceNormal = normalize(fragNormal);
  vec3 surfaceTangent = normalize(fragTangent);

  surfaceTangent = normalize(surfaceTangent - dot(surfaceTangent, surfaceNormal) * surfaceNormal);

  vec3 surfaceBitangent = cross(surfaceTangent, surfaceNormal);

  return mat3(surfaceTangent, surfaceBitangent, surfaceNormal);
}

float createRadialWave(float x, float y, float frequency, float offset) {
  const float PI = 3.141592;
  const float HALF_PI = PI * 0.5;

  float cx = 2.0 * (x - 0.5);
  float cy = 2.0 * (y - 0.5);
  float radius = sqrt(cx * cx + cy * cy);

  return sin(cx * HALF_PI + radius * PI * frequency + offset) * cos(cy * HALF_PI);
}

vec3 getNormal() {
  float u = fragUv.x;
  float v = fragUv.y;

  float x = (
    createRadialWave(u, v, 10.0, -time) * 0.3 +
    createRadialWave(u + 0.5, v - 0.3, 10.0, -time) * 0.6 +
    sin((u + (sin(v * 10.0) + sin(v * (6.0 + v))) * 0.05) * 20.0) * sin(time * 2.0) * 0.2 +
    sin((u + sin(v * 5.0) * 0.1) * 200.0 + time * 3.0) * 0.01
  );

  float y = (
    createRadialWave(v, u, 10.0, -time) * 0.3 +
    createRadialWave(v + 0.5, u - 0.3, 10.0, -time) * 0.6 +
    sin((v + (sin(u * 10.0) + sin(u * (3.7 + u))) * 0.05) * 20.0) * cos(time * 2.0) * 0.2 +
    sin(v * 70.0 + time * 3.0) * 0.2 +
    sin((v + sin(u * 5.0) * 0.1) * 200.0 + time * 5.0) * 0.2 +
    cos((v + sin(u * 10.0) * 0.025) * 300.0 + time * 5.0) * 0.01
  );

  vec3 normal = vec3(x, y, 1.0);

  return normalize(getTBNMatrix() * normal);
}

void main() {
  const float REFRACTION_INTENSITY = 1.0;

  vec3 world_position = getWorldPosition(gl_FragCoord.z, getPixelCoords(), matInverseProjection, matInverseView);
  vec3 normalized_fragment_to_camera = normalize(cameraPosition - world_position);
  vec3 color = vec3(1.0);
  vec3 normal = getNormal();// normalize(fragNormal);

  // normal = normalize(normal + getNormalOffset(world_position));

  vec3 refraction_ray = refract(normalized_fragment_to_camera, normal, 0.7);
  vec3 world_refraction_ray = world_position + refraction_ray * REFRACTION_INTENSITY;
  // @hack invert Z
  vec3 view_refraction_ray = glVec3(matView * glVec4(world_refraction_ray));
  vec2 refracted_color_coords = getScreenCoordinates(view_refraction_ray, matProjection);
  float sample_depth = texture(texColorAndDepth, getPixelCoords()).w;

  if (sample_depth < 1.0 && isOffScreen(refracted_color_coords, 0.0)) {
    // If the fragment has a depth closer than the far plane,
    // discard any attempts at offscreen color reading
    discard;
  }

  if (gl_FragCoord.z > sample_depth) {
    // Accommodation for alpha-blended particles, which write to the
    // depth channel of the color and depth texture, but not to the
    // depth buffer, in order to properly blend against themselves.
    // Perform a 'manual' depth test to ensure that particles in front
    // of the refractive geometry aren't overwritten and incorrectly
    // rendered 'behind' it.
    discard;
  }

  vec4 refracted_color_and_depth = texture(texColorAndDepth, refracted_color_coords);

  if (refracted_color_and_depth.w < gl_FragCoord.z) {
    refracted_color_and_depth.rgb = vec3(0);
  }

  // Skybox
  vec3 reflection_ray = reflect(normalized_fragment_to_camera * -1, normal);

  refracted_color_and_depth.rgb += getSkyColor(reflection_ray);

  // Slightly darken fragments facing the camera more directly
  float intensity = 1.0 - 0.2 * dot(normal, normalized_fragment_to_camera);

  refracted_color_and_depth.rgb *= fragColor;

  out_color_and_depth = vec4(refracted_color_and_depth.rgb * intensity, gl_FragCoord.z);
}