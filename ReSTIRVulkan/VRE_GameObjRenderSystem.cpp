#include "VRE_GameObjRenderSystem.h"
#include <stdexcept>

VRE::VRE_GameObjRenderSystem::VRE_GameObjRenderSystem(VRE_SharedContext* sharedContext)
    : mSharedContext(sharedContext) {}

VRE::VRE_GameObjRenderSystem::~VRE_GameObjRenderSystem()
{
    vkDestroyPipelineLayout(mSharedContext->mDevice->GetVkDevice(), mPipelineLayout, nullptr);
}

void VRE::VRE_GameObjRenderSystem::Init()
{
    CreatePipelineLayouts();
    CreatePipelines();
}



void VRE::VRE_GameObjRenderSystem::Render()
{
    mSharedContext->mObjectDescPools[mSharedContext->mRenderer->GetFrameIndex()]->ResetPool();

    mPipeline->Bind(mSharedContext->mRenderer->GetCurrentCommandBuffer());

    vkCmdBindDescriptorSets(mSharedContext->mRenderer->GetCurrentCommandBuffer(), VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1,
                            &mSharedContext->mSceneDescriptorSets[mSharedContext->mRenderer->GetFrameIndex()], 0, nullptr);

    for (auto& e : *mSharedContext->mGameObjMap) {
        if (!e.second.mModel)
            continue;

        e.second.mModel->Bind(mSharedContext->mRenderer->GetCurrentCommandBuffer());

        auto bufferInfo = e.second.GetBufferInfo();
        auto writer = VRE_DescriptorWriter(*mDescSetLayout, *mSharedContext->mObjectDescPools[mSharedContext->mRenderer->GetFrameIndex()]);
        writer.WriteBuffer(0, &bufferInfo);
        e.second.mModel->Render(mSharedContext->mRenderer->GetCurrentCommandBuffer(), mPipelineLayout, writer);
    }
}

void VRE::VRE_GameObjRenderSystem::CreatePipelineLayouts()
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(GameObjectBufferData) };

    mDescSetLayout = VRE_DescriptorSetLayout::Builder(*mSharedContext->mDevice)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

    std::vector<VkDescriptorSetLayout> descSetLayouts{ mSharedContext->mGlobalDescSetLayout->GetDescriptorSetLayout(),
                                                       mDescSetLayout->GetDescriptorSetLayout()};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mSharedContext->mDevice->GetVkDevice(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS)
        throw std::runtime_error("Failed to create pipeline layout!");
}

void VRE::VRE_GameObjRenderSystem::CreatePipelines()
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.mRenderPass = mSharedContext->mRenderer->GetSwapChainRenderPass();
    pipelineConfig.mPipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(*mSharedContext->mDevice, pipelineConfig, "Shaders/test_shader.vert.spv", "Shaders/test_shader.frag.spv");
}