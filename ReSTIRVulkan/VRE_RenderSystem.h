#pragma once
#include "VRE_Device.h"
#include "VRE_GameObject.h"
#include "VRE_Pipeline.h"
#include <memory>

namespace VRE {
    class VRE_RenderSystem
    {
    public:
        VRE_RenderSystem(VRE_Device &device, VkRenderPass renderPass);
        ~VRE_RenderSystem();

        VRE_RenderSystem(const VRE_RenderSystem&) = delete;
        VRE_RenderSystem& operator=(const VRE_RenderSystem&) = delete;

        void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VRE_GameObject>& gameObjects);

    private:
        void CreatePipelineLayout();
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
    };
}