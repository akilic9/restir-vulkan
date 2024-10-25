#pragma once
#include "VRE_Device.h"
#include "VRE_SharedContext.h"
#include "VRE_Pipeline.h"

namespace VRE {
    class VRE_PointLightRenderSystem
    {
    public:
        VRE_PointLightRenderSystem(VRE_SceneContext& sceneContext);
        ~VRE_PointLightRenderSystem();

        VRE_PointLightRenderSystem(const VRE_PointLightRenderSystem&) = delete;
        VRE_PointLightRenderSystem& operator=(const VRE_PointLightRenderSystem&) = delete;

        void Update(UBO &ubo, float dt);
        void RenderLights();

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        VRE_SceneContext& mSceneContext;
    };
}