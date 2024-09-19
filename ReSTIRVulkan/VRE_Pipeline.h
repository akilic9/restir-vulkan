#pragma once
#include <string>
#include <vector>
#include "VRE_Device.h"

namespace VRE {

    struct PipelineConfigInfo {
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