#pragma once
#include "VRE_Device.h"
#include "VRE_GameObject.h"
#include "VRE_Pipeline.h"
#include "VRE_Camera.h"
#include "VRE_FrameInfo.h"
#include <memory>

namespace VRE {
    class VRE_RenderSystem
    {
    public:
        VRE_RenderSystem(VRE_Device &device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout);
        ~VRE_RenderSystem();

        VRE_RenderSystem(const VRE_RenderSystem&) = delete;
        VRE_RenderSystem& operator=(const VRE_RenderSystem&) = delete;

        void RenderGameObjects(VRE_FrameInfo &frameInfo, std::vector<VRE_GameObject>& gameObjects);

    private:
        void CreatePipelineLayout(VkDescriptorSetLayout descSetLayout);
        void CreatePipeline(VkRenderPass renderPass);

        VRE_Device& mDevice;
        std::unique_ptr<VRE_Pipeline> mPipeline;
        VkPipelineLayout mPipelineLayout;
    };
}