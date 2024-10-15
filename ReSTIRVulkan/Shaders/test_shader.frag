#version 460

layout (location = 0) in vec3 inFragColor;
layout (location = 1) in vec3 inFragPosWorld;
layout (location = 2) in vec3 inFragNormalWorld;
layout (location = 3) in vec2 inFragTexCoord;

layout (location = 0) out vec4 outColor;

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

layout(set = 1, binding = 1) uniform sampler2D texSampler;

void main() {
    vec3 diffuse = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; //Apply the intensity scale to the ambient.
    vec3 surfaceNormal = normalize(inFragNormalWorld);
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.invViewMat[3].xyz;
    vec3 viewDir = normalize(cameraPosWorld - inFragPosWorld);

    for (int i = 0; i < ubo.activeLightCount; i++){
        PointLightInfo light = ubo.pointLights[i];
        vec3 lightDir = light.position.xyz - inFragPosWorld;
        float attenuation = 1.0 / dot(lightDir, lightDir); //Inverse distance to light squared.
        lightDir = normalize(lightDir);

        float lambertian = max(dot(surfaceNormal, lightDir), 0);
        vec3 lightIntensity = light.color.xyz * light.color.w * attenuation; //Apply the intensity scale to the color.

        diffuse += lightIntensity * lambertian;

        //blinn phong specular
        vec3 halfAngle = normalize(lightDir + viewDir);
        float specAngle = max(dot(surfaceNormal, halfAngle), 0.0);
        specAngle = pow(specAngle, 512.0); // shininess hardcoded for now.
        specularLight += lightIntensity * specAngle;
    }

    vec3 color = texture(texSampler, inFragTexCoord).xyz;
    outColor = vec4(diffuse * color + specularLight * inFragColor, 1.0);
}