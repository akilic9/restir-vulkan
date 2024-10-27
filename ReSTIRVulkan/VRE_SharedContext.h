#pragma once
#include "vulkan/vulkan.h"
#include "VRE_Renderer.h"
#include "VRE_PointLight.h"
#include "VRE_Descriptor.h"

namespace VRE {

    #define MAX_LIGHTS 10

    struct VRE_SharedContext {
        VRE_Device* mDevice;
        VRE_Renderer* mRenderer;
        std::shared_ptr<VRE_DescriptorSetLayout> mGlobalDescSetLayout;
        std::vector<VkDescriptorSet> mSceneDescriptorSets;
        std::vector<VRE_PointLight> mPointLights;
        VRE_GameObject::GameObjectsMap& mGameObjMap;
    };

    struct PointLightInfo {
        glm::vec4 mPosition{};  // w is just for alignment.
        glm::vec4 mColor{};     // w is intensity
    };

    struct UBO {
        glm::mat4 mProjectionMat = 1.f;
        glm::mat4 mViewMat = 1.f;
        glm::mat4 mInvViewMat = 1.f;
        glm::vec4 mAmbientLightColor{ 1.f, 1.f, 1.f, 0.02f }; // r, g, b, intensity
        PointLightInfo mPointLights[MAX_LIGHTS];
        int mActiveLightCount;
    };
}