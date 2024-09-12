#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform UBO {
    mat4 projectionViewMat;
    vec4 ambientLightColor; // r, g, b, intensity
    vec3 lightPosition;
    vec4 lightColor; // r, g, b, intensity
} ubo;

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec3 directionToLight = ubo.lightPosition - fragPosWorld;

    float attenuation = 1.0 / dot(directionToLight, directionToLight); //inverse distance to light squared.

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; //apply the intensity scale to the color.

    vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; //apply the intensity scale to the ambient.

    vec3 diffuse = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4((diffuse + ambient) * fragColor, 1.0);
}