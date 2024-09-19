#version 460

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;

struct PointLightInfo {
    vec4 position;  // w is just for alignment.
    vec4 color;     // w is intensity.
};

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionMat;
    mat4 viewMat;
    mat4 invViewMat;
    vec4 ambientLightColor; // r, g, b, intensity.
    PointLightInfo pointLights[10]; //In FrameInfo.h, hard coded for now :(
    int activeLightCount;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec4 positionInWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionMat * ubo.viewMat * positionInWorld; //The order is important!

    fragNormalWorld = normalize(mat3(push.normalMatrix) * normal);
    fragPosWorld = positionInWorld.xyz;
    fragColor = color;
}