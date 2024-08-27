#pragma once
#include <string>
#include <vector>
#include "VRE_Device.h"

namespace VRE {

    struct PipelineConfigInfo {
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
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