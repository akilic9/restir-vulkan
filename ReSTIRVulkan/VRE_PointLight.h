/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*/
#pragma once
#include "glm.hpp"
#include <memory>

namespace VRE {
    struct VRE_PointLight {

        glm::vec4 mPosition{0.f, 0.f, 0.f, 1.f}; //vec4 for alignment.
        float mScale{ 1.f };
        glm::vec3 mColor{1.0f};
        float mLightIntensity = 1.0f;

        VRE_PointLight() {}

        static VRE_PointLight CreatePointLight(float lightIntensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f)) {
            auto pointLight = VRE_PointLight();
            pointLight.mColor = color;
            pointLight.mLightIntensity = lightIntensity;
            pointLight.mScale = radius;
            return pointLight;
        }
    };
}