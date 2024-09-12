#version 460

layout (location = 0) in vec2 fragOffset;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionMat;
    mat4 viewMat;
    vec4 ambientLightColor; // r, g, b, intensity
    vec3 lightPosition;
    vec4 lightColor; // r, g, b, intensity
} ubo;

void main() {
  float distance = sqrt(dot(fragOffset, fragOffset));

  if (distance >= 1.0)
    discard;

  outColor = vec4(ubo.lightColor.xyz, 1.0);
}