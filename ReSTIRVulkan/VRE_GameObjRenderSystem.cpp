#include "VRE_GameObjRenderSystem.h"
#include <stdexcept>

VRE::VRE_GameObjRenderSystem::VRE_GameObjRenderSystem(VRE_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout)
    : mDevice(device)
{
    CreatePipelineLayout(descSetLayout);
    CreatePipeline(renderPass);
}

VRE::VRE_GameObjRenderSystem::~VRE_GameObjRenderSystem()
{
    vkDestroyPipelineLayout(mDevice.GetVkDevice(), mPipelineLayout, nullptr);
}

void VRE::VRE_GameObjRenderSystem::RenderGameObjects(VRE_SharedContext& frameInfo)
{
    //mPipeline->Bind(frameInfo.mCommandBuffer);

    //vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &frameInfo.mDescSet, 0, nullptr);

    //for (auto& e : frameInfo.mGameObjects) {
    //    if (!e.second.mModel)
    //        continue;

    //    auto bufferInfo = e.second.GetBufferInfo(frameInfo.mFrameIndex);

    //    VkDescriptorSet gameObjDescSet;
    //    VRE_DescriptorWriter(*mRenderSystemLayout, frameInfo.mFrameDescPool)
    //        .WriteBuffer(0, &bufferInfo)
    //        .Build(gameObjDescSet);

    //    vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 1, 1, &gameObjDescSet, 0, nullptr);

    //    e.second.mModel->Bind(frameInfo.mCommandBuffer);
    //    e.second.mModel->Draw(frameInfo.mCommandBuffer);
    //}
}

void VRE::VRE_GameObjRenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descSetLayout)
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GameObjectBufferData) };

    mRenderSystemLayout = VRE_DescriptorSetLayout::Builder(mDevice)
                         .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                         .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                         .Build();

    std::vector<VkDescriptorSetLayout> descSetLayouts{descSetLayout, mRenderSystemLayout->GetDescriptorSetLayout() };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mDevice.GetVkDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");
}

void VRE::VRE_GameObjRenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.mRenderPass = renderPass;
    pipelineConfig.mPipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(mDevice, pipelineConfig, "Shaders/test_shader.vert.spv", "Shaders/test_shader.frag.spv");
}