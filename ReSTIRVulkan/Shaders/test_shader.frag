#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLightInfo {
    vec4 position;  // w is just for alignment.
    vec4 color;     // w is intensity
};

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionMat;
    mat4 viewMat;
    vec4 ambientLightColor; // r, g, b, intensity
    PointLightInfo pointLights[10]; //In FrameInfo.h, hard coded for now :(
    int activeLightCount;
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec3 diffuse = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; //apply the intensity scale to the ambient.
    vec3 surfaceNormal = normalize(fragNormalWorld);

    for (int i = 0; i < ubo.activeLightCount; i++){
        PointLightInfo light = ubo.pointLights[i];
        vec3 directionToLight = light.position.xyz - fragPosWorld;
        float attenuation = 1.0 / dot(directionToLight, directionToLight); //inverse distance to light squared.
        float cosAngIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0);
        vec3 lightIntensity = light.color.xyz * light.color.w * attenuation; //apply the intensity scale to the color.

        diffuse += lightIntensity * cosAngIncidence;
    }

    outColor = vec4(diffuse * fragColor, 1.0);
}