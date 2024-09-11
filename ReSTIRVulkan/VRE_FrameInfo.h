#pragma once
#include "vulkan/vulkan.h"
#include "VRE_Camera.h"

namespace VRE {
    struct VRE_FrameInfo
    {
        int mFrameIndex;
        float mFrameTime;
        VkCommandBuffer mCommandBuffer;
        VRE_Camera &mCamera;
        VkDescriptorSet mDescSet;
    };
}