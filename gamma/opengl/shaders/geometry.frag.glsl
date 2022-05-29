#version 460 core

uniform bool hasTexture = false;
uniform bool hasNormalMap = false;
uniform sampler2D meshTexture;
uniform sampler2D meshNormalMap;

flat in vec3 fragColor;
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

  out_color_and_depth = vec4(color, gl_FragCoord.z);
  out_normal_and_emissivity = vec4(getNormal(), 0.0);
}