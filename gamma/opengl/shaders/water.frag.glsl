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

vec3 getNormal(vec3 world_position) {
  float wx = world_position.x;
  float wz = world_position.z;
  float t = time;
  float x = 0;
  float y = 0;

  x = 0.1 * sin(t + wx * 0.5 + sin(t * 5 + wz * 0.5));
  y = 0.1 * sin(t + wz * 0.5 + sin(t * 5 + wx * 0.5));

  x += 0.1 * sin(wx + sin(wz * 0.3));
  y += 0.1 * sin(wz + sin(wx * 0.3));

  // Choppiness
  x += 0.03 * sin(time + wz * 2.0 + sin(time + wx * 2.0));
  y += 0.03 * sin(time + wx * 2.0 + sin(time + wz * 2.0));

  vec3 normal = vec3(x, y, 1.0);

  return normalize(getTBNMatrix() * normal);
}

void main() {
  const float REFRACTION_INTENSITY = 1.0;

  vec3 world_position = getWorldPosition(gl_FragCoord.z, getPixelCoords(), matInverseProjection, matInverseView);
  vec3 normalized_fragment_to_camera = normalize(cameraPosition - world_position);
  vec3 color = vec3(1.0);
  vec3 normal = getNormal(world_position);// normalize(fragNormal);

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

  // refracted_color_and_depth.rgb *= fragColor;

  // refracted_color_and_depth.rgb = vec3(reflection_ray.y);

  out_color_and_depth = vec4(refracted_color_and_depth.rgb * intensity, gl_FragCoord.z);
}