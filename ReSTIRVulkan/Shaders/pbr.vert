#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexCoord0;
layout(location = 4) in vec2 inTexCoord1;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outTexCoord0;
layout(location = 4) out vec2 outTexCoord1;

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

layout (set = 2, binding = 0) uniform UBONode {
	mat4 matrix;
} node;


void main() {
}