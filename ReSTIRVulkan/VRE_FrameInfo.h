#pragma once
#include "vulkan/vulkan.h"
#include "VRE_Camera.h"
#include "VRE_GameObject.h"
#include "VRE_PointLight.h"

namespace VRE {

    #define MAX_LIGHTS 10

    struct VRE_FrameInfo {
        int mFrameIndex;
        VkCommandBuffer mCommandBuffer;
        VRE_Camera &mCamera;
        VkDescriptorSet mDescSet;
        VRE_GameObject::GameObjectsMap &mGameObjects;
        std::vector<VRE_PointLight> &mPointLights;
    };

    struct PointLightInfo {
        glm::vec4 mPosition{};  // w is just for alignment.
        glm::vec4 mColor{};     // w is intensity
    };

    struct UBO {
        glm::mat4 mProjectionMat = 1.f;
        glm::mat4 mViewMat = 1.f;
        glm::vec4 mAmbientLightColor{ 1.f, 1.f, 1.f, 0.02f }; // r, g, b, intensity
        PointLightInfo mPointLights[MAX_LIGHTS];
        int mActiveLightCount;
    };
}