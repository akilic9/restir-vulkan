#pragma once
#include "vulkan/vulkan.h"
#include "VRE_Camera.h"
#include "VRE_GameObject.h"

namespace VRE {
    struct VRE_FrameInfo
    {
        int mFrameIndex;
        float mFrameTime;
        VkCommandBuffer mCommandBuffer;
        VRE_Camera &mCamera;
        VkDescriptorSet mDescSet;
        VRE_GameObject::ObjectsMap& mGameObjects;
    };
}