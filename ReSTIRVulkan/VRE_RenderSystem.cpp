#include "VRE_RenderSystem.h"
#include <stdexcept>

//will be refactored and moved.
namespace VRE {
    //push constant data.
    struct SimplePCData {
        glm::mat2 transform{ 1.f };
        glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };
}

VRE::VRE_RenderSystem::VRE_RenderSystem(VRE_Device& device, VkRenderPass renderPass)
    : mDevice(device)
{
    CreatePipelineLayout();
    CreatePipeline(renderPass);
}

VRE::VRE_RenderSystem::~VRE_RenderSystem()
{
    vkDestroyPipelineLayout(mDevice.device(), mPipelineLayout, nullptr);
}

void VRE::VRE_RenderSystem::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<VRE_GameObject>& gameObjects)
{
    mPipeline->Bind(commandBuffer);
    for (auto& obj : gameObjects) {
        SimplePCData data{};
        data.offset = obj.mTransform.position;
        data.color = obj.mColor;
        data.transform = obj.mTransform.mat2();

        vkCmdPushConstants(commandBuffer,
            mPipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePCData),
            &data);
        obj.mModel->Bind(commandBuffer);
        obj.mModel->Draw(commandBuffer);
    }
}

void VRE::VRE_RenderSystem::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange{ VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(SimplePCData) };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(mDevice.device(), &pipelineLayoutInfo, nullptr, &mPipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout!");
    }
}

    void VRE::VRE_RenderSystem::CreatePipeline(VkRenderPass renderPass)
{
    assert(mPipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

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
