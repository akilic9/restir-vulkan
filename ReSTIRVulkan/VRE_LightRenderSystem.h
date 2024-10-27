#pragma once
#include "VRE_Device.h"
#include "VRE_SharedContext.h"
#include "VRE_Pipeline.h"

namespace VRE {
    class VRE_LightRenderSystem
    {
    public:
        VRE_LightRenderSystem(VRE_SharedContext* sceneContext);
        ~VRE_LightRenderSystem();

        VRE_LightRenderSystem(const VRE_LightRenderSystem&) = delete;
        VRE_LightRenderSystem& operator=(const VRE_LightRenderSystem&) = delete;

        void Init();
        void Update(UBO &ubo, float dt);
        void RenderLights();

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
        VRE_SharedContext* mSceneContext;
    };
}