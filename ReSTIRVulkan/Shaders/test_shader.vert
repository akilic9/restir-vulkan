#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outPosWorld;
layout(location = 2) out vec3 outNormalWorld;
layout(location = 3) out vec2 outTexCoord;

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

layout(set = 1, binding = 0) uniform GameObjectBufferData {
  mat4 modelMatrix;
  mat4 normalMatrix;
} gameObject;

void main() {
    vec4 positionInWorld = gameObject.modelMatrix * vec4(inPosition, 1.0);
    gl_Position = ubo.projectionMat * ubo.viewMat * positionInWorld; //The order is important!

    outNormalWorld = normalize(mat3(gameObject.normalMatrix) * inNormal);
    outPosWorld = positionInWorld.xyz;
    outColor = inColor;
    outTexCoord = inTexCoord;
}