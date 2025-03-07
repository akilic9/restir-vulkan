/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*/
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

struct PointLightInfo {
    vec4 position;  // w is just for alignment.
    vec4 color;     // w is intensity.
};

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionMat;
    mat4 viewMat;
    mat4 invViewMat;
    vec4 ambientLightColor; // r, g, b, intensity.
    PointLightInfo pointLights[10]; //In ShaerdContext.h, hard coded for now :(
    int activeLightCount;
} ubo;

layout(push_constant) uniform Push {
    vec4 position;
    vec4 color;
    float radius;
} push;

void main() {
    fragOffset = VertexOffsets[gl_VertexIndex];

    vec4 lightPos = ubo.viewMat * push.position; //In camera space.
    vec4 fragPos = lightPos + push.radius * vec4(fragOffset, 0.0, 0.0); //In camera space.

    gl_Position = ubo.projectionMat * fragPos;
}
