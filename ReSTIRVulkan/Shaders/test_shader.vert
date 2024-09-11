#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

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
    vec4 positionInWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projectionViewMat * positionInWorld;

    vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

    vec3 vecToLight = ubo.lightPosition - positionInWorld.xyz;

    float attenuation = 1.0 / dot(vecToLight, vecToLight); //inverse distance to light squared

    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation; //apply the intensity scale to the color.
    vec3 ambient = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; //apply the intensity scale to the color.

    vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(vecToLight)), 0);

    fragColor = (diffuseLight + ambient) * color;
}