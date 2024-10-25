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

        void RenderGameObjects();

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        std::unique_ptr<VRE_DescriptorSetLayout> mRenderSystemLayout;
    };
}