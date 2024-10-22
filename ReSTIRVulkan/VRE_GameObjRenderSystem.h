#pragma once
#include "VRE_Device.h"
#include "VRE_GameObject.h"
#include "VRE_Pipeline.h"
#include "VRE_Camera.h"
#include "VRE_SharedContext.h"
#include <memory>

namespace VRE {
    class VRE_GameObjRenderSystem
    {
    public:
        VRE_GameObjRenderSystem(VRE_Device &device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout);
        ~VRE_GameObjRenderSystem();

        VRE_GameObjRenderSystem(const VRE_GameObjRenderSystem&) = delete;
        VRE_GameObjRenderSystem& operator=(const VRE_GameObjRenderSystem&) = delete;

        void RenderGameObjects(VRE_FrameContext &frameInfo);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        std::unique_ptr<VRE_DescriptorSetLayout> mRenderSystemLayout;
    };
}

/*
     mFramePools.resize(VRE_SwapChain::MAX_FRAMES_IN_FLIGHT);
    auto framePoolBuilder = VRE_DescriptorPool::Builder(mDevice)
                            .SetMaxSets(1000)
                            .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
                            .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
                            .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT);

    for (int i = 0; i < mFramePools.size(); i++) {
        mFramePools[i] = framePoolBuilder.Build();
    }




*/