#version 460

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragPosWorld;
layout (location = 2) in vec3 fragNormalWorld;

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

layout(push_constant) uniform Push {
    mat4 modelMatrix;
    mat4 normalMatrix;
} push;

void main() {
    vec3 diffuse = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w; //Apply the intensity scale to the ambient.
    vec3 surfaceNormal = normalize(fragNormalWorld);
    vec3 specularLight = vec3(0.0);

    vec3 cameraPosWorld = ubo.invViewMat[3].xyz;
    vec3 viewDir = normalize(cameraPosWorld - fragPosWorld);

    for (int i = 0; i < ubo.activeLightCount; i++){
        PointLightInfo light = ubo.pointLights[i];
        vec3 lightDir = light.position.xyz - fragPosWorld;
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

    outColor = vec4(diffuse * fragColor + specularLight * fragColor, 1.0);
}