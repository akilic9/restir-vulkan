/*
*  Resources:
*   Galea, B. (2020). Vulkan Game Engine Tutorial. [online] YouTube. Available at: https://www.youtube.com/watch?v=Y9U9IE0gVHA&list=PL8327DO66nu9qYVKLDmdLW_84-yE4auCR&index=1 and https://github.com/blurrypiano/littleVulkanEngine (Accessed 15 June 2024).
*   Willems, S. (2023). Vulkan C++ examples and demos. [online] GitHub. Available at: https://github.com/SaschaWillems/Vulkan (Accessed 12 June 2024).
*   Overvoorde, A. (2017). Khronos Vulkan Tutorial. [online] Vulkan.org. Available at: https://docs.vulkan.org/tutorial/latest/00_Introduction.html (Accessed 07 June 2024).
*/
#pragma once
#include <string>
#include <vector>
#include "VRE_Device.h"

namespace VRE {

    struct PipelineConfigInfo {
        PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        std::vector<VkVertexInputBindingDescription> mBindingDescriptions{};
        std::vector<VkVertexInputAttributeDescription> mAttributeDescriptions{};
        VkPipelineViewportStateCreateInfo mViewportInfo;
        VkPipelineInputAssemblyStateCreateInfo mInputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo mRasterizationInfo;
        VkPipelineMultisampleStateCreateInfo mMultisampleInfo;
        VkPipelineColorBlendAttachmentState mColorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo mColorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo mDepthStencilInfo;
        std::vector<VkDynamicState> mDynamicStateEnables;
        VkPipelineDynamicStateCreateInfo mDynamicStateInfo;
        VkPipelineLayout mPipelineLayout = nullptr;
        VkRenderPass mRenderPass = nullptr;
        uint32_t mSubpass = 0;
    };

    class VRE_Pipeline
    {
    public:
        VRE_Pipeline(VRE_Device &device, const PipelineConfigInfo& configInfo, const std::string& vertShaderPath, const std::string& fragShaderPath);
        ~VRE_Pipeline();

        VRE_Pipeline(const VRE_Pipeline&) = delete;
        VRE_Pipeline& operator=(const VRE_Pipeline&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        static void GetDefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

    private:
        static std::vector<char> ReadFile(const std::string& filePath);
        void CreateGraphicsPipeline(const PipelineConfigInfo& configInfo, const std::string& vertShaderPath, const std::string& fragShaderPath);
        void CreateShaderModule(const std::vector<char>& shaderCode, VkShaderModule* shaderModule);

        VRE_Device& mDevice;
        VkPipeline mGraphicsPipeline;
        VkShaderModule mVertShaderModule;
        VkShaderModule mFragShaderModule;
    };
}