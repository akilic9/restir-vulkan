#version 460

const vec2 VertexOffsets[6] = vec2[](
  vec2(-1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, -1.0),
  vec2(1.0, -1.0),
  vec2(-1.0, 1.0),
  vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionMat;
    mat4 viewMat;
    vec4 ambientLightColor; // r, g, b, intensity
    vec3 lightPosition;
    vec4 lightColor; // r, g, b, intensity
} ubo;

const float LIGHT_RADIUS = 0.1;

void main() {
    fragOffset = VertexOffsets[gl_VertexIndex];

    vec4 lightPos = ubo.viewMat * vec4(ubo.lightPosition, 1.0); //In camera space.
    vec4 fragPos = lightPos + LIGHT_RADIUS * vec4(fragOffset, 0.0, 0.0); //In camera space.

    gl_Position = ubo.projectionMat * fragPos;
}
