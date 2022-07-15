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

vec2 createRadialWave(vec2 offset) {
  const float TAU = 3.141592 * 2.0;
  // @todo make configurable
  const float speed = 1.0;
  const float frequency = 10.0;

  float u = fragUv.x;
  float v = fragUv.y;
  float radius = length(fragUv - vec2(0.5) + offset);
  float t = -time * speed;
  float r = radius * frequency * TAU;

  return vec2(
    sin(t + r + u),
    cos(t + r + v)
  );
}

vec3 getNormal(vec3 world_position) {
  float wx = world_position.x;
  float wz = world_position.z;
  float t = time;
  vec2 n = vec2(0);

  // @todo make configurable
  n += createRadialWave(vec2(0)) * 0.5;
  n += createRadialWave(vec2(0.3, -0.2)) * 0.4;
  n += createRadialWave(vec2(-0.7, 0.3)) * 0.3;

  n.x += 0.5 * sin(t + wx * 0.2 + sin(t * 3 + wz * 0.2));
  n.y += 0.5 * sin(t + wz * 0.2 + sin(t * 3 + wx * 0.2));

  n.x += 0.3 * sin(wx * 0.3 + sin(wz * 0.3));
  n.y += 0.3 * sin(wz * 0.3 + sin(wx * 0.3));

  n.x += 0.1 * sin(time + wz * 2.0 + sin(time + wx));
  n.y += 0.1 * sin(time + wx * 2.0 + sin(time + wz)); 

  vec3 n_normal = normalize(fragNormal);
  vec3 n_tangent = normalize(fragTangent);
  vec3 n_bitangent = normalize(fragBitangent);

  mat3 tbn_matrix = mat3(
    n_tangent,
    n_bitangent,
    n_normal
  );

  vec3 tangent_normal = vec3(n.x, n.y, 1.0);
  vec3 world_normal = normalize(tbn_matrix * tangent_normal);

  world_normal.y += n_normal.y * 3.0;

  return normalize(world_normal);
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
    // If the refraction sample coordinates are off-screen,
    // disable refractive effects
    // @todo improve this
    refracted_color_coords = getPixelCoords();
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
  float refraction_fresnel = dot(-refraction_ray, normalized_fragment_to_camera);

  if (refracted_color_and_depth.w < gl_FragCoord.z) {
    refracted_color_and_depth.rgb = vec3(0);
  }

  // @hack @todo description
  if (refraction_fresnel < 0) refraction_fresnel *= -1;

  refracted_color_and_depth.rgb *= refraction_fresnel;

  // Skybox
  vec3 reflection_ray = reflect(normalized_fragment_to_camera * -1, normal);
  float reflection_fresnel = min(1.0, 1.0 - dot(reflection_ray, normalized_fragment_to_camera));

  refracted_color_and_depth.rgb += getSkyColor(reflection_ray) * reflection_fresnel;

  // Slightly darken fragments facing the camera more directly
  float intensity = 1.0 - 0.2 * dot(normal, normalized_fragment_to_camera);

  refracted_color_and_depth.rgb *= fragColor;

  out_color_and_depth = vec4(refracted_color_and_depth.rgb * intensity, gl_FragCoord.z);
}