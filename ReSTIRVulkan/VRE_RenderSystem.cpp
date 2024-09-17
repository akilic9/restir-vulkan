#include "VRE_RenderSystem.h"
#include <stdexcept>

//will be refactored and moved.
namespace VRE {
    //push constant data.
    struct SimplePCData {
        glm::mat4 mModelMatrix{ 1.f };
        glm::mat4 mNormalMatrix{ 1.f };
    };
}

VRE::VRE_RenderSystem::VRE_RenderSystem(VRE_Device& device, VkRenderPass renderPass, VkDescriptorSetLayout descSetLayout)
    : mDevice(device)
{
    CreatePipelineLayout(descSetLayout);
    CreatePipeline(renderPass);
}

VRE::VRE_RenderSystem::~VRE_RenderSystem()
{
    vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
}

void VRE::VRE_RenderSystem::RenderGameObjects(VRE_FrameInfo& frameInfo)
{
    mPipeline->Bind(frameInfo.mCommandBuffer);

    vkCmdBindDescriptorSets(frameInfo.mCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &frameInfo.mDescSet, 0, nullptr);

    for (auto& e : frameInfo.mGameObjects) {
        if (!e.second.mModel)
            continue;

        SimplePCData pc{};
        pc.mModelMatrix = e.second.mTransform.Mat4();
        pc.mNormalMatrix = e.second.mTransform.NormalMatrix();

        vkCmdPushConstants(frameInfo.mCommandBuffer,
            mPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePCData),
            &pc);

        e.second.mModel->Bind(frameInfo.mCommandBuffer);
        e.second.mModel->Draw(frameInfo.mCommandBuffer);
    }
}

void VRE::VRE_RenderSystem::CreatePipelineLayout(VkDescriptorSetLayout descSetLayout)
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePCData) };

    std::vector<VkDescriptorSetLayout> descSetLayouts{descSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

    void VRE::VRE_RenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout!");

    PipelineConfigInfo pipelineConfig{};
    VRE_Pipeline::GetDefaultPipelineConfigInfo(pipelineConfig);
    pipelineConfig.renderPass = renderPass;
    pipelineConfig.pipelineLayout = mPipelineLayout;
    mPipeline = std::make_unique<VRE_Pipeline>(
        mDevice,
        pipelineConfig,
        "Shaders/test_shader.vert.spv",
        "Shaders/test_shader.frag.spv");
}