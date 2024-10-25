#pragma once
#include "vulkan/vulkan.h"
#include "VRE_Renderer.h"
#include "VRE_PointLight.h"
#include "VRE_Descriptor.h"

namespace VRE {

    #define MAX_LIGHTS 10

    struct VRE_FrameContext {
        int mFrameIndex;
        VkCommandBuffer mCommandBuffer;
        VkDescriptorSet mDescSet;
    };

    struct VRE_SceneContext {
        std::vector<VRE_PointLight> mPointLights;
        std::shared_ptr<VRE_DescriptorSetLayout> mGlobalDescSet;
        VRE_Renderer* mRenderer;
        std::vector<VkDescriptorSet> mSceneDescriptorSets;
        VRE_Device* mDevice;
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