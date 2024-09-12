#include "VRE_PointLightRenderSystem.h"
#include <stdexcept>

VRE::VRE_PointLightRenderSystem::VRE_PointLightRenderSystem(VRE_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout)
    : mDevice(device)
{
    CreatePipelineLayout(descSetLayout);
    CreatePipeline(renderPass);
}

VRE::VRE_PointLightRenderSystem::~VRE_PointLightRenderSystem()
{
    vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
}

void VRE::VRE_PointLightRenderSystem::RenderLights(VRE_FrameInfo& frameInfo)
{
    mPipeline->Bind(frameInfo.mCommandBuffer);

    vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &frameInfo.mDescSet, 0, nullptr);

    vkCmdDraw(frameInfo.mCommandBuffer, 6, 1, 0, 0);
}

void VRE::VRE_PointLightRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descSetLayout)
{
    //VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
    //                       0,
    //                       sizeof(SimplePCData) };

    std::vector<VkDescriptorSetLayout> descSetLayouts{ descSetLayout };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;

    if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");
}

void VRE::VRE_PointLightRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.attributeDescriptions.clear();
    pipelineConfig.bindingDescriptions.clear();
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(
        mDevice,
        pipelineConfig,
        "Shaders/point_light.vert.spv",
        "Shaders/point_light.frag.spv");
}
