#pragma once
#include "VRE_Device.h"
#include "VRE_SharedContext.h"
#include "VRE_Pipeline.h"

namespace VRE {
    class VRE_PointLightRenderSystem
    {
    public:
        VRE_PointLightRenderSystem(VRE_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout);
        ~VRE_PointLightRenderSystem();

        VRE_PointLightRenderSystem(const VRE_PointLightRenderSystem&) = delete;
        VRE_PointLightRenderSystem& operator=(const VRE_PointLightRenderSystem&) = delete;

        void Update(VRE_FrameContext& frameInfo, UBO &ubo, float dt, VRE_SceneContext& sceneContext);
        void RenderLights(VRE_FrameContext& frameContext, VRE_SceneContext& sceneContext);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
    };
}