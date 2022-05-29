#version 460 core

uniform bool hasTexture = false;
uniform bool hasNormalMap = false;
uniform vec3 cameraPosition;
uniform sampler2D meshTexture;
uniform sampler2D meshNormalMap;
uniform samplerCube probeMap;

flat in vec3 fragColor;
in vec3 fragPosition;
in vec3 fragNormal;
in vec3 fragTangent;
in vec3 fragBitangent;
in vec2 fragUv;

layout (location = 0) out vec4 out_color_and_depth;
layout (location = 1) out vec4 out_normal_and_emissivity;

vec3 getNormal() {
  vec3 n_fragNormal = normalize(fragNormal);

  if (hasNormalMap) {
    vec3 mappedNormal = texture(meshNormalMap, fragUv).rgb * 2.0 - vec3(1.0);

    mat3 tangentMatrix = mat3(
      normalize(fragTangent),
      normalize(fragBitangent),
      n_fragNormal
    );

    return normalize(tangentMatrix * mappedNormal);
  } else {
    return n_fragNormal;
  }
}

void main() {
  vec3 color = hasTexture ? texture(meshTexture, fragUv).rgb * fragColor : fragColor;
  vec3 position = fragPosition;
  vec3 normal = getNormal();

  vec3 incident_vector = normalize(position - cameraPosition);
  vec3 reflection_vector = reflect(incident_vector, normal);
  // @todo support roughness
  vec3 probe_reflector_color = color * texture(probeMap, reflection_vector).rgb;

  out_color_and_depth = vec4(probe_reflector_color, gl_FragCoord.z);
  // @todo support custom emissivity
  out_normal_and_emissivity = vec4(normal, 0.3);
}